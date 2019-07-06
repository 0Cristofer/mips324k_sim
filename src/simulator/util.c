/* Mips32 4K simulator util functions
   Authors: Cristofer Oswald
   Created: 02/04/2019
   Edited: 02/04/2019 */

#include <stdio.h>

#include "include/simulator.h"
#include "include/util.h"

char*register_names[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                         "s0", "s1", "s2", "s3", "s4", "s5", "s5", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra", "hi", "lo"};

void printDebugMessage(char *message){
    if(!debug) return;
    printf("DEBUG: %s\n", message);
}

void printDebugMessageInt(char *message, int d){
    if(!debug) return;
    printf("DEBUG: %s, %d\n", message, d);
}

void printDebugError(char *stage, char *message){
    if(!debug) return;
    fprintf(stderr, "ERROR: stage - %s, message: %s\n", stage, message);
}

void printRegister(enum register_name reg){
    if(!debug) return;
    printf("DEBUG: Printing register %s, %d\n", register_names[reg], registers[reg]);
}