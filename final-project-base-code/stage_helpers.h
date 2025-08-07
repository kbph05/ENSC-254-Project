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
          break;
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
          break;
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
          break; 
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
          break;
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
          break;
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
          break;
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
          break;
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
          break;
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
          switch (idex_reg.instr.itype.imm >> 5 & 0x7F) { //potential error
            case 0x0:
              alu_control = 0xC;
              break;
            default:
              alu_control = 0xBADCAFFE;
              break;
          }
          break;
        case 0x5:
          switch (idex_reg.instr.itype.imm >> 5 & 0x7F) {
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
          break;
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
      break;
  // case 0x3: break;//I type load
  // case 0x6F: break;// JAL
  // case 0x67: break;// JALR
    case 0x23: break;// store
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
    break;
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
      break;
    case 0x23: // S-type
      idex_reg.read_funct3 = instruction.stype.funct3;
      idex_reg.read_opcode = instruction.opcode;
      break;
    case 0x13: // I-type
      idex_reg.read_funct3 = instruction.itype.funct3;
      idex_reg.read_opcode = instruction.itype.opcode;
      break;
    case 0x63: // B-type
      idex_reg.read_funct3 = instruction.sbtype.funct3;
      idex_reg.read_opcode = instruction.sbtype.opcode;
      break;
    default: // Remaining opcodes
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

  pregs_p->exmem_preg.out.write_rd, pregs_p->idex_preg.out.read_rs1;
  
  // First check if the function is using rs1 or rs2
  uint8_t temp_rs1 = 0;
  uint8_t temp_rs2 = 0;
  uint32_t read_opcode = pregs_p->idex_preg.out.instr.opcode;

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
    case 0x6F: // JAL
      temp_rs1 = pregs_p->idex_preg.out.instr.itype.rs1;
      break;
    case 0x67: // JALR
      temp_rs1 = pregs_p->idex_preg.out.instr.itype.rs1;
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
        fprintf("[FWD]: Resolving EX hazard on RS1: x&d\n", temp_rs1);
      #endif
    }

    // This essentially checks if the alu src if the opcode uses rs2 (checcks validity of rs2)
    if ((read_opcode == 0x33) || (read_opcode == 0x23) || (read_opcode == 0x63)) {
      if (exmem_temp_rd == temp_rs2) {
        pwires_p->forwardB = 2;
        fwd_exex_counter += 1;
        #ifdef DEBUG_CYCLE
        fprintf("[FWD]: Resolving EX hazard on RS2: x&d\n", temp_rs2);
        #endif
      }
    }
  }
  // Check for MEM/WB stage now
  if (memwb_temp_reg_write != 0 && memwb_temp_rd != 0) {
    if ((memwb_temp_rd == temp_rs1) && (pwires_p->forwardA == 0)) {
      pwires_p->forwardA = 1;
    }
    if ((read_opcode == 0x33) || (read_opcode == 0x23) || (read_opcode == 0x63)) {
      if ((memwb_temp_rd == temp_rs2) && (pwires_p->forwardB == 0)) {
        pwires_p->forwardB = 2;
        fwd_exex_counter += 1;
        #ifdef DEBUG_CYCLE
        fprintf("[FWD]: Resolving EX hazard on RS2: x&d\n", temp_rs2);
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
 */
void detect_hazard(pipeline_regs_t *pregs_p, pipeline_wires_t *pwires_p, regfile_t *regfile_p) 
{
  // Reset hazard control signals
  pwires_p->insert_bubble_idex = false;
  pwires_p->insert_bubble_ifid = false;


  // Get current instructions in each stage
  ifid_reg_t ifid = pregs_p->ifid_preg.out;
  idex_reg_t idex = pregs_p->idex_preg.out;
  
  /*---------------------------LOAD-USE HAZARD---------------------------*/
  
  /* If ID/EX is a load instruction (mem_read) &&  it writes to a register (rd != 0)
     check if load destination matches either source in IF/ID stage */


  /* detect hazard if
  - we have non zero instructions in both stages
  - the idex stage is a load instruction (uses mem_read)
  - the destination register is not x0 (rd != 0)
  */
  if (ifid.instr_bits != 0 && idex.instr_bits != 0 && idex.mem_read && idex.write_rd != 0) {
    
    uint8_t rs1 = 0, rs2 = 0;// initialize for source registers in IF/ID
    bool rs1_used = false, rs2_used = false; //flags to track if rs1/rs2 are used by the instruction
    
    // Determine which registers are used as sources in IF/ID stage
    switch (ifid.instr.opcode) {
      case 0x33: // R-type
        rs1 = ifid.instr.rtype.rs1;
        rs2 = ifid.instr.rtype.rs2;
        rs1_used = true;
        rs2_used = true;
        break;
      case 0x13: // I-type ALU
        rs1 = ifid.instr.itype.rs1;
        rs1_used = true;
        // rs2 is immediate, so not used
        break;
      case 0x03: // Load
        rs1 = ifid.instr.itype.rs1;
        rs1_used = true;
        // rs2 not used in load (its an immediate)
        break;
      case 0x23: // Store
        rs1 = ifid.instr.stype.rs1;
        rs2 = ifid.instr.stype.rs2;
        rs1_used = true;
        rs2_used = true;
        break;
      case 0x63: // Branch
        rs1 = ifid.instr.sbtype.rs1;
        rs2 = ifid.instr.sbtype.rs2;
        rs1_used = true;
        rs2_used = true;
        break;
      case 0x67: // JALR
        rs1 = ifid.instr.itype.rs1;
        rs1_used = true;
        // rs2 not used its an immediate (jump address)
        break;
      case 0x37: // LUI - no hazards for LUI
      case 0x6F: // JAL - no hazards for JAL
      default:
        // For other instructions, no hazard
        break;
    }
    

    /* this is rs1 and rs2 of the (IF/ID) stage being read and comparted to the current load in (ID/EX) stage
    if 
    - rs1/rs2 is being used (IF/ID)
    - rs1/rs2 is not x0
    - rs1/rs2 matches the destination register of the load in ID/EX (idex.rd)
    */
    if ((rs1_used && rs1 != 0 && rs1 == idex.write_rd) || (rs2_used && rs2 != 0 && rs2 == idex.write_rd)) {
      // Set stall and insert bubble flags
      pwires_p->insert_stall = true;
      pwires_p->insert_bubble_idex = true;
      stall_counter++; // Increment stall counter
      
      #ifdef DEBUG_CYCLE
      printf("[HZD]: Stalling and rewriting PC: 0x%08x\n", ifid.instr_addr);
      #endif
    }
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
