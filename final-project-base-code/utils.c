#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/* Unpacks the 32-bit machine code instruction given into the correct
 * type within the instruction struct */

//// Kirstin ////
Instruction parse_instruction(uint32_t instruction_bits) {
  /* YOUR CODE HERE */

  Instruction instruction;
  // add x9, x20, x21   hex: 01 5A 04 B3, binary = 0000 0001 0101 1010 0000 0100 1011 0011
  // Opcode: 0110011 (0x33) Get the Opcode by &ing 0x1111111, bottom 7 bits
  instruction.opcode = instruction_bits & ((1U << 7) - 1);

  // Shift right to move to pointer to interpret next fields in instruction.
  instruction_bits >>= 7;

  switch (instruction.opcode) {
  // R-Type
  case 0x33:
    // instruction: 0000 0001 0101 1010 0000 0100 1, destination : 01001
    instruction.rtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 0001 0101 1010 0000, func3 : 000
    instruction.rtype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    // instruction: 0000 0001 0101 1010 0, src1: 10100
    instruction.rtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 0001 0101, src2: 10101
    instruction.rtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // funct7: 0000 000
    instruction.rtype.funct7 = instruction_bits & ((1U << 7) - 1);
    break;
  // cases for other types of instructions
  /* YOUR CODE HERE */

  // I-Type
  case 0x13:

    // 0000 0001 0101 1010 0000 0100 1
    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101 1010 0000
    instruction.itype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3 ;

    // 0000 0001 0101 1010 0
    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101
    instruction.itype.imm = instruction_bits & ((1U << 12) - 1);
  break;

  // I-Type
  case 0x3:

    // 0000 0001 0101 1010 0000 0100 1
    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101 1010 0000
    instruction.itype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    // 0000 0001 0101 1010 0
    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101
    instruction.itype.imm = instruction_bits & ((1U << 12) - 1);;

  break;

  // I-Type
  case 0x73:

  // 0000 0001 0101 1010 0000 0100 1
    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101 1010 0000
    instruction.itype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;
    
    // 0000 0001 0101 1010 0
    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101
    instruction.itype.imm = instruction_bits & ((1U << 12) - 1);

  break;

  // S-Type
  case 0x23:

    // 0000 0001 0101 1010 0000 0100 1
    instruction.stype.imm5 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101 1010 0000
    instruction.stype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    // 0000 0001 0101 1010 0
    instruction.stype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101
    instruction.stype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 000
    instruction.stype.imm7 = instruction_bits & ((1U << 7) - 1);
    
  break;


  // SB-Type
  case 0x63:

    // 0000 0001 0101 1010 0000 0100 1
    instruction.sbtype.imm5 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101 1010 0000
    instruction.sbtype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    // 0000 0001 0101 1010 0
    instruction.sbtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101
    instruction.sbtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 000
    instruction.sbtype.imm7 = instruction_bits & ((1U << 7) - 1);

  break;

  // U-Type
  case 0x37:

    //0000 0001 0101 1010 0000 0100 1
    instruction.utype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101 1010 0000
    instruction.utype.imm = instruction_bits & ((1U << 20) - 1);

  break;

  // UJ-Type
  case 0x6F:

    // 0000 0001 0101 1010 0000 0100 1
    instruction.ujtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // 0000 0001 0101 1010 0000
    instruction.ujtype.imm = instruction_bits & ((1U << 20) - 1);
  
  break;


  #ifndef TESTING
  default:
    exit(EXIT_FAILURE);
  #endif
  }
  return instruction;
}

/************************Helper functions************************/
/* Here, you will need to implement a few common helper functions, 
 * which you will call in other functions when parsing, printing, 
 * or executing the instructions. */

/* Sign extends the given field to a 32-bit integer where field is
 * interpreted an n-bit integer. */

int sign_extend_number(unsigned int field, unsigned int n) {
  /* YOUR CODE HERE */

  // Shift the bits left by 32 - n to get the MSB at position 31
  int shift1 = (field << (32 - n));

  // Sign cast the shift so that when its shifted back to the right it will shift right arithmetrically
  int signCast = (int)shift1;

  // Shift the bits back to the right by 32. It will shift right arithmetrically, so depending on the signed bit, the new bits will be 1 or 0.
  int shift2 = (signCast >> (32 - n));

  return shift2;
}

/* Return the number of bytes (from the current PC) to the branch label using
 * the given branch instruction */
int get_branch_offset(Instruction instruction) {
  
  // use shifts and bitmasking to obtain the bits
  uint32_t imm = 0x0;
  uint32_t imm12 = (instruction.sbtype.imm7 >> 6) & 0x1;
  uint32_t imm10_5 = (instruction.sbtype.imm7) & 0x3F;
  uint32_t imm11 = (instruction.sbtype.imm5) & 0x1;
  uint32_t imm4_1 = (instruction.sbtype.imm5 >> 1) & 0x1F;
  
  imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1); // because imm is 12:1, theres one extra bit at position 0. must shift so that theres an extra bit at position 0
  return sign_extend_number(imm, 13); // must sign extend to extend the offset bits
}

/* Returns the number of bytes (from the current PC) to the jump label using the
 * given jump instruction */
int get_jump_offset(Instruction instruction) {
  /**/
  // 00000000010000000000 10011 1101111
  int container = instruction.ujtype.imm; // assign immediate
  int imm = 0x0;


  int imm_20 = (container >> 19) & 0x1; //In decoded immediate order, grabbing 20th bit
  int imm_10_1 = (container >> 10) & 0x3FF; //grabbing [10:1] bit
  int imm_11 = (container >> 8) & 0x1; //Grabbing 11th bit
  int imm_19_12 = (container >> 0) & 0xFF;

  imm = (imm_20 << 20) |
        (imm_19_12 << 12) |
        (imm_11 << 11) |
        (imm_10_1 << 1);

    return (imm << 1);
  
  //return (instruction.ujtype.imm >> 8);
}

/* Returns the number of bytes (from the current PC) to the base address using the
 * given store instruction */
int get_store_offset(Instruction instruction) {

  uint32_t imm = 0x0; // set a bit string of 0s for storing imm5 and imm7
  uint32_t imm5 = instruction.stype.imm5; // imm[4:0]
  uint32_t imm7 = instruction.stype.imm7; // imm[11:5]

  imm = (imm7 << 5 | imm5); // set imm equal to imm7, shifted by 5 bits to make room for imm5, which is ORed with imm7
  return imm;
}
/************************Helper functions************************/

void handle_invalid_instruction(Instruction instruction) {
  printf("Invalid Instruction: 0x%08x\n", instruction.bits);
}

void handle_invalid_read(Address address) {
  printf("Bad Read. Address: 0x%08x\n", address);
  exit(-1);
}

void handle_invalid_write(Address address) {
  printf("Bad Write. Address: 0x%08x\n", address);
  exit(-1);
}
