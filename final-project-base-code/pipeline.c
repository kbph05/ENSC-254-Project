#include <stdbool.h>
#include "cache.h"
#include "riscv.h"
#include "types.h"
#include "utils.h"
#include "pipeline.h"
#include "stage_helpers.h"

uint64_t total_cycle_counter = 0;
uint64_t miss_count = 0;
uint64_t hit_count = 0;
uint64_t stall_counter = 0;
uint64_t branch_counter = 0;
uint64_t fwd_exex_counter = 0;
uint64_t fwd_exmem_counter = 0;

simulator_config_t sim_config = {0};

///////////////////////////////////////////////////////////////////////////////

void bootstrap(pipeline_wires_t* pwires_p, pipeline_regs_t* pregs_p, regfile_t* regfile_p) {
  // PC src must get the same value as the default PC value
  pwires_p->pc_src0 = regfile_p->PC;
}

///////////////////////////
/// STAGE FUNCTIONALITY ///
///////////////////////////

/**
 * STAGE  : stage_fetch
 * output : ifid_reg_t
 **/ 
// Lex
ifid_reg_t stage_fetch(pipeline_wires_t* pwires_p, regfile_t* regfile_p, Byte* memory_p) {
  ifid_reg_t ifid_reg = {0};
  uint32_t instruction_bits;
  
  // need to find the address (this is the multiplexer before instruction memory)
  if (pwires_p->pcsrc == 0) {
    ifid_reg.pc = pwires_p->pc_src0;
    pwires_p->pc_src0 = pwires_p->pc_src0 + 4;
  }
  else if (pwires_p->pcsrc == 1) {
    ifid_reg.pc = pwires_p->pc_src1;
    pwires_p->pc_src0 = pwires_p->pc_src1 + 4;
  }
  
  // getting the address for getting instruction from memory
  ifid_reg.instr_addr = ifid_reg.pc; 

  if (ifid_reg.pc < MEMORY_SPACE - 3) {
    instruction_bits = (memory_p[ifid_reg.pc + 3] << 24) |
                      (memory_p[ifid_reg.pc + 2] << 16) |
                      (memory_p[ifid_reg.pc+ 1] << 8) |
                      (memory_p[ifid_reg.pc]);
  }

  if (instruction_bits == 0) {
    instruction_bits = 0x00000013; // NOP instruction
  }

  ifid_reg.instr = parse_instruction(instruction_bits); // parse the instruction bits into an Instruction struct

  // if (!pwires_p->stall) { //Decode must check if pc stalls
  //   regfile_p->PC += 4;
  //   stall_counter++;
  // }
  ifid_reg.instr_bits = instruction_bits;

  #ifdef DEBUG_CYCLE
  printf("[IF ]: Instruction [%08x]@[%08x]: ", instruction_bits, ifid_reg.pc);
  decode_instruction(instruction_bits);
  #endif
  return ifid_reg;
}

/**
 * STAGE  : stage_decode
 * output : idex_reg_t
 **/ 
// Kirstin
idex_reg_t stage_decode(ifid_reg_t ifid_reg, pipeline_wires_t* pwires_p, regfile_t* regfile_p) {
  idex_reg_t idex_reg = {0};

  idex_reg = gen_control(ifid_reg.instr); // set control values
  idex_reg.read_rs1 = regfile_p->R[ifid_reg.write_rs1]; // pull rs1 from regfile
  idex_reg.read_rs2 = regfile_p->R[ifid_reg.write_rs2]; // pull rs2 from regfile
  idex_reg.read_imm = gen_imm(ifid_reg.instr); // generate an imm value
  idex_reg.instr_bits = ifid_reg.instr_bits; // transfer instruction bits to next stage for debug cycle
  idex_reg.pc = ifid_reg.pc; // set PC to PC from fetch stage

  // if we are writing back to reg file we need to save this value for the writeback stage
  idex_reg.write_rd = ifid_reg.instr.itype.rd;

  #ifdef DEBUG_CYCLE
  printf("[ID ]: Instruction [%08x]@[%08x]: ", ifid_reg.instr_bits, ifid_reg.pc);
  decode_instruction(ifid_reg.instr_bits);
  #endif
  return idex_reg;
}

/**
 * STAGE  : stage_execute
 * output : exmem_reg_t
 **/
// Lex
exmem_reg_t stage_execute(idex_reg_t idex_reg, pipeline_wires_t* pwires_p) {
  exmem_reg_t exmem_reg = {0};

  exmem_reg.instr_bits = idex_reg.instr_bits;


  idex_reg.alu_op = gen_alu_control(idex_reg);
  if (idex_reg.read_opcode == 0x63) {
    gen_branch
  }
  exmem_reg.result = (alu_src) ? execute_alu(idex_reg.read_rs1, idex_reg.read_imm, idex_reg.alu_op) : execute_alu(idex_reg.read_rs1, idex_reg.read_rs2, idex_reg.alu_op);
  
  switch (idex_reg.read_opcode) {
    case 0x33: // Rtype
      idex_reg.reg_write = 1; // should cascade down
      // intentional fall through
    case 0x13: // I type ALU
    case 0x37: // LUI
    case 0x6F: // jal
    case 0x67: //JALR
      idex_reg.mem_to_reg = 0;
      break;
    case 0x03: // I type load
      idex_reg.mem_to_reg = 1;
      idex_reg.mem_read = 1;
      idex_reg.reg_write = 1;
      break;
    case 0x23: //S type
      idex_reg.mem_write = 1;
      break;
    default: // idk what to put for error code
      fprintf(stderr, "Unrecognized Code: 0x%02X\n", idex_reg.read_opcode); // took from internet
      break;
  }


  // switch (idex_reg.read_opcode) {
  //   case 0x33:
  //     exmem_reg.result = execute_alu(idex_reg.read_rs1, idex_reg.read_rs2, control_thing);
  //     break;
  //   case 0x13:
  //     // Might need a system to isolate just imm[0:4]
  //     break;
  //   case 0x03: // meant to write to write_addr for load
  //   case 0x23: //meant to write to write_addr for store
  //   case 0x6F: // meant to write to write_addr for JAL
  //   case 0x67: // meant to write to write_addr for JALR
  //     exmem_reg.write_addr = execute_alu(idex_reg.read_rs1, idex_reg.read_imm, control_thing);
  //     break;
  //   case 0x63:
  //     exmem_reg.write_addr = execute_alu(idex_reg.read_rs1, idex_reg.read_rs2, control_thing); // may need to implement the logic for branch
  //     break;
  //   default:
  //     break; // idk what to do here
  // }
  exmem_reg.pc = idex_reg.pc;
  // if (extend == true) {
  //   exmem_reg = sign_extend_number(exmem_reg, size(exmem_reg));
  // }

  #ifdef DEBUG_CYCLE
  printf("[EX ]: Instruction [%08x]@[%08x]: ", exmem_reg.instr_bits, exmem_reg.pc);
  decode_instruction(exmem_reg.instr_bits);
  #endif

  return exmem_reg;
}

/**
 * STAGE  : stage_mem
 * output : memwb_reg_t
 **/ 
// Kirstin
memwb_reg_t stage_mem(exmem_reg_t exmem_reg, pipeline_wires_t* pwires_p, Byte* memory_p, Cache* cache_p) {
  memwb_reg_t memwb_reg = {0};

  // for the debug cycle
  memwb_reg.instr_bits = exmem_reg.instr_bits;
  memwb_reg.pc = exmem_reg.pc;

  // transfer data from last cycle:
  memwb_reg.instr_addr = exmem_reg.instr_addr;
  memwb_reg.alu_result = exmem_reg.alu_result;
  memwb_reg.write_rd = exmem_reg.write_rd;
  memwb_reg.read_rs2 = exmem_reg.read_rs2;

  // transfer control signals from last cycle
  memwb_reg.reg_write = exmem_reg.reg_write;
  memwb_reg.mem_to_reg = exmem_reg.mem_to_reg;
  
  // get the alignment for word value in memory
  Alignment alignment;
  switch (exmem_reg.instr.itype.funct3) {
    case 0x0:
      alignment = LENGTH_BYTE;
      break;
    case 0x1:
      alignment = LENGTH_HALF_WORD;
      break;
    case 0x2:
      alignment = LENGTH_WORD;
      break;
    default: 
      break; 
  }

  // from the control in execute stage:
  if (exmem_reg.mem_write) {store(memory_p, memwb_reg.read_rs2, alignment, memwb_reg.alu_result);} // store in memory if the control signal is 1
  if (exmem_reg.mem_read) {memwb_reg.mem_read = load(memory_p, memwb_reg.alu_result, alignment); } // load from memory into mem_write if control signal is 1
  if (exmem_reg.branch) {pwires_p->pcsrc = 1;} else {pwires_p->pcsrc = 0;} // when pcsrc = 1 then its a branch otherwise its not a branch 


  #ifdef DEBUG_CYCLE
  printf("[MEM ]: Instruction [%08x]@[%08x]: ", memwb_reg.instr_bits, memwb_reg.pc);
  decode_instruction(memwb_reg.instr_bits);
  #endif

  return memwb_reg;
}

/**
 * STAGE  : stage_writeback
 * output : nothing - The state of the register file may be changed
 **/ 
// Kirstin
void stage_writeback(memwb_reg_t memwb_reg, pipeline_wires_t* pwires_p, regfile_t* regfile_p) {

  if (memwb_reg.mem_to_reg) {regfile_p->R[memwb_reg.write_rd] = memwb_reg.alu_result;} else {regfile_p->R[memwb_reg.write_rd] = memwb_reg.instr_addr;}
  
  #ifdef DEBUG_CYCLE
  printf("[WB ]: Instruction [%08x]@[%08x]: ", memwb_reg.instr_bits, memwb_reg.pc);
  decode_instruction(memwb_reg.instr_bits);
  #endif
  
}

///////////////////////////////////////////////////////////////////////////////

/** 
 * excite the pipeline with one clock cycle
 **/
void cycle_pipeline(regfile_t* regfile_p, Byte* memory_p, Cache* cache_p, pipeline_regs_t* pregs_p, pipeline_wires_t* pwires_p, bool* ecall_exit) {
  #ifdef DEBUG_CYCLE
  printf("v==============");
  printf("Cycle Counter = %5ld", total_cycle_counter);
  printf("==============v\n\n");
  #endif

  // process each stage

  /* Output               |    Stage      |       Inputs  */
  pregs_p->ifid_preg.inp  = stage_fetch     (pwires_p, regfile_p, memory_p);
  
  pregs_p->idex_preg.inp  = stage_decode    (pregs_p->ifid_preg.out, pwires_p, regfile_p);

  detect_hazard(pregs_p, pwires_p, regfile_p);

  pregs_p->exmem_preg.inp = stage_execute   (pregs_p->idex_preg.out, pwires_p);
  
  pregs_p->memwb_preg.inp = stage_mem       (pregs_p->exmem_preg.out, pwires_p, memory_p, cache_p);

                            stage_writeback (pregs_p->memwb_preg.out, pwires_p, regfile_p);

  // update all the output registers for the next cycle from the input registers in the current cycle
  pregs_p->ifid_preg.out  = pregs_p->ifid_preg.inp;
  pregs_p->idex_preg.out  = pregs_p->idex_preg.inp;
  pregs_p->exmem_preg.out = pregs_p->exmem_preg.inp;
  pregs_p->memwb_preg.out = pregs_p->memwb_preg.inp;

  /////////////////// NO CHANGES BELOW THIS ARE REQUIRED //////////////////////

  // increment the cycle
  total_cycle_counter++;

  #ifdef DEBUG_REG_TRACE
  print_register_trace(regfile_p);
  #endif

  /**
   * check ecall condition
   * To do this, the value stored in R[10] (a0 or x10) should be 10.
   * Hence, the ecall condition is checked by the existence of following
   * two instructions in sequence:
   * 1. <instr>  x10, <val1>, <val2> 
   * 2. ecall
   * 
   * The first instruction must write the value 10 to x10.
   * The second instruction is the ecall (opcode: 0x73)
   * 
   * The condition checks whether the R[10] value is 10 when the
   * `memwb_reg.instr.opcode` == 0x73 (to propagate the ecall)
   * 
   * If more functionality on ecall needs to be added, it can be done
   * by adding more conditions on the value of R[10]
   */
  if( (pregs_p->memwb_preg.out.instr.bits == 0x00000073) &&
      (regfile_p->R[10] == 10) )
  {
    *(ecall_exit) = true;
  }
}

