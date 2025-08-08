#ifndef __STAGE_HELPERS_H__
#define __STAGE_HELPERS_H__

#include <stdio.h>
#include "utils.h"
#include "pipeline.h"

/// EXECUTE STAGE HELPERS ///

/**
 * input  : idex_reg_t
 * output : uint32_t alu_control signal
 * Lex
 **/
uint32_t gen_alu_control(idex_reg_t idex_reg) {
  uint32_t alu_control = 0;
  // Lex
  switch (idex_reg.read_opcode) {
    case 0x33: // R-type
      switch (idex_reg.read_funct3) {
        case 0x0:
          switch (idex_reg.read_funct7) {
            case 0x00:
              alu_control = 0x0; // add
              break;
            case 0x20:
              alu_control = 0x1; // sub
              break;
            case 0x01:
              alu_control = 0x4; // mul
              break;
          }
        case 0x1:
          switch (idex_reg.read_funct7) {
            case 0x00:
              alu_control = 0x07;
              break;
            case 0x01:
              alu_control = 0xB; // mulh, S * U
              idex_reg.read_rs1 = (sWord)idex_reg.read_rs1;
              idex_reg.read_rs2 = (Word)idex_reg.read_rs2;

              break;
            default:
              //default: // undefined
              alu_control = 0xBADCAFFE;
              break;
          }
        case 0x2:
          switch (idex_reg.read_funct7) {
            case 0x0: // sltu
            alu_control = 0x9;
            break;
          case 0x01: // mulh unsigned
            alu_control = 0xB;
            idex_reg.read_rs2 = (unsigned)idex_reg.read_rs2; //Maybe this? ask
            break;
          default:
            alu_control = 0XBACAFFE;
            break;
        } 
        case 0x3:
          switch (idex_reg.read_funct7) {
             case 0x0: // unsigned slt
            alu_control = 0x9;
            break;
          case 0x1: //Multiply high unsigned
            alu_control = 0xB;
            idex_reg.read_rs2 = (unsigned)idex_reg.read_rs2;
            idex_reg.read_rs1 = (unsigned)idex_reg.read_rs1;
            break;
          default:
            alu_control = 0XBADCAFFE;
            break;
          }
        case 0x4:
          switch (idex_reg.read_funct7) {
            case 0x01: //div
              alu_control = 0x5;
              break;
            case 0x00:
              alu_control = 0x6;
              break;
            default:
              alu_control = 0XBADCAFFE;
              break;
          }
        case 0x5:
          switch (idex_reg.read_funct7) {
            case 0x0:
            case 0x20: //msb required
              alu_control = 0x8;
              break;
            case  0x01: //div unsigned
              alu_control = 0x5;
              idex_reg.read_rs1 = (unsigned)idex_reg.read_rs1;
              idex_reg.read_rs2 = (unsigned)idex_reg.read_rs2;
              break;
            default:
              alu_control = 0XBADCAFFE;
              break;
          }
        case 0x6:
          switch (idex_reg.read_funct7) {
            case 0x0: //or
              alu_control = 0x3;
              break;
            case 0x01: // rem
              alu_control = 0xA;
              break;
            default:
              alu_control = 0XBADCAFFE;
              break;
          }
        case 0x7:
          switch(idex_reg.read_funct7) {
            case 0x00:
              alu_control = 0x2;
              break;
            case 0x01: //unsigned rem
              alu_control = 0xA;
              break;
            default:
              alu_control = 0XBADCAFFE;
              break;
          }
        default: // undefined
          alu_control = 0xBADCAFFE;
          break;
      }
    case 0x13: // I type
      switch (idex_reg.read_funct3) {
        case 0x0: // add
          alu_control = 0x0;
          break;
        case 0x4: //xor
          alu_control = 0x6;
          break;
        case 0x6: // or
          alu_control = 0x3;
          break;
        case 0x7:
          alu_control = 0x2;
          break;
        case 0x1:
          switch (idex_reg.read_imm) { //potential error
            case 0x0:
              alu_control = 0xC;
              break;
            default:
              alu_control = 0xBADCAFFE;
              break;
          }
        case 0x5:
          switch (idex_reg.read_imm) {
            case 0x0:
              alu_control = 0xD;
              break;
            case 0x20: //extend
              alu_control = 0xD;
              break;
            default:
              alu_control = 0xBADCAFFE;
              break;
          }
        case 0x2:
          alu_control = 0x9;
          break;
        case 0x3: // (U), zero extend
          alu_control = 0x9;
          //extend = true;
          idex_reg.read_rs1 = (unsigned)idex_reg.read_rs1;
          break;
        default:
          alu_control = 0xBADCAFFE;
          break;
      }
  case 0x3: //I type load
  case 0x6F: // JAL
  case 0x67: // JALR
    case 0x23: // store
      alu_control = 0x0;
      break;
    case 0x63: //branch
      alu_control = 0x1;
      break;
    default: // undefined
      alu_control = 0xBADCAFFE;
      break;
  }
  return (alu_control);
}

/**
 * input  : alu_inp1, alu_inp2, alu_control
 * output : uint32_t alu_result
 * Lex
 **/
uint32_t execute_alu(uint32_t alu_inp1, uint32_t alu_inp2, uint32_t alu_control) {
  uint32_t result;
  switch (alu_control) {
    case 0x0: // add
      result = alu_inp1 + alu_inp2;
      break;
    case 0x1: // sub
      result = alu_inp1 - alu_inp2;
      break;
    case 0x2: // and
      result = alu_inp1 & alu_inp2;
      break;
    case 0x3: // or
      result = alu_inp1 | alu_inp2;
      break;
    // idk if legal, consult
    case 0x4: // mul
      result = alu_inp1 * alu_inp2;
      break;
    case 0x5: // div
      if (alu_inp2 == 0) {
        result = 0xBADCAFFE; // error code
      }
      else {
        result = alu_inp1 / alu_inp2;
      }
      break;
    case 0x6: // xor
      result = alu_inp1 ^ alu_inp2;
      break;
    case 0x7: // <<
      result = alu_inp1 << alu_inp2;
      break;
    case 0x8: // >>
      result = alu_inp1 >> alu_inp2;
      break;
    case 0x9: // ?
      result = (alu_inp1 < alu_inp2) ? 1 : 0;
      break;
    case 0xA: // %
      if (alu_inp2 == 0) {
        result = 0xBADCAFFE; // error code
      }
      else {
        result = alu_inp1 % alu_inp2;
      }
      break;

    case 0xB: //mulh
      result = (alu_inp1 * alu_inp2);
      break;

    case 0xC: //slli
      result = (alu_inp1 << alu_inp2);
      break;
    
    case 0xD: //slri
      result =(alu_inp1 >> alu_inp2);
      break;

    default:
      result = 0xBADCAFFE;
      break;
  };
  return result;
}

/// DECODE STAGE HELPERS ///

/**
 * input  : Instruction
 * output : idex_reg_t
 * Kirstin
 **/
uint32_t gen_imm(Instruction instruction) {

  int imm_val = 0;
  // get the imm values for instructions with imms
  switch (instruction.opcode) {
  case 0x63: // B-type
    imm_val = get_branch_offset(instruction);
    break;
  case 0x13: // I-type
    imm_val = instruction.itype.imm;
    break;
  case 0x23: // S-type
    imm_val = get_store_offset(instruction);
    break;
  case 0x6F: // J-type
    imm_val = get_jump_offset(instruction);
    break;
  case 0x37: // U-type
    imm_val = instruction.ujtype.imm;
  default: // R and undefined opcode
    break;
  };
  return imm_val;
}

/**
 * generates all the control logic that flows around in the pipeline
 * input  : Instruction
 * output : idex_reg_t
 * Kirstin
 **/
idex_reg_t gen_control(Instruction instruction) {
  idex_reg_t idex_reg = {0};
  // get the opcode instruction, determine what the register idex_reg needs to hold
  switch (instruction.opcode) {
    case 0x33: // R-type
      idex_reg.read_funct3 = instruction.rtype.funct3;
      idex_reg.read_funct7 = instruction.rtype.funct7;
      idex_reg.read_opcode = instruction.opcode;
      idex_reg.alu_op = true;  
      idex_reg.alu_src = false;  
      idex_reg.mem_read = false;
      idex_reg.mem_write = false;
      idex_reg.mem_to_reg = false;
      idex_reg.reg_write = true;
      idex_reg.branch = false;
      break;
    case 0x23: // S-type
      idex_reg.read_funct3 = instruction.stype.funct3;
      idex_reg.read_opcode = instruction.opcode;
      idex_reg.alu_op = true;
      idex_reg.alu_src = true;
      idex_reg.mem_read = false;
      idex_reg.mem_write = true;
      idex_reg.mem_to_reg = false;
      idex_reg.reg_write = false;
      idex_reg.branch = false;
      break;
    case 0x13: // I-type
      idex_reg.read_funct3 = instruction.itype.funct3;
      idex_reg.read_opcode = instruction.itype.opcode;
      idex_reg.alu_op = true;
      idex_reg.alu_src = true;
      idex_reg.mem_read = false;
      idex_reg.mem_write = false;
      idex_reg.mem_to_reg = false;
      idex_reg.reg_write = true;
      idex_reg.branch = false;
      break;
    case 0x63: // B-type
      idex_reg.read_funct3 = instruction.sbtype.funct3;
      idex_reg.read_opcode = instruction.sbtype.opcode;
      idex_reg.alu_op = true; 
      idex_reg.alu_src = false;
      idex_reg.mem_read = false;
      idex_reg.mem_write = false;
      idex_reg.mem_to_reg = false;
      idex_reg.reg_write = false;
      idex_reg.branch = true;
      break;
    case 0x6F: // JAL
      idex_reg.alu_op = false;
      idex_reg.alu_src = false;
      idex_reg.mem_read = false;
      idex_reg.mem_write = false;
      idex_reg.mem_to_reg = false;
      idex_reg.reg_write = true;
      idex_reg.branch = true;
      break;
    case 0x67: // JALR
      idex_reg.read_funct3 = instruction.itype.funct3;
      idex_reg.alu_op = false;
      idex_reg.alu_src = true;
      idex_reg.mem_read = false;
      idex_reg.mem_write = false;
      idex_reg.mem_to_reg = false;
      idex_reg.reg_write = true;
      idex_reg.branch = true;
      break;
    case 0x37: // LUI
      idex_reg.alu_op = false;
      idex_reg.alu_src = true;
      idex_reg.mem_read = false;
      idex_reg.mem_write = false;
      idex_reg.mem_to_reg = false;
      idex_reg.reg_write = true;
      idex_reg.branch = false;
      break;
    default:
      // leave as NOP-like defaults
      break;
  }
  return idex_reg;
}

/// MEMORY STAGE HELPERS ///

/**
 * evaluates whether a branch must be taken
 * input  : <open to implementation>
 * output : bool
 * Kirstin
 **/
bool gen_branch(Instruction instruction, uint32_t read_rs1, uint32_t read_rs2) {
  
  // if instruction is a branch instruction
  if (instruction.opcode == 0x63) {
    switch (instruction.sbtype.funct3) {
      case 0x0:
        return (read_rs1 == read_rs2);
      case 0x1:
        return (read_rs1 != read_rs2);
      case 0x4:
        return (read_rs1 < read_rs2);
      case 0x5:
        return (read_rs1 >=read_rs2);
      case 0x6:
        return (read_rs1 < read_rs2);
      case 0x7:
        return (read_rs1 >=read_rs2);
      default:
        break;
    }

  }
  return false;

}

/// PIPELINE FEATURES ///

/**
 * Task   : Sets the pipeline wires for the forwarding unit's control signals
 *           based on the pipeline register values.
 * input  : pipeline_regs_t*, pipeline_wires_t*
 * output : None
 * Lex
 */
void gen_forward(pipeline_regs_t *pregs_p, pipeline_wires_t *pwires_p) {
  /**
   * 1. EX Hazard: When resolving an EX hazard (which will require a forwarding from EXMEM 
        register  to  the  EX  stage),  the  simulator  should  print  the  following  line: “[FWD]: Resolving EX hazard on RS: xREG” 
     
        2. MEM  Hazard:  When  resolving  a  MEM  hazard  (which  will  require  a  forwarding  from 
        MEMWB  register  to  the  EX  stage),  the  simulator  should  print  the  following  line: “[FWD]: Resolving MEM hazard on RS: xREG” 
   */
  pwires_p->forwardA = 0; //for rs1
  pwires_p->forwardB = 0; // for rs2

  // First check if the function is using rs1 or rs2
  uint8_t temp_rs1 = 0;
  uint8_t temp_rs2 = 0;
  pregs_p->idex_preg.out = pregs_p->idex_preg.inp;
  uint32_t read_opcode = pregs_p->idex_preg.out.instr.opcode; // opcode is 0!!

  // Print current ID/EX instruction opcode
  

  switch (read_opcode) {
    case 0x33: //R type uses both rs1 and rs2, so need to check those
      temp_rs1 = pregs_p->idex_preg.out.instr.rtype.rs1;
      temp_rs2 = pregs_p->idex_preg.out.instr.rtype.rs2;
      break;
    case 0x13: // I type without load, rs2 is just an immediate
      temp_rs1 = pregs_p->idex_preg.out.instr.itype.rs1;
      break;
    case 0x03: // Load, rs2 excluded since only importing data to rs1
      temp_rs1 = pregs_p->idex_preg.out.instr.itype.rs1;
      break;
    case 0x23: // Store
      temp_rs1 = pregs_p->idex_preg.out.instr.stype.rs1;
      temp_rs2 = pregs_p->idex_preg.out.instr.stype.rs2;
      break;
    case 0x63: // Branch
      temp_rs1 = pregs_p->idex_preg.out.instr.sbtype.rs1;
      temp_rs2 = pregs_p->idex_preg.out.instr.sbtype.rs2;
      break;
    case 0x67: // JALR, SKIPPED JAL CUZ IT DOESN'T USE ANY
      temp_rs1 = pregs_p->idex_preg.out.instr.itype.rs1;
      break;
    default:
      break;
  }
  
  
  // Set variables to check for reg_write = 1 and check if match_rd =rs1 or rs2, is writing to this destination
  // need to be done on both exmem and wbmem

  // EX/MEM
  bool exmem_temp_reg_write = pregs_p->exmem_preg.out.reg_write;
  uint8_t exmem_temp_rd = pregs_p->exmem_preg.out.write_rd;

  // WB/MEM
  bool memwb_temp_reg_write = pregs_p->memwb_preg.out.reg_write;
  uint8_t memwb_temp_rd = pregs_p->memwb_preg.out.write_rd;

  // Check EX/MEM stage
  if (exmem_temp_reg_write != 0 && exmem_temp_rd != 0) {
    if (exmem_temp_rd == temp_rs1) {
      pwires_p->forwardA = 2;
      fwd_exex_counter += 1;

      #ifdef DEBUG_CYCLE
      printf("[FWD]: Resolving EX hazard on RS1: x%d\n", temp_rs1);
      #endif
    }

  // This essentially checks if the alu src if the opcode uses rs2 (checcks validity of rs2)
    if ((read_opcode == 0x33) || (read_opcode == 0x23) || (read_opcode == 0x63)) {
        if (exmem_temp_rd == temp_rs2) {
          pwires_p->forwardB = 2;
          fwd_exex_counter += 1;

          #ifdef DEBUG_CYCLE
          printf("[FWD]: Resolving EX hazard on RS2: x%d\n", temp_rs2);
          #endif
        }
      }
  }
  // Check for MEM/WB stage now
  if (memwb_temp_reg_write && memwb_temp_rd != 0) {
    if ((memwb_temp_rd == temp_rs1) && (pwires_p->forwardA == 0)) {
      pwires_p->forwardA = 1;
      fwd_exmem_counter += 1;
      #ifdef DEBUG_CYCLE
      printf("[FWD]: Resolving MEM hazard on RS1: x%d\n", temp_rs1);
      #endif
    }
    if ((read_opcode == 0x33) || (read_opcode == 0x23) || (read_opcode == 0x63)) {
      if ((memwb_temp_rd == temp_rs2) && (pwires_p->forwardB == 0)) {
        pwires_p->forwardB = 1;
        fwd_exmem_counter += 1;
        #ifdef DEBUG_CYCLE

        printf("[FWD]: Resolving MEM hazard on RS2: x%d\n", temp_rs2);
        #endif
      }
    }
  }
}
/**
 * Task   : Sets the pipeline wires for the hazard unit's control signals
 *           based on the pipeline register values.
 * input  : pipeline_regs_t*, pipeline_wires_t*
 * output : None
 * Lex
 */
void detect_hazard(pipeline_regs_t *pregs_p, pipeline_wires_t *pwires_p, regfile_t *regfile_p) {
  gen_forward(pregs_p, pwires_p);
  uint8_t read_rs1 = 0;
  uint8_t read_rs2 = 0;
  bool read_rs1_usage = false;
  bool read_rs2_usage = false;
  bool load_use_hazard = false;
  bool load_mem_read = pregs_p->idex_preg.out.mem_read;

  // LOAD USE HAZARD
  // Reset stall and bubble signals at start
  pwires_p->idex_bubble_insert = false;
  pwires_p->stall_insert = false;
  pwires_p->flush_insert = false;  // Also reset flush

  // Extract current values
  ifid_reg_t ifid_data = pregs_p->ifid_preg.out;
  idex_reg_t idex_data = pregs_p->idex_preg.out;
  uint32_t load_rd = idex_data.write_rd;
  if (ifid_data.instr_bits == 0 && idex_data.instr_bits == 0 && idex_data.write_rd != 0 && idex_data.mem_read) {
    return false;
  }

    // initializing needed variables. We need the destination of data, and also to track if they are being used

    // Checks IF/ID instructions to see which registers are being used
    switch (ifid_data.instr.opcode) {
      case 0x33: //R type
        read_rs1 = ifid_data.instr.rtype.rs1;
        read_rs2 = ifid_data.instr.rtype.rs2;
        read_rs1_usage = true;
        read_rs2_usage = true;
        break;
      case 0x13: //I type without load
        read_rs1 = ifid_data.instr.itype.rs1;
        read_rs1_usage = true;
        break;
        // rs2 is just an immediate
      case 0x03: //Load
        read_rs1 = ifid_data.instr.itype.rs1;
        read_rs1_usage = true;
        break;
        // once again, rs2 is an immedaite
      case 0x23: //store
        read_rs1 = ifid_data.instr.stype.rs1;
        read_rs2 = ifid_data.instr.stype.rs2;
        read_rs1_usage = true;
        read_rs2_usage = true;
        break;
      case 0x63: //Branch
        read_rs1 = ifid_data.instr.sbtype.rs1;
        read_rs2 = ifid_data.instr.sbtype.rs2;
        read_rs1_usage = true;
        read_rs2_usage = true;
        break;
      case 0x67: //JALR
        read_rs1 = ifid_data.instr.itype.rs1;
        read_rs1_usage = true;
        break;
      default:
        break; //Everything else does not use a register
    }

  //     printf("Load-use hazard check:\n");
  // printf("IF/ID opcode: 0x%02x\n", ifid_data.instr.opcode);
  // printf("read_rs1 = x%d, read_rs2 = x%d\n", read_rs1, read_rs2);
  // printf("ID/EX mem_read = %d, write_rd = x%d\n", idex_data.mem_read, idex_data.write_rd);  

    // Hazard exist if the ifid_data.rd matches with any of the registered used, read_r#_usage is true and the provided register is not x0
      if ((read_rs1_usage && read_rs1 != 0 && read_rs1 == load_rd) || (read_rs2_usage && read_rs2 != 0 && read_rs2 == load_rd)) {
        load_use_hazard = true;
      }
    

    if (load_use_hazard) {
      pwires_p->idex_bubble_insert = true;
      pwires_p->stall_insert = true;
      stall_counter++;

      #ifdef DEBUG_CYCLE
      printf("[HZD]: Stalling and rewriting PC: 0x%08x\n", ifid_data.instr_addr);
      #endif
    }

  // Control Hazard
    if (gen_branch(idex_data.instr, read_rs1, read_rs2) && read_rs1 != 0 && read_rs2 != 0) { //Checks if the branch is taken and whether the compared registers are valid (aka, not x0)
      pwires_p->flush_insert = true;
    }
    
}
///////////////////////////////////////////////////////////////////////////////

/// RESERVED FOR PRINTING REGISTER TRACE AFTER EACH CLOCK CYCLE ///
void print_register_trace(regfile_t *regfile_p)
{
  // print
  for (uint8_t i = 0; i < 8; i++) // 8 columns
  {
    for (uint8_t j = 0; j < 4; j++) // of 4 registers each
    {
      printf("r%2d=%08x ", i * 4 + j, regfile_p->R[i * 4 + j]);
    }
    printf("\n");
  }
  printf("\n");
}

#endif // __STAGE_HELPERS_H__
