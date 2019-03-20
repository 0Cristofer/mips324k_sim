/* Mips32 4K simulator main file
   Authors: Cristofer Oswald
   Created: 19/03/2019
   Edited: 20/03/2019 */

#ifndef MIPS324K_SIM_MIPS324K_SIM_H
#define MIPS324K_SIM_MIPS324K_SIM_H

// General program definitions
extern int ok;

/**
 * Clears the program, freeing all memoryand should be called before program end.
 * After this function being called, the program may exit
 */
void clearAll();

// Simulator specific definitions
extern int *prog_mem;


#endif //MIPS324K_SIM_MIPS324K_SIM_H
