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
uint32_t gen_alu_control(idex_reg_t idex_reg)
{
  uint32_t alu_control = 0;
  /**
   * YOUR CODE HERE
   */
  return alu_control;
}

/**
 * input  : alu_inp1, alu_inp2, alu_control
 * output : uint32_t alu_result
 * Lex
 **/
uint32_t execute_alu(uint32_t alu_inp1, uint32_t alu_inp2, uint32_t alu_control) {
  uint32_t result;
  switch (alu_control)
  {
  case 0x0: // add
    result = alu_inp1 + alu_inp2;
    break;
  /**
   * YOUR CODE HERE
   */
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
  /**
   * YOUR CODE HERE
   */
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
  /**
   * YOUR CODE HERE
   */
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
  switch (instruction.opcode)
  {
  case 0x33: // R-type
    /**
     * YOUR CODE HERE
     */
    idex_reg.read_rs1 = instruction.rtype.rs1;
    idex_reg.read_rs2 = instruction.rtype.rs2;
    break;
  case 0x23: // S-type
    idex_reg.read_rs1 = instruction.stype.rs1;
    idex_reg.read_rs2 = instruction.stype.rs2;
    break;
  case 0x13: // I-type
    idex_reg.read_rs1 = instruction.itype.rs1;
    break;
  case 0x63: // B-type
    idex_reg.read_rs1 = instruction.sbtype.rs1;
    idex_reg.read_rs2 = instruction.sbtype.rs2;
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
  if (instruction.opcode == 0x0) {
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
  /**
   * YOUR CODE HERE
   */
  return false;
}

/// PIPELINE FEATURES ///

/**
 * Task   : Sets the pipeline wires for the forwarding unit's control signals
 *           based on the pipeline register values.
 * input  : pipeline_regs_t*, pipeline_wires_t*
 * output : None
 */
void gen_forward(pipeline_regs_t *pregs_p, pipeline_wires_t *pwires_p)
{
  /**
   * YOUR CODE HERE
   */
}

/**
 * Task   : Sets the pipeline wires for the hazard unit's control signals
 *           based on the pipeline register values.
 * input  : pipeline_regs_t*, pipeline_wires_t*
 * output : None
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
