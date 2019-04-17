/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 02/04/2019 */

#include "include/simulator.h"
#include "include/branchComponent.h"
#include "include/util.h"

#include <stdlib.h>
#include <stdint.h>

int debug;
int has_error = 0;

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
    int next_pc;
    queue_data_t data;

    if(instruction_queue.size) return; // If there is elements in the instruction queue, do nothing

    while((instruction_queue.size < INST_QUEUE_SIZE) && running){
        data.instruction = instructions[pc];

        pushQueue(&instruction_queue, data);

        next_pc = branchComponent();

        if(next_pc) updatePc(next_pc); // If a branch was taken, go to given instruction
        else updatePc(1); // Else, go to next instruction
    }
}

/*void instruction(){
    unsigned int instruction, op_type, op_code, rd = 0, rt = 0, rs = 0, offset = 0, code = 0;
    uint16_t immediate = 0;

    if(has_error) return;

    instruction = instructions[pc];

    printDebugMessageInt("Decoding instruction", pc);

    // Decodification
    switch (instruction >> 26){
        case OP_DECODE_0:
            op_type = SPECIAL;

            op_code =  instruction & (unsigned int)63; // Get only first 6 bits

            if(op_code == SYSCALL){
                code = instruction >> 6;
            }
            else {

                rd = (instruction >> 11) & (unsigned int) 31; // Get only first 5 bits
                rt = (instruction >> 16) & (unsigned int) 31;
                rs = (instruction >> 21) & (unsigned int) 31;
            }

            break;

        case OP_DECODE_1:
            op_type = REGIMM;

            offset =  instruction & (unsigned int)65535; // Get only first 16 bits
            op_code = ((instruction >> 16) & (unsigned int)31);
            rs = (instruction >> 21) & (unsigned int)31;

            break;

        case OP_DECODE_2:
            op_type = SPECIAL2;

            op_code =  instruction & (unsigned int)63; // Get only first 6 bits
            rd = (instruction >> 11) & (unsigned int)31; // Get only first 5 bits
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

    inst_barrier.op_type = op_type;
    inst_barrier.op_code = op_code;
    inst_barrier.rd = rd;
    inst_barrier.rt = rt;
    inst_barrier.rs = rs;
    inst_barrier.immediate = immediate;
    inst_barrier.offset = offset;
    inst_barrier.code = code;

    updatePc(op_type, op_code, rs, rt, immediate, offset);

    if(pc == num_instructions) running = 0;
}*/

void execution(){
    if(has_error) return;
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

    if(pc == num_instructions) running = 0;
}

void error(){
    has_error = 1;
    running = 0;
    printDebugMessage("An error happend, stopping.");
}