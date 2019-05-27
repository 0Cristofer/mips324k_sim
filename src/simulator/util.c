/* Mips32 4K simulator util functions
   Authors: Cristofer Oswald
   Created: 02/04/2019
   Edited: 02/04/2019 */

#include <stdio.h>

#include "include/simulator.h"
#include "include/util.h"

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