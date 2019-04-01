/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 29/03/2019 */

#ifndef MIPS324K_SIM_SIMULATOR_H
#define MIPS324K_SIM_SIMULATOR_H

unsigned int running;
unsigned int *instructions;
unsigned int num_instructions;
unsigned int pc;

void startSimulation(unsigned int *insts, unsigned int num_insts);
void clock();
void pipeline();
void instruction();
void execution();
void memory();
void align();
void write();
void branchPrecdictor();

#endif //MIPS324K_SIM_SIMULATOR_H
