/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 17/04/2019 */

#include "include/simulator.h"
#include "include/branchComponent.h"
#include "include/util.h"

#include <stdlib.h>
#include <stdint.h>

int debug;
int has_error = 0;

unsigned int pc;
queue_t instruction_queue;

void startSimulation(unsigned int *insts, unsigned int num_insts, int b){
    int i;

    debug = b;
    instructions = insts;
    running = 1;
    num_instructions = num_insts;
    pc = 0;

    initQueue(&instruction_queue);

    for(i = 0; i < NUM_REGISTERS; i++){
        registers[i] = 0;
    }

    clock();

    cleanup();
}

void clock(){
    int cycle = 0;

    while(running){
        printDebugMessageInt("************** Cycle *******", cycle);

        pipeline();

        cycle++;
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
    int next_pc;
    queue_data_t data;

    if(has_error) return;

    if(instruction_queue.size){ // If there is elements in the instruction queue, do nothing
        printDebugMessage("---Instruction stage. Queue has elements, skipping fetch...---");
        return;
    }

    printDebugMessage("---Instruction stage. Fetching instructions---");

    while((instruction_queue.size < INST_QUEUE_SIZE) && running){
        if(pc == num_instructions){
            printDebugMessage("Out of instructions. Stopping fetch");
            return;
        }

        data.instruction = instructions[pc];

        printDebugMessageInt("\tFetched instruction", pc);

        pushQueue(&instruction_queue, data);

        next_pc = branchComponent(pc);

        updatePc(next_pc); // The PC increment will be given from the branch component
    }
}

void execution(){
    unsigned int instruction, op_type, op_code, rd = 0, rt = 0, rs = 0, offset = 0, code = 0;
    uint16_t immediate = 0;

    if(has_error) return;

    if((!instruction_queue.size) && (pc == num_instructions)) running = 0; // Temporary

    if(!instruction_queue.size){
        printDebugMessage("---Execution stage. Queue is empety, skipping...---");
        return;
    }

    printDebugMessage("---Execution stage. Decoding instruction---");

    instruction = popQueue(&instruction_queue).instruction;

    // Decodification
    switch (instruction >> 26){
        case OP_DECODE_0:
            op_type = SPECIAL;

            op_code =  instruction & (unsigned int)63;

            if(op_code == SYSCALL){
                code = instruction >> 6;
            }
            else {

                rd = (instruction >> 11) & (unsigned int) 31;
                rt = (instruction >> 16) & (unsigned int) 31;
                rs = (instruction >> 21) & (unsigned int) 31;
            }

            break;

        case OP_DECODE_1:
            op_type = REGIMM;

            offset =  instruction & (unsigned int)65535;
            op_code = ((instruction >> 16) & (unsigned int)31);
            rs = (instruction >> 21) & (unsigned int)31;

            break;

        case OP_DECODE_2:
            op_type = SPECIAL2;

            op_code =  instruction & (unsigned int)63;
            rd = (instruction >> 11) & (unsigned int)31;
            rt = (instruction >> 16) & (unsigned int)31;
            rs = (instruction >> 21) & (unsigned int)31;

            break;

        default:
            op_type = NONE;

            op_code = instruction >> 26;

            if(op_code == J){
                offset = instruction & (unsigned int)67108863;
            }

            immediate = (uint16_t)(instruction & (unsigned int)65535);
            rt = (instruction >> 16) & (unsigned int)31;
            rs = (instruction >> 21) & (unsigned int)31;

            break;
    }
}

void memory(){
    if(has_error) return;
}

void align(){
    if(has_error) return;
}

void write(){
    if(has_error) return;
}

void updatePc(int next_pc){
    pc = pc + next_pc;
}

void cleanup(){
    clearQueue(&instruction_queue);
}

void error(){
    has_error = 1;
    running = 0;
    printDebugMessage("An error happend, stopping.");
}