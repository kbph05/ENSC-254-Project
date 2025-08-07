#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include "config.h"
#include "types.h"
#include "cache.h"
#include <stdbool.h>

///////////////////////////////////////////////////////////////////////////////
/// Functionality
///////////////////////////////////////////////////////////////////////////////

extern simulator_config_t sim_config;
extern uint64_t miss_count;
extern uint64_t hit_count;
extern uint64_t total_cycle_counter;
extern uint64_t stall_counter;
extern uint64_t branch_counter;
extern uint64_t fwd_exex_counter;
extern uint64_t fwd_exmem_counter;

///////////////////////////////////////////////////////////////////////////////
/// RISC-V Pipeline Register Types (encapsulates the data passed between two successive pipeline stages)
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
  Instruction instr;
  uint32_t instr_addr;
  uint32_t instr_bits;
  unsigned int write_rs1; // write address for rs1
  unsigned int write_rs2; // write address for rs2
  unsigned int write_rd;
  unsigned int write_imm;
  unsigned int pc;

}ifid_reg_t;

// Kirstin
typedef struct
{
  Instruction  instr;
  uint32_t instr_addr;
  uint32_t instr_bits;
  unsigned int read_rs1;
  unsigned int read_rs2;
  unsigned int read_funct7;
  unsigned int read_funct3;
  unsigned int write_rd;
  unsigned int read_opcode;
  uint32_t read_imm;
  unsigned int pc;
  
  // next stage:
  bool alu_op;
  bool alu_src;

  bool mem_write;
  bool mem_to_reg;
  bool mem_read;
  bool reg_write;

}idex_reg_t;

typedef struct
{
  Instruction instr;
  uint32_t instr_addr;
  uint32_t instr_bits;
  unsigned int alu_result; //to store the computation results
  unsigned int write_rd; // to store the address of rd for regfile
  unsigned int read_rs1;
  unsigned int read_rs2;
  unsigned int pc;

  // next stage:
  bool mem_read;
  bool mem_write;
  bool branch;

  bool mem_to_reg;
  bool reg_write;

}exmem_reg_t;

// Kirstin
typedef struct
{
  Instruction instr;
  uint32_t instr_bits;
  uint32_t instr_addr;

  unsigned int alu_result;
  unsigned int mem_read; //rename
  unsigned int pc;
  unsigned int read_rs2;
  unsigned int write_rd;

  // next stage:
  bool mem_to_reg;
  bool reg_write;

}memwb_reg_t;


///////////////////////////////////////////////////////////////////////////////
/// Register types with input and output variants for simulator (contains a pair of the pipeline registers, ‘inp’ and 'out')
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
  ifid_reg_t inp;
  ifid_reg_t out;
}ifid_reg_pair_t;

typedef struct
{
  idex_reg_t inp;
  idex_reg_t out;
}idex_reg_pair_t;

typedef struct
{
  exmem_reg_t inp;
  exmem_reg_t out;
}exmem_reg_pair_t;

typedef struct
{
  memwb_reg_t inp;
  memwb_reg_t out;
}memwb_reg_pair_t;

///////////////////////////////////////////////////////////////////////////////
/// Functional pipeline requirements
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
  ifid_reg_pair_t  ifid_preg;
  idex_reg_pair_t  idex_preg;
  exmem_reg_pair_t exmem_preg;
  memwb_reg_pair_t memwb_preg;
}pipeline_regs_t;

typedef struct
{
  bool pcsrc; // manages the multiplexer with PC + 4  (are we branching or just going to next instruction) 
  uint32_t pc_src0; // adds 4 to PC (for when the next instruction is to be fetched)
  uint32_t pc_src1; // PC += imm (yes branch or j-type)

  int forwardA;
  int forwardB;
  bool stall_insert = false;
  bool idex_bubble_insert = false;
  bool flush_insert = false;

}pipeline_wires_t;


///////////////////////////////////////////////////////////////////////////////
/// Function definitions for different stages
///////////////////////////////////////////////////////////////////////////////

/**
 * output : ifid_reg_t
 **/ 
ifid_reg_t stage_fetch(pipeline_wires_t* pwires_p, regfile_t* regfile_p, Byte* memory_p);

/**
 * output : idex_reg_t
 **/ 
idex_reg_t stage_decode(ifid_reg_t ifid_reg, pipeline_wires_t* pwires_p, regfile_t* regfile_p);

/**
 * output : exmem_reg_t
 **/ 
exmem_reg_t stage_execute(idex_reg_t idex_reg, pipeline_wires_t* pwires_p);

/**
 * output : memwb_reg_t
 **/ 
memwb_reg_t stage_mem(exmem_reg_t exmem_reg, pipeline_wires_t* pwires_p, Byte* memory, Cache* cache_p);

/**
 * output : write_data
 **/ 
void stage_writeback(memwb_reg_t memwb_reg, pipeline_wires_t* pwires_p, regfile_t* regfile_p);

void cycle_pipeline(regfile_t* regfile_p, Byte* memory_p, Cache* cache_p, pipeline_regs_t* pregs_p, pipeline_wires_t* pwires_p, bool* ecall_exit);

void bootstrap(pipeline_wires_t* pwires_p, pipeline_regs_t* pregs_p, regfile_t* regfile_p);

#endif  // __PIPELINE_H__
