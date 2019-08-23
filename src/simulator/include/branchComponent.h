/* Mips32 4K simulator branch component header file
   Authors: Cristofer Oswald
   Created: 17/04/2019
   Edited: 04/07/2019 */

#ifndef MIPS324K_SIM_BRANCHCOMPONENT_H
#define MIPS324K_SIM_BRANCHCOMPONENT_H

#include <stdint.h>
#include "../../helpers/include/linked_list.h"

#define BRENCH_PRED_SIZE 1024

/**
 * Structure used in the two bit branch predictor.
 */
typedef struct {
    unsigned int is_new : 1;
    unsigned int pred : 2;
} two_bit_t;

/**
 * Verifies if this is instruction is being speculated. If there it comes from an unresolved branch, adds it the
 * its speculated instrucions, else does nothing.
 * @param inst_data The instruction data
 */
void addSpeculative(instruction_data_t *inst_data);

/**
 * Branch component is the first semi-processing of an instruction. Must happen right after a instruction is put in the
 * instruction queue. Checks whether this instruction is a branch/jump and if it is, do a branch prediction or
 * unconditional branch/jump, this way, the next instruction fetched is a result of this branch/jump.
 * In the case of unconditional branchs/jumps, the instruction is removed from the queue, since it was already processed
 * @param current_pc The program counter pointer at the moment
 * @return An integer, setting the offset to the next instruction. If no branch was taken, then it is just 1, else
 * is the offset to the branch/jump.
 */
int branchComponent(int current_pc);

/**
 * Initializates the branch predictor. Should be called at simulator initialization
 */
void initBranchPredictor();

/**
 * Updates the BHT given the branch and it's result
 * @param index The index of the BHT to be updated
 * @param b The branch result
 */
void updateBht(uint16_t index, unsigned int b);

extern instruction_data_t *current_branch_inst;

#endif //MIPS324K_SIM_BRANCHCOMPONENT_H
