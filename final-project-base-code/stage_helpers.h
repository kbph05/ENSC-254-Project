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
              alu_control = 0x04; // mulh, S * U
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
          case 0x01: // multiply unsigned
            alu_control = 0x4;
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
            alu_control = 0x01;
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
              alu_control = 0x7;
              break;
            default:
              alu_control = 0xBADCAFFE;
              break;
          }
        case 0x5:
          switch (idex_reg.read_imm) {
            case 0x0:
              break;
            case 0x20:
              alu_control = 0x8;
              break;
            default:
              alu_control = 0xBADCAFFE;
              break;
          }
      //case 0x2:
        case 0x3: // (U), zero extend
          alu_control = 0x9;
          break;
        default:
          alu_control = 0xBADCAFFE;
          break;
      }
  // case 0x3: //I type load
  // case 0x6F: // JAL
  // case 0x67: // JALR
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
  return alu_control;
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
    // idk if legal, consult -> yes instead use shifting to change (its a 32 binary number)
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
      idex_reg.read_rs1 = instruction.rtype.rs1;
      idex_reg.read_rs2 = instruction.rtype.rs2;
      idex_reg.read_funct3 = instruction.rtype.funct3; // yes these are correct!
      idex_reg.read_funct7 = instruction.rtype.funct7;
      idex_reg.read_opcode = instruction.opcode;
      break;
    case 0x23: // S-type
      idex_reg.read_rs1 = instruction.stype.rs1;
      idex_reg.read_rs2 = instruction.stype.rs2;
      idex_reg.read_funct3 = instruction.stype.funct3;
      idex_reg.read_opcode = instruction.opcode;
      break;
    case 0x13: // I-type
      idex_reg.read_rs1 = instruction.itype.rs1;
      idex_reg.read_funct3 = instruction.itype.funct3;
      idex_reg.read_opcode = instruction.itype.opcode;
      break;
    case 0x63: // B-type
      idex_reg.read_rs1 = instruction.sbtype.rs1;
      idex_reg.read_rs2 = instruction.sbtype.rs2;
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
bool gen_branch(Instruction instruction, int PC) {
  // if instruction is a branch instruction
  if (instruction.opcode == 0x63) {
    switch (instruction.sbtype.funct3) {
      case 0x0:
        if (instruction.sbtype.rs1 == instruction.sbtype.rs2) {
          // increment pc by instruction.sbtype.imm branch offset
          PC += get_branch_offset(instruction);
        }
        break;
      case 0x1:
        if (instruction.sbtype.rs1 != instruction.sbtype.rs2) {
          // increment pc by instruction.sbtype.imm branch offset
          PC += get_branch_offset(instruction);
        }
        break;
      case 0x4:
        if (instruction.sbtype.rs1 < instruction.sbtype.rs2) {
          // increment pc by instruction.sbtype.imm branch offset
          PC += get_branch_offset(instruction);
        }
        break;
      case 0x5:
        if (instruction.sbtype.rs1 >= instruction.sbtype.rs2) {
          // increment pc by instruction.sbtype.imm branch offset
          PC += get_branch_offset(instruction);
        }
        break;
      case 0x6:
        if (instruction.sbtype.rs1 < instruction.sbtype.rs2) {
          // increment pc by instruction.sbtype.imm branch offset (zero extends)
          PC += sign_extend_number(get_branch_offset(instruction), 32);
        }
        break;
      case 0x7:
        if (instruction.sbtype.rs1 >= instruction.sbtype.rs2) {
          // increment pc by instruction.sbtype.imm branch offset (zero extends)
          PC += sign_extend_number(get_branch_offset(instruction), 32);
        }
        break;
      default:
        break;
    }
    return true;
  }
  return false;
}

/// PIPELINE FEATURES ///

/**
 * Task   : Sets the pipeline wires for the forwarding unit's control signals
 *           based on the pipeline register values.
 * input  : pipeline_regs_t*, pipeline_wires_t*
 * output : None
 * Kirstin
 */
void gen_forward(pipeline_regs_t *pregs_p, pipeline_wires_t *pwires_p)
{
  /**
   * YOUR CODE HERE
   */
  // connect pwires from one stage to another stage

}

/**
 * Task   : Sets the pipeline wires for the hazard unit's control signals
 *           based on the pipeline register values.
 * input  : pipeline_regs_t*, pipeline_wires_t*
 * output : None
 * Lex
 */
void detect_hazard(pipeline_regs_t *pregs_p, pipeline_wires_t *pwires_p, regfile_t *regfile_p)
{
  /**
   * YOUR CODE HERE
   */
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
