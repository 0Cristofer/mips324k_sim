/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 02/04/2019 */

#ifndef MIPS324K_SIM_SIMULATOR_H
#define MIPS324K_SIM_SIMULATOR_H

#include <stdint-gcc.h>

#define NUM_REGISTERS 32
#define OP_DECODE_0 0
#define OP_DECODE_1 1
#define OP_DECODE_2 28

/* Op code groups
    SPECIAL: ADD, AND, DIV, JR, MFHI, MFLO, MOVN, MOVZ, MTHI, MTLO, MULT, NOP, NOR, OR, SUB, XOR
    REGIMM: BGEZ, BLTZ
    SPECIAL2: MADD, MSUB, MUL
    NONE: ADDI, ANDI, B, BEQ, BEQL, BGTZ, BLEZ, BNE, J, LUI, ORI, XORI
 */

enum op_types{
    SPECIAL, REGIMM, SPECIAL2, NONE
};

enum op_codes{
    ADD = 32, ADDI = 8, AND = 36, ANDI = 12,
    B = 4, BEQ = 4, BEQL = 20, BGEZ = 1, BGTZ = 7, BLEZ = 6, BLTZ = 0, BNE = 5,
    DIV = 26,
    J = 2, JR = 8,
    LUI = 15,
    MADD = 0, MFHI = 16, MFLO = 18, MOVN = 11, MOVZ = 10, MSUB = 4, MTHI = 17, MTLO = 19, MUL = 2, MULT = 24,
    NOP = 0, NOR = 39,
    OR = 37, ORI = 13,
    SUB = 34, SYSCALL = 12,
    XOR = 38, XORI = 14
};

typedef struct{
    unsigned int rd, rs, rt;
    unsigned int op_type, op_code, offset, code;
    uint16_t immediate;
}inst_barrier_t;

unsigned int running, num_instructions;
extern int debug;

unsigned int *instructions;
unsigned int registers[NUM_REGISTERS];
unsigned int pc;
inst_barrier_t inst_barrier;

void startSimulation(unsigned int *insts, unsigned int num_insts, int b);
void error();

void clock();
void pipeline();
void instruction();
void execution();
void memory();
void align();
void write();
void branchPrecdictor();
void unconditionalBranch(unsigned int op_c, unsigned int r, uint16_t imm, unsigned int off);

#endif //MIPS324K_SIM_SIMULATOR_H
