/* Mips32 4K simulator ALU header file
   Authors: Cristofer Oswald
   Created: 16/05/2019
   Edited: 27/05/2019 */

#ifndef MIPS324K_SIM_ALU_H
#define MIPS324K_SIM_ALU_H

#define NUM_FU_MUL 2
#define NUM_FU_DIV 2
#define NUM_FU_SUB 1
#define NUM_FU_ADD 3

#define HI_REG 32
#define LO_REG 33

typedef struct functional_unit functional_unit_t;
typedef struct instruction_data instruction_data_t;

/**
 * Scoreboarding funciontal unit structure
 */
struct functional_unit{
    instruction_data_t *instruction;
    int busy;
    int op;
    int fi, fj, fk;
    functional_unit_t *qj, *qk;
    int rj, rk;
    int cicles_to_end;
};

/**
 * Initiates the ALU. Should be called in the simulator initialization.
 */
void initAlu();

/**
 * Verifies if a register has no functional unit writing to it.
 * @param r The register to be verified
 * @return 1 if the register is free, else 0
 */
int isRegFree(int r);

/**
 * Verifies if there a is MUL functional unit free
 * @return
 */
int hasFuMul();

/**
 * Verifies if there a is DIV functional unit free
 * @return
 */
int hasFuDiv();

/**
 * Verifies if there a is SUB functional unit free
 * @return
 */
int hasFuSub();

/**
 * Verifies if there a is ADD functional unit free
 * @return
 */
int hasFuAdd();

/**
 * Allocates a MUL functional unit, setting the necessary fields
 * @param instruction Instruction data necessary to the execution
 */
void allocFuMul(instruction_data_t *instruction);

/**
 * Allocates a DIV functional unit, setting the necessary fields
 * @param instruction Instruction data necessary to the execution
 */
void allocFuDiv(instruction_data_t *instruction);

/**
 * Allocates a SUB functional unit, setting the necessary fields
 * @param instruction Instruction data necessary to the execution
 */
void allocFuSub(instruction_data_t *instruction);

/**
 * Allocates a ADD functional unit, setting the necessary fields
 * @param instruction Instruction data necessary to the execution
 */
void allocFuAdd(instruction_data_t *instruction);

/**
 * Runs a cicle for each busy functional unit
 */
void runAlu();

#endif //MIPS324K_SIM_ALU_H
