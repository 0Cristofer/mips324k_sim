/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 29/03/2019 */

#include "src/simulator/include/simulator.h"

void startSimulation(unsigned int *insts, unsigned int num_insts){
    instructions = insts;
    running = 1;
    num_instructions = num_insts;
    pc = 0;

    clock();
}

void clock(){
    while(running){
        pipeline();
    }
}

void pipeline(){
    write();
    align();
    memory();
    execution();
    instruction();
}

void instruction(){
    branchPrecdictor();
    pc = pc + 1;

    if(pc == num_instructions) running = 0;
}

void execution(){

}

void memory(){

}

void align(){

}

void write(){

}

void branchPrecdictor(){

}