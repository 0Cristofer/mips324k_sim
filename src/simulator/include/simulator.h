/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 27/05/2019 */

#ifndef MIPS324K_SIM_SIMULATOR_H
#define MIPS324K_SIM_SIMULATOR_H

#include <stdint-gcc.h>

#include "../../helpers/include/queue.h"
#include "../../helpers/include/linked_list.h"

#define OP_DECODE_0 0
#define OP_DECODE_1 1
#define OP_DECODE_2 28

#define NUM_REGISTERS 34
#define MAX_INST_QUEUE_SIZE 4
#define MAX_DECODE_QUEUE_SIZE 2

/* Op code groups
    SPECIAL: ADD, AND, DIV, MFHI, MFLO, MOVN, MOVZ, MTHI, MTLO, MULT, NOP, NOR, OR, SUB, SYSCALL, XOR
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
 * Op codes
 */
enum op_codes{
    ADD = 32, ADDI = 8, AND = 36, ANDI = 12,
    B = 4, BEQ = 4, BEQL = 20, BGEZ = 1, BGTZ = 7, BLEZ = 6, BLTZ = 0, BNE = 5,
    DIV = 26,
    J = 2,
    LUI = 15,
    MADD = 0, MFHI = 16, MFLO = 18, MOVN = 11, MOVZ = 10, MSUB = 4, MTHI = 17, MTLO = 19, MUL = 2, MULT = 24,
    NOP = 0, NOR = 39,
    OR = 37, ORI = 13,
    SUB = 34, SYSCALL = 12,
    XOR = 38, XORI = 14
};

typedef struct functional_unit functional_unit_t;

/**
 * Main instrucion structure. Holds all information about a instruction and all fields are filled at the end of the
 * decode stage
 */
struct instruction_data{
    unsigned int instruction;
    unsigned int op_type, op_code;
    unsigned int rd, rs, rt;
    uint16_t imm;
    unsigned int pc;
    int is_speculate;
    functional_unit_t *f;
    linked_list_t *speculative_insts;
};

// General simulator variables
unsigned int running, num_instructions;
extern int debug;

// Program specific
unsigned int *instructions;
unsigned int registers[NUM_REGISTERS];
extern unsigned int pc;
extern queue_t instruction_queue;

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

/**
 * Frees utilized memory
 */
void cleanup();

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
 * First pipeline stage. If the instruction queue is emepty, fetches instructions until the instruction queue is full.
 * Each instruction fetched goes to the branch component, so the program counter can be updated if there is a branch/jump.
 */
void instruction();

/**
 * Second pipeline stage. Removes instructions from the queue, if possible. Decodes the instructions
 * TODO
 */
void execution();

/**
 * Third pipeline stage.
 * TODO
 */
void memory();

/**
 * Fourth pipeline stage.
 * TODO
 */
void alignAccumulate();

/**
 * Fifth pipeline stage.
 * TODO
 */
void writeback();

/**
 * Updates the program counter based on an offset
 * @param next_pc The offset to be added to the program counter
 */
void updatePc(int next_pc);

#endif //MIPS324K_SIM_SIMULATOR_H
