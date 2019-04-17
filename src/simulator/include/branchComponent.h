/* Mips32 4K simulator branch component header file
   Authors: Cristofer Oswald
   Created: 17/04/2019
   Edited: 17/04/2019 */

#ifndef MIPS324K_SIM_BRANCHCOMPONENT_H
#define MIPS324K_SIM_BRANCHCOMPONENT_H

int branchComponent();

/**
 * Effectively verifies if the current instruction is a branch and if it is a unconditional branch, just update, else
 * calls the branch predictor
 * @param op_type Type of the op code
 * @param op_code The op code
 * @param rs Source register
 * @param rt Target register
 * @param immediate Immediate data
 * @param offset Offset data
 * @return If the current instruction is a branch
 */
unsigned int branchTest(unsigned int op_type, unsigned int op_code,
                        unsigned int rs, unsigned int rt, uint16_t immediate, unsigned int offset);

/**
 * TODO
 */
void branchPredictor();

/**
 * Makes a unconditional branch, updating pc.
 * @param op_c The op code
 * @param r Registe
 * @param imm Immediate
 * @param off Offset
 */
void unconditionalBranch(unsigned int op_c, unsigned int r, uint16_t imm, unsigned int off);

#endif //MIPS324K_SIM_BRANCHCOMPONENT_H
