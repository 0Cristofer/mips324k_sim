/* Mips32 4K simulator branch component header file
   Authors: Cristofer Oswald
   Created: 17/04/2019
   Edited: 17/04/2019 */

#ifndef MIPS324K_SIM_BRANCHCOMPONENT_H
#define MIPS324K_SIM_BRANCHCOMPONENT_H

#include <stdint.h>

/**
 * Structure used in the two bit branch predictor.
 */
typedef struct{
    unsigned int is_new : 1;
    unsigned int pred : 2;
}two_bit_t;

/**
 * Branch component is the first semi-processing of an instruction. Must happen right after a instruction is put in the
 * instruction queue. Checks whether this instruction is a branch/jump and if it is, do a branch prediction or
 * unconditional branch/jump, this way, the next instruction fetched is a result of this branch/jump.
 * In the case of unconditional branchs/jumps, the instruction is removed from the queue, since it was already processed
 * @param pc The program counter pointer at the moment
 * @return An integer, setting the offset to the next instruction. If no branch was taken, then it is just 1, else
 * is the offset to the branch/jump.
 */
int branchComponent(int pc);

/**
 * TODO
 */
int branchPredictor(uint16_t offset);


#endif //MIPS324K_SIM_BRANCHCOMPONENT_H
