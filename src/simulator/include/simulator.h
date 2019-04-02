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

/**
 * Types of op codes
 */
enum op_types{
    SPECIAL, REGIMM, SPECIAL2, NONE
};

/**
 * Op codes, should be used with type check
 */
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

/**
 * Barrier between instruction and execution stages
 */
typedef struct{
    unsigned int rd, rs, rt;
    unsigned int op_type, op_code, offset, code;
    uint16_t immediate;
}inst_barrier_t;

// General simulator variables
unsigned int running, num_instructions;
extern int debug;

// Program specific
unsigned int *instructions;
unsigned int registers[NUM_REGISTERS];
unsigned int pc;
inst_barrier_t inst_barrier;

// General simulator functions

/**
 * Starts the MIPS32 4K simulation
 * @param insts Array of instructions of the program to be simulated
 * @param num_insts Number of instructions
 * @param b Debug mode, 0 to OFF, 1 to ON
 */
void startSimulation(unsigned int *insts, unsigned int num_insts, int b);

/**
 * Sets the error flags
 */
void error();

// Hardware implementations

/**
 * Keeps the system running, giving "clock" pulses each iteration
 */
void clock();

/**
 * Calls all pipeline stages.
 * Note: The stages are called in reverse order, this way each instruction goes only one step per clock
 */
void pipeline();

/**
 * First pipeline stage. Decodes the instruction, calling the branch predictor and setting up the registers needed
 */
void instruction();

/**
 * Second pipeline stage. Starts de execution of a instruction.
 */
void execution();

/**
 * Third pipeline stage. Finaliza a execução das instruções
 */
void memory();

/**
 * Fourth pipeline stage. Aligns the data
 */
void align();

/**
 * Fifth pipeline stage. Writes the data to the source
 */
void write();

/**
 * Branch prediction and program counter component. Calls change to pc if there is a unconditional branch, or just increments
 * it. After, looks up the branch prediction table (if ther branch is conditional).
 * @param op_type Type of the op code
 * @param op_code The op code
 * @param rs Source register
 * @param rt Target register
 * @param immediate Immediate data
 * @param offset Offset data
 */
void branchPrecdictor(unsigned int op_type, unsigned int op_code,
                      unsigned int rs, unsigned int rt, uint16_t immediate, unsigned int offset);

/**
 * Makes a unconditional branch, updating pc.
 * @param op_c The op code
 * @param r Registe
 * @param imm Immediate
 * @param off Offset
 */
void unconditionalBranch(unsigned int op_c, unsigned int r, uint16_t imm, unsigned int off);

#endif //MIPS324K_SIM_SIMULATOR_H
