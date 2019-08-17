/* Mips32 4K simulator util functions
   Authors: Cristofer Oswald
   Created: 02/04/2019
   Edited: 02/04/2019 */

#ifndef MIPS324K_SIM_UTIL_H
#define MIPS324K_SIM_UTIL_H

void printDebugMessage(char *message);
void printDebugMessageInt(char *message, int d);
void printDebugError(char *stage, char *message);
void printRegister(enum register_name reg);
void printCycles(int cycles);
void printInstruction(char *int_str);
void printCurrentCycle(int cycle);

#endif //MIPS324K_SIM_UTIL_H
