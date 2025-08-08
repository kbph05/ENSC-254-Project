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
  uint32_t instruction_bits = 0;
  
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
  if (!pwires_p->stall_insert) { //Decode must check if pc stalls
    regfile_p->PC += 4;
  }
  if (pwires_p->flush_insert) {
    ifid_reg.instr_bits = 0x00000013; //set to NOP
  } else {
    ifid_reg.instr_bits = instruction_bits;
  }

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

  if (pwires_p->stall_insert) {
    idex_reg = (idex_reg_t){0};
    idex_reg.instr_bits = 0x00000013;
    idex_reg.reg_write = false;
    idex_reg.mem_read = false;
    idex_reg.mem_write = false;
    idex_reg.mem_to_reg = false;
    idex_reg.branch = false;
    return idex_reg;
  }
  idex_reg = gen_control(ifid_reg.instr); // set control values
  idex_reg.instr_addr = ifid_reg.instr_addr; 
  idex_reg.instr = ifid_reg.instr;
  idex_reg.read_rs1 = regfile_p->R[ifid_reg.instr.rtype.rs1];  // or appropriate instruction format
  idex_reg.read_rs2 = regfile_p->R[ifid_reg.instr.rtype.rs2];
  idex_reg.read_imm = gen_imm(ifid_reg.instr); // generate an imm value
  idex_reg.instr_bits = ifid_reg.instr_bits; // transfer instruction bits to next stage for debug cycle
  idex_reg.pc = ifid_reg.pc; // set PC to PC from fetch stage
  
  // if we are writing back to reg file we need to save this value for the writeback stage
  switch (ifid_reg.instr.opcode) {
    case 0x33:  // R-type
      idex_reg.write_rd = ifid_reg.instr.rtype.rd;
      break;
    case 0x13:  // I-type
      idex_reg.write_rd = ifid_reg.instr.itype.rd;
      break;
    case 0x03:  // Load-type
      idex_reg.write_rd = ifid_reg.instr.itype.rd;
      break;
    case 0x6F:  // JAL
      idex_reg.write_rd = ifid_reg.instr.ujtype.rd;
      break;
    default:
      idex_reg.write_rd = 0;  // No write register
  }

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
 
  if (pwires_p->flush_insert) {
    exmem_reg.instr_bits = 0x00000013; //NOP
  } else {
    exmem_reg.instr_bits = idex_reg.instr_bits;
  }
  uint32_t alu_control_signal; // control signal for the alu unit
  uint32_t alu_inp2; // second input (could be imm or rs2)

  exmem_reg.instr_bits = idex_reg.instr_bits;  
  exmem_reg.write_rd = idex_reg.write_rd; 
  exmem_reg.instr = idex_reg.instr;
  exmem_reg.instr_addr = idex_reg.instr_addr;
  exmem_reg.pc = idex_reg.pc; 
  alu_control_signal = gen_alu_control(idex_reg);
  

  /*
  MS1:
    if (idex_reg.alu_op) {alu_control_signal = gen_alu_control(idex_reg);} // if alu_op is 1 then generate the alu_control signal needed for the operation
    if (idex_reg.alu_src) {alu_inp2 = idex_reg.read_imm;} else {alu_inp2 = idex_reg.read_rs2;} // determine the value for one of the inputs
  */

  // -- Forwarding conditions --

  // switch cases for rs1:
  switch(pwires_p->forwardA) {
    case 0:
      exmem_reg.read_rs1 = idex_reg.read_rs1; // no forwarding
      break;
    case 1:
      exmem_reg.read_rs1 = pwires_p->memwb_write_rd; // set the memwb_write data to the rs1 register
      break;
    case 2:
      exmem_reg.read_rs1 = pwires_p->exmem_alu_result; // set the rs1 to the alu result from previous result
      break;
    default:
      break;
  }

  // switch cases for rs2:
  switch(pwires_p->forwardB) {
    case 0:
      alu_inp2 = idex_reg.read_rs2; // no forwarding
      break;
    case 1:
      alu_inp2 = pwires_p->memwb_write_rd; // set the the rs1 register to the memwb_write_data i
      break;
    case 2:
      alu_inp2 = pwires_p->exmem_alu_result; // set the rs2 to the alu result from previous result
      break;
    default:
      break;
  }

  // cases for instructions that dont use the ALU, and if they do they just follow the default:
  switch(exmem_reg.instr.opcode) {
    case 0x37: // LUI
      exmem_reg.alu_result = idex_reg.read_imm << 12; 
      break;
    case 0x6F: // JAL
      exmem_reg.alu_result = idex_reg.instr_addr + 4;
      pwires_p->pc_src1 = idex_reg.instr_addr + idex_reg.read_imm;
      pwires_p->pcsrc = 1;
      branch_counter++; // JAL is another type of branch
      break;
    default:
       // Ensure we use the forwarded rs1 (stored in exmem_reg.read_rs1) and the chosen alu_inp2
      exmem_reg.alu_result = execute_alu(exmem_reg.read_rs1, alu_inp2, alu_control_signal);
      break;
  }

  // these are all the control signals needed for next stages of cpu
  exmem_reg.mem_read = idex_reg.mem_read; // transfer to memory stage
  exmem_reg.mem_write = idex_reg.mem_write; // transfer to memory stage
  exmem_reg.mem_to_reg = idex_reg.mem_to_reg; // transfer for WB stage
  exmem_reg.reg_write = idex_reg.reg_write; // transfer for WB stage
  exmem_reg.write_rd = idex_reg.write_rd;

  // for forwarding in future, store the actual value that will be written back (either loaded data or ALU result)
  pwires_p->exmem_alu_result = exmem_reg.alu_result;

  if (idex_reg.branch && idex_reg.instr.opcode == 0x63) {
    bool branch = false;
    switch(idex_reg.instr.sbtype.funct3) {
      case 0x0: // BEQ
        branch = (exmem_reg.read_rs1 == exmem_reg.read_rs2);
        break;
      case 0x1: // BNE
        branch = (exmem_reg.read_rs1 != exmem_reg.read_rs2);
        break;
    }
  }

  // branch condition for b-type instructions
  if (gen_branch(idex_reg.instr, idex_reg.read_rs1, idex_reg.read_rs2)) {
    pwires_p->pc_src1 = (uint32_t)(idex_reg.read_imm + idex_reg.instr_addr);
    pwires_p->pcsrc = true;
    branch_counter++;
  }


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
  if (pwires_p->flush_insert) {
    memwb_reg.instr_bits = 0x00000013;
  } else {
    memwb_reg.instr_bits = exmem_reg.instr_bits;
  }
  memwb_reg.pc = exmem_reg.pc;


  // transfer data from last cycle:
  memwb_reg.instr_addr = exmem_reg.instr_addr;
  memwb_reg.alu_result = exmem_reg.alu_result;
  memwb_reg.write_rd = exmem_reg.write_rd;
  memwb_reg.read_rs2 = exmem_reg.read_rs2;
  memwb_reg.instr = exmem_reg.instr;
  memwb_reg.pc = exmem_reg.pc;

  // transfer control signals from last cycle
  memwb_reg.reg_write = exmem_reg.reg_write;
  memwb_reg.mem_to_reg = exmem_reg.mem_to_reg;

  // For forwarding, store the actual value that will be written back (either loaded data or ALU result)
  int wb_data = memwb_reg.mem_to_reg ? (int)memwb_reg.mem_read : (int)memwb_reg.alu_result;
  pwires_p->memwb_write_rd = wb_data;


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
  if (memwb_reg.reg_write && memwb_reg.write_rd != 0) {
    if (memwb_reg.mem_to_reg) {
      regfile_p->R[memwb_reg.write_rd] = memwb_reg.mem_read; // write loaded memory data
    } else {
      regfile_p->R[memwb_reg.write_rd] = memwb_reg.alu_result; // write ALU result
    }
  }

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
   
  detect_hazard(pregs_p, pwires_p, regfile_p);
  
  pregs_p->idex_preg.inp  = stage_decode    (pregs_p->ifid_preg.out, pwires_p, regfile_p);

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
  if((pregs_p->memwb_preg.out.instr_bits == 0x00000073) && (regfile_p->R[10] == 10)) 
  {
    *(ecall_exit) = true;
  }
}

