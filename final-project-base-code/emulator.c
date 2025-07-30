#include <stdio.h> // for stderr
#include <stdlib.h> // for exit()
#include "types.h"
#include "utils.h"
#include "riscv.h"

void execute_rtype(Instruction, Processor *);
void execute_itype_except_load(Instruction, Processor *);
void execute_branch(Instruction, Processor *);
void execute_jal(Instruction, Processor *);
void execute_load(Instruction, Processor *, Byte *);
void execute_store(Instruction, Processor *, Byte *);
void execute_ecall(Processor *, Byte *);
void execute_lui(Instruction, Processor *);

void execute_instruction(uint32_t instruction_bits, Processor *processor,Byte *memory) {    
    Instruction instruction = parse_instruction(instruction_bits);
    switch(instruction.opcode) {
        case 0x33:
            execute_rtype(instruction, processor);
            break;
        case 0x13:
            execute_itype_except_load(instruction, processor);
            break;
        case 0x73:
            execute_ecall(processor, memory);
            break;
        case 0x63:
            execute_branch(instruction, processor);
            break;
        case 0x6F:
            execute_jal(instruction, processor);
            break;
        case 0x23:
            execute_store(instruction, processor, memory);
            break;
        case 0x03:
            execute_load(instruction, processor, memory);
            break;
        case 0x37:
            execute_lui(instruction, processor);
            break;
        default: // undefined opcode
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
}

void execute_rtype(Instruction instruction, Processor *processor) {
    switch (instruction.rtype.funct3){
        case 0x0:
            switch (instruction.rtype.funct7) {
                case 0x0:
                    // Add
                    processor->R[instruction.rtype.rd] = // processor->[] accesses a register
                        ((sWord)processor->R[instruction.rtype.rs1]) + // sWord is a cast that converts the register value to 32 signed bit integers
                        ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                case 0x01:
                    // Mul
                    processor->R[instruction.rtype.rd] =
                        ((sWord)processor->R[instruction.rtype.rs1]) *
                        ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                case 0x20:
                    // Sub
                    processor->R[instruction.rtype.rd] =
                        ((sWord)processor->R[instruction.rtype.rs1]) -
                        ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;

        case 0x1:
            switch (instruction.rtype.funct7) {
                case 0x00:
                    // sll - shift bits of rs1 by an amount rs2 and then storing it in rd
                    processor->R[instruction.rtype.rd] =
                    ((sWord)processor->R[instruction.rtype.rs1]) << 
                    ((sWord)processor->R[instruction.rtype.rs2]); // accessing registers for r-type rs1 and rs2
                    break;
                case 0x1:
                    // mulh - multiply rs1 by rs2, and then shift by 32 bits to the right to store the highest 32 bits of the value. All of these values are signed Word or Double
                    processor->R[instruction.rtype.rd] =
                    (sWord)(((sDouble)processor->R[instruction.rtype.rs1]) *
                    ((sDouble)processor->R[instruction.rtype.rs2]) >> 32);
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;
        case 0x2:
            switch (instruction.rtype.funct7) {
                case 0x01:
                    //mulsu - multiply rs1 by rs2, and then shift by 32 bits to the right to store the highest 32 bits of the value. rs1 is a signed word or signed double, and rs2 is an unsigned double
                    processor->R[instruction.rtype.rd] =
                    (sWord)(((sDouble)processor->R[instruction.rtype.rs1]) *
                    ((Double)processor->R[instruction.rtype.rs2]) >> 32);
                    break;
                case 0x00:
                    //slt - set rd to be 1 if the value of rs1 is less than rs2, otherwise 0
                    processor->R[instruction.rtype.rd]=
                    ((sWord)processor->R[instruction.rtype.rs1] < 
                    (sWord)processor->R[instruction.rtype.rs2]) ? 1 : 0;
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            break;
            }
            break;
        case 0x3:
            switch (instruction.rtype.funct7) {
                case 0x00:
                    // sltu - same as slt, but the values are unsigned
                    processor->R[instruction.rtype.rd]=
                    ((Word)processor->R[instruction.rtype.rs1] < 
                    (Word)processor->R[instruction.rtype.rs2]) ? 1 : 0;
                    break;

                case 0x01:
                    // mulu - multiply rs1 by rs2, and then shift by 32 bits to the right to store the highest 32 bits of the value. rs1 is a unsigned double or signed word, and rs2 is an unsigned double
                    processor->R[instruction.rtype.rd] =
                    (sWord)(((Double)processor->R[instruction.rtype.rs1]) *
                    ((Double)processor->R[instruction.rtype.rs2]) >> 32);
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            break;
            }
            break;
        case 0x4:
            switch (instruction.rtype.funct7) {
                case 0x0:
                // Xor - finding the exclusive OR for rs1 and rs2. Both of these values are signed
                processor->R[instruction.rtype.rd] =
                    ((sWord)processor->R[instruction.rtype.rs1]) ^
                    ((sWord)processor->R[instruction.rtype.rs2]);
                break;
                case 0x01:
                // Div - dividing rs1 (signed) by rs2 (signed)
                processor->R[instruction.rtype.rd] = 
                    ((sWord)processor->R[instruction.rtype.rs1]) /
                    ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            break;
            }
            break;
        case 0x5:
            switch (instruction.rtype.funct7) {
                case 0x00:
                    //srl - shift rs1 bits right by an amount of rs2. rs1 is signed but rs2 is unsigned because the msb does not extend. Only zeros can extend
                    processor->R[instruction.rtype.rd] =
                    (((sWord)processor->R[instruction.rtype.rs1]) >>
                    ((Word)processor->R[instruction.rtype.rs2]));
                    break;
                case 0x01:
                    // divu - same as div, only now rs1 and rs2 are unsigned numbers
                    processor->R[instruction.rtype.rd] =
                    (Word)processor->R[instruction.rtype.rs1] /
                    (Word)processor->R[instruction.rtype.rs2];

                    break;
                case 0x20:
                    // sra - shift the bits right by an amount of rs2, but rs1 and rs2 are signed, so the msb does extend
                    processor->R[instruction.rtype.rd] =
                    ((sWord)processor->R[instruction.rtype.rs1]) >>
                    ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            break;
            }
            break;
        case 0x6:
            switch (instruction.rtype.funct7) {
                case 0x00:
                    // or - finds the OR operation with rs1 (unsigned) and rs2 (unsigned)
                    processor->R[instruction.rtype.rd] =
                    (Word)processor->R[instruction.rtype.rs1] |
                    (Word)processor->R[instruction.rtype.rs2];
                    break;
                case 0x01:
                    // rem - returns the remainder (or modulus) of rs1 divided by rs2. Both of these values are signed
                    processor->R[instruction.rtype.rd] =
                    (sWord)processor->R[instruction.rtype.rs1] %
                    (sWord)processor->R[instruction.rtype.rs2];
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            break;
            }
            break;
        case 0x7:
            switch (instruction.rtype.funct7) {
                case 0x00:
                    // and - finds the AND operation ofr rs1 (unsigned) and rs2 (unsigned)
                    processor->R[instruction.rtype.rd] =
                    (Word)processor->R[instruction.rtype.rs1] &
                    (Word)processor->R[instruction.rtype.rs2];
                    break;
                case 0x01:
                    // remu - returns the remainder (or modulus) of rs1 divided by rs2. Both of these values are unsigned
                    processor->R[instruction.rtype.rd] =
                    (Word)processor->R[instruction.rtype.rs1] %
                    (Word)processor->R[instruction.rtype.rs2];
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            break;
            }
            break;
        }
       
                
    // update PC
    processor->PC += 4;
}

void execute_itype_except_load(Instruction instruction, Processor *processor) {

    switch (instruction.itype.funct3) {
        case 0x0:
            // addi - ADD IMMEDIATE - adds values from rs1 and a signed immediate (constant) value. The immediate is sign extended so that it is 32 bits long and preserves the sign
            processor->R[instruction.itype.rd] =
            ((sWord)processor->R[instruction.itype.rs1] +
            (sWord)sign_extend_number(instruction.itype.imm, 12));
            break;
        case 0x1:

           switch ((instruction.itype.imm >> 5) & (0x7F)) {
                case 0x0:
                    // slli - SHIFT LOGICAL LEFT IMMEDIATE - shifts rs1 (unsigned) left by an amount unsigned immediate (constant) value
                    processor->R[instruction.itype.rd] =
                    ((Word)processor->R[instruction.itype.rs1] << 
                    ((Word)instruction.itype.imm & 0x1F));
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;

        case 0x2:
            // slti - SET LESS THAN IMMEDIATE - sets rd to 1 if rs1 (signed) is less than signed immediate (constant) value, otherwise 0. The immediate is sign extended so that it is 32 bits long and preserves the sign
            processor->R[instruction.itype.rd] =
            (((sWord)processor->R[instruction.itype.rs1] <
            (sWord)sign_extend_number(instruction.itype.imm, 12)? 1 : 0));
            break;

        case 0x3:
            // sltiu - SET LESS THAN IMMEDIATE UNSIGNED -  sets rd to 1 if rs1 (unsigned) is less than unsigned immediate (constant) value
            processor->R[instruction.itype.rd] =
            (((Word)processor->R[instruction.itype.rs1] <
            (Word)instruction.itype.imm)? 1 : 0);
            break;

        case 0x4:
            // xori - EXCLUSIVE OR IMMEDIATE - takes the exclusive or of unsigned rs1 and an immediate value
            processor->R[instruction.itype.rd] =
            ((Word)processor->R[instruction.itype.rs1] ^
            (Word)sign_extend_number(instruction.itype.imm, 12));
            break;

        case 0x5:


            switch ((instruction.itype.imm >> 5) & (0x7F)) {
                case 0x0:
                    // srli - SHIFT RIGHT LOGICAL IMMEDIATE - shifts rs1 (unsigned) right logically by an amount unsigned immediate (constant) value (where imm is the first 5 bits of imm)
                    processor->R[instruction.itype.rd] =
                    ((Word)processor->R[instruction.itype.rs1] >>
                    ((Word)instruction.itype.imm & 0x1F));
                    break;
                case 0x20:
                    // srai - SHIFT RIGHT ARITHMETRIC IMMEDIATE - shifts rs1 (signed) right logically by an amount signed immediate (constant) value
                    processor->R[instruction.itype.rd] =
                    ((sWord)processor->R[instruction.itype.rs1] >>
                    instruction.itype.imm);
                    break;

                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;


        case 0x6:
            // ori - OR IMMEDIATE
            processor->R[instruction.itype.rd] =
            ((Word)processor->R[instruction.itype.rs1] |
            (Word)sign_extend_number(instruction.itype.imm, 12));
            break;
        case 0x7:
            // andi - AND IMMEDIATE
            processor->R[instruction.itype.rd] =
            ((Word)processor->R[instruction.itype.rs1] &
            (Word)sign_extend_number(instruction.itype.imm, 12));
            break;
        default:
            handle_invalid_instruction(instruction);
            break;
    }
    // Update PC
    processor->PC += 4;
}

void execute_ecall(Processor *p, Byte *memory) {
    Register i;
    // syscall number is given by a0 (x10)
    // argument is given by a1
    switch(p->R[10]) {
        case 1: // print an integer
            printf("%d",p->R[11]);
            p->PC += 4;
            break;
        case 4: // print a string
            for(i=p->R[11];i<MEMORY_SPACE && load(memory,i,LENGTH_BYTE);i++) {
                printf("%c",load(memory,i,LENGTH_BYTE));
            }
            p->PC += 4;
            break;
        case 10: // exit
            printf("exiting the simulator\n");
            exit(0);
            break;
        case 11: // print a character
            printf("%c",p->R[11]);
            p->PC += 4;
            break;
        default: // undefined ecall
            printf("Illegal ecall number %d\n", p->R[10]);
            exit(-1);
            break;
    }
}

void execute_branch(Instruction instruction, Processor *processor) {
    switch (instruction.sbtype.funct3) {
        case 0x0:
            // beq - BRANCH IF EQUALS - if rs1 (signed) is equal to (signed) rs2, add the value of immediate to the PC, otherwise add 4 to PC
            if ((sWord)processor->R[instruction.sbtype.rs1] == (sWord)processor->R[instruction.sbtype.rs2]) {
                processor->PC += get_branch_offset(instruction);
            } else {
                processor->PC +=4;
            }

            break;

        case 0x1:
            // bne - BRANCH IF NOT EQUALS - if rs1 (signed) is not equal to (signed) rs2, add the value of immediate to the PC, otherwise add 4 to PC
            if ((sWord)processor->R[instruction.sbtype.rs1] != (sWord)processor->R[instruction.sbtype.rs2]) {
                processor->PC += get_branch_offset(instruction);
            } else {
                processor->PC +=4;
            }
            break;

        case 0x4:
            // blt - BRANCH IF LESS THAN - if rs1 (signed) is less than (signed) rs2, add the value of immediate to the PC, otherwise add 4 to PC
            ((sWord)processor->R[instruction.sbtype.rs1] < (sWord)processor->R[instruction.sbtype.rs2])? 
            (processor->PC += get_branch_offset(instruction)) : (processor->PC +=4);
            break;

        case 0x5:
            // bge - BRANCH IF GREATER OR EQUAL - if rs1 (signed) is greater or equal to (signed) rs2, add the value of immediate to the PC, otherwise add 4 to PC
            ((sWord)processor->R[instruction.sbtype.rs1] >= (sWord)processor->R[instruction.sbtype.rs2])? 
            (processor->PC += get_branch_offset(instruction)) : (processor->PC +=4);
            break;

        case 0x6:
            // bltu - BRANCH IF LESS THAN UNSIGNED - if rs1 (unsigned) is less than (unsigned) rs2, add the value of immediate to the PC, otherwise add 4 to PC
            ((Word)processor->R[instruction.sbtype.rs1] < (Word)processor->R[instruction.sbtype.rs2])? 
            (processor->PC += get_branch_offset(instruction)) : (processor->PC +=4);
            break;

        case 0x7:
            // bgeu - BRANCH IF GREATER OR EQUAL UNSIGNED - if rs1 (unsigned) is greater or equal to (unsigned) rs2, add the value of immediate to the PC, otherwise add 4 to PC
            ((Word)processor->R[instruction.sbtype.rs1] >= (Word)processor->R[instruction.sbtype.rs2])? 
            (processor->PC += get_branch_offset(instruction)) : (processor->PC +=4);
            break;

            
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
}

void execute_load(Instruction instruction, Processor *processor, Byte *memory) {
    /* YOUR CODE HERE */
    switch (instruction.itype.funct3) {
        case 0x0:
            // lb - LOAD BYTE - must sign extend after loading the value because it needs to be 32 bits long, and LENGTH_BYTE is just 8 bits long 
            processor->R[instruction.itype.rd] = 
            sign_extend_number(load(memory, processor->R[instruction.itype.rs1] + 
            sign_extend_number(instruction.itype.imm, 12), LENGTH_BYTE), 8);
            break;
        case 0x1:
            // lh - LOAD HALF BYTE - must also sign extend after loading since HALF_WORD is only 16 bits long
            processor->R[instruction.itype.rd] = 
            sign_extend_number(load(memory, processor->R[instruction.itype.rs1] + 
            sign_extend_number(instruction.itype.imm, 12),  LENGTH_HALF_WORD), 16);
            break;
        case 0x2:
            // lw - signed but no need to sign extend because a word length is 32 bits long
            processor->R[instruction.itype.rd] = 
            load(memory, processor->R[instruction.itype.rs1] + 
            sign_extend_number(instruction.itype.imm, 12),  LENGTH_WORD);
            break; 
        case 0x4:
            // lbu - LOAD BYTE UNSIGNED - unsigned, so no sign extension needed when setting rd
            processor->R[instruction.itype.rd] = 
            load(memory, processor->R[instruction.itype.rs1] + 
            sign_extend_number(instruction.itype.imm, 12), LENGTH_BYTE);
            break;
        case 0x5:
            // lhu - LOAD HALF WORD UNSIGNED - unsigned, so no sign extension needed when setting rd
            processor->R[instruction.itype.rd] = 
            load(memory, processor->R[instruction.itype.rs1] + 
            sign_extend_number(instruction.itype.imm, 12), LENGTH_HALF_WORD);
            break;
        default:
            handle_invalid_instruction(instruction);
            break;
    }

    // update PC
    processor->PC += 4;
}

void execute_store(Instruction instruction, Processor *processor, Byte *memory) {
    switch (instruction.stype.funct3) {
        case 0x0:
            // sb - STORE BYTE - stores a byte sized value rs2 into memory at address = rs1 + imm
            store(memory, processor->R[instruction.stype.rs1] + 
            get_store_offset(instruction), LENGTH_BYTE, processor->R[instruction.stype.rs2]); // dont forget to store the value of rs2 not the actual address of rs2
            break;
        case 0x1:
            // sh - STORE HALF WORD - stores a half word size (2 bytes) into memory at address = rs1 + imm
            store(memory, processor->R[instruction.stype.rs1] + 
            get_store_offset(instruction), LENGTH_HALF_WORD, processor->R[instruction.stype.rs2]);
            break;
        case 0x2:
            // sw - STORE WORD - stores a full word size (4 bytes) into memory at address = rs1 + imm
            store(memory, processor->R[instruction.stype.rs1] + 
            get_store_offset(instruction), LENGTH_WORD, processor->R[instruction.stype.rs2]);
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }

    // update PC
    processor->PC += 4;
}

void execute_jal(Instruction instruction, Processor *processor) {
    // jal - JUMP AND LINK
    processor->R[instruction.ujtype.rd] = (Word)(processor->PC + 4); // rd = PC + 4
    processor->PC += (sWord)(sign_extend_number(get_jump_offset(instruction), 20)); // PC = PC + imm
}

void execute_lui(Instruction instruction, Processor *processor) {
    // lui - LOAD UPPER IMM - loads imm into rd by converting imm into a signed byte and by shifting imm left by 12 bits  
    processor->R[instruction.utype.rd] = (sWord)(instruction.utype.imm) << 12;
    // update PC
    processor->PC += 4;
}

void store(Byte *memory, Address address, Alignment alignment, Word value) {
    /* YOUR CODE HERE */ 
    // reference the load function here. muust include 3 cases: LENGTH BYTE, LENGTH HALF WORD, LENGTH WORD
    // address is a 32 bit index in memory, memory (RAM) is a stack/array that contains stored words
    switch(alignment) {
        case LENGTH_BYTE:
            memory[address] = value & 0xFF; // store 1 byte of the value of the word by masking the first 8 bits of value
        break;
        case LENGTH_HALF_WORD:
            memory[address] = value & 0xFF; // storing 1 byte 
            memory[address + 1] = (value >> 8) & 0xFF; // storing first byte
        break;
        case LENGTH_WORD:
            memory[address] = value & 0xFF; // storing 1 byte starting from msb
            memory[address + 1] = (value >> 8) & 0xFF; // another byte after
            memory[address + 2] = (value >> 16) & 0xFF; // another byte after
            memory[address + 3] = (value >> 24) & 0xFF; // first byte in string
        break;
    }
}

Word load(Byte *memory, Address address, Alignment alignment) {
    // chooses which n bytes to return 
    if(alignment == LENGTH_BYTE) {
        return memory[address]; // return value at memory at address at first location
    } else if(alignment == LENGTH_HALF_WORD) {
        return (memory[address+1] << 8) + memory[address]; // return value at memory at address at 1 and 2 locations
    } else if(alignment == LENGTH_WORD) {
        return (memory[address+3] << 24) + (memory[address+2] << 16) 
               + (memory[address+1] << 8) + memory[address]; // return value at memory at addresses 1 2 and 3 locations
    } else {
        printf("Error: Unrecognized alignment %d\n", alignment);
        exit(-1);
    }
} 