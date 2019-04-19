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
    initQueue(&decode_queue);

    has_functional_unit = 1; // Temporary

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
        printDebugMessage("---Instruction stage. Instruction queue has elements, skipping fetch---");
        return;
    }

    if(pc == num_instructions){
        printDebugMessage("---Instruction stage. PC at the end of program, skipping fetch---");
        return;
    }

    printDebugMessage("---Instruction stage. Fetching instructions---");

    while(instruction_queue.size < MAX_INST_QUEUE_SIZE){
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
    static int is_decode = 1;
    queue_data_t data;

    unsigned int instruction, op_type, op_code, rd = 0, rt = 0, rs = 0, offset = 0, code = 0, has_fu = 0;
    uint16_t immediate = 0;

    if(has_error) return;

    if((!instruction_queue.size) && (!decode_queue.size) && (pc == num_instructions)) running = 0; // Temporary

    if(is_decode){
        printDebugMessage("---Execution stage (Decode/Reg)---");

        if(!instruction_queue.size){
            printDebugMessage("Instruction queue is empety, skipping");

            is_decode = 0;
            return;
        }

        while(decode_queue.size < MAX_DECODE_QUEUE_SIZE){
            if(!instruction_queue.size){
                printDebugMessage("Out of instructions. Stopping decode");

                is_decode = 0;
                return;
            }

            instruction = instruction_queue.head->data.instruction;

            // Decodification
            switch (instruction >> 26){
                case OP_DECODE_0:
                    op_type = SPECIAL;
                    op_code =  instruction & (unsigned int)63;

                    switch (op_code){
                        case ADD:
                        case AND:
                        case MOVN:
                        case MOVZ:
                        case NOP:
                        case NOR:
                        case OR:
                        case XOR:
                            if(has_functional_unit){ // Temporary, should verify specific functional unit
                                printDebugMessage("Has free ADD/LOGIC function unit, decoding");

                                has_fu = 1;
                                has_functional_unit = 0; // Temporary
                            }
                            else{
                                printDebugMessage("No free ADD/LOGIC function unit, decoding");
                                has_functional_unit = 1;

                                is_decode = 0;
                                return;
                            }

                            break;

                        case SUB:
                            if(has_functional_unit){ // Temporary, should verify specific functional unit
                                printDebugMessage("Has free SUB function unit, decoding");

                                has_fu = 1;
                                has_functional_unit = 0; // Temporary
                            }
                            else {
                                printDebugMessage("No free SUB function unit, decoding");
                                has_functional_unit = 1;

                                is_decode = 0;
                                return;
                            }

                            break;

                        case MFHI:
                        case MFLO:
                        case MTHI:
                        case MTLO:
                            if(has_functional_unit){ // Temporary, should verify specific functional unit
                                printDebugMessage("Has free HILO function unit, decoding");

                                has_fu = 1;
                                has_functional_unit = 0; // Temporary
                            }
                            else {
                                printDebugMessage("No free HILO function unit, decoding");
                                has_functional_unit = 1;

                                is_decode = 0;
                                return;
                            }

                            break;

                        case MULT:
                            if(has_functional_unit){ // Temporary, should verify specific functional unit
                                printDebugMessage("Has free MUL function unit, decoding");

                                has_fu = 1;
                                has_functional_unit = 0; // Temporary
                            }
                            else {
                                printDebugMessage("No free MUL function unit, decoding");
                                has_functional_unit = 1;

                                is_decode = 0;
                                return;
                            }

                            break;

                        case DIV:
                            if(has_functional_unit){ // Temporary, should verify specific functional unit
                                printDebugMessage("Has free DIV function unit, decoding");

                                has_fu = 1;
                                has_functional_unit = 0; // Temporary
                            }
                            else {
                                printDebugMessage("No free DIV function unit, decoding");
                                has_functional_unit = 1;

                                is_decode = 0;
                                return;
                            }

                            break;

                        case SYSCALL:
                            code = instruction >> 6;
                            instruction = popQueue(&instruction_queue).instruction;

                            break;

                        default:
                            printDebugError("Execution Stage", "Op code not found");
                            error();
                            break;
                    }

                    if(has_fu) {
                        instruction = popQueue(&instruction_queue).instruction;

                        rd = (instruction >> 11) & (unsigned int) 31;
                        rt = (instruction >> 16) & (unsigned int) 31;
                        rs = (instruction >> 21) & (unsigned int) 31;

                        // Register file read should happen here

                        data.instruction = instruction;
                        pushQueue(&decode_queue, data);
                    }

                    break;

                case OP_DECODE_1:
                    op_type = REGIMM;
                    op_code = ((instruction >> 16) & (unsigned int)31);

                    if(has_functional_unit){
                        printDebugMessage("Has free ADD/LOGIC function unit, decoding");

                        instruction = popQueue(&instruction_queue).instruction;

                        offset =  instruction & (unsigned int)65535;
                        rs = (instruction >> 21) & (unsigned int)31;

                        // Register file read should happen here

                        data.instruction = instruction;
                        pushQueue(&decode_queue, data);

                        has_functional_unit = 0;
                    }
                    else{
                        printDebugMessage("No free ADD/LOGIC function unit, decoding");

                        has_functional_unit = 1;

                        is_decode = 0;
                        return;
                    }

                    break;

                case OP_DECODE_2:
                    op_type = SPECIAL2;
                    op_code =  instruction & (unsigned int)63;

                    switch (op_code){
                        case MADD:
                            if(has_functional_unit){ // Temporary, should verify specific functional unit
                                printDebugMessage("Has free ADD/LOGIC function unit, decoding");

                                has_fu = 1;
                                has_functional_unit = 0; // Temporary
                            }
                            else{
                                printDebugMessage("No free ADD/LOGIC function unit, decoding");
                                has_functional_unit = 1;

                                is_decode = 0;
                                return;
                            }

                            break;

                        case MSUB:
                            if(has_functional_unit){ // Temporary, should verify specific functional unit
                                printDebugMessage("Has free SUB function unit, decoding");

                                has_fu = 1;
                                has_functional_unit = 0; // Temporary
                            }
                            else {
                                printDebugMessage("No free SUB function unit, decoding");
                                has_functional_unit = 1;

                                is_decode = 0;
                                return;
                            }

                            break;

                        case MUL:
                            if(has_functional_unit){ // Temporary, should verify specific functional unit
                                printDebugMessage("Has free MUL function unit, decoding");

                                has_fu = 1;
                                has_functional_unit = 0; // Temporary
                            }
                            else {
                                printDebugMessage("No free MUL function unit, decoding");
                                has_functional_unit = 1;

                                is_decode = 0;
                                return;
                            }

                            break;

                        default:
                            printDebugError("Execution Stage", "Op code not found");
                            error();
                            break;
                    }

                    if(has_fu) {
                        instruction = popQueue(&instruction_queue).instruction;

                        rd = (instruction >> 11) & (unsigned int) 31;
                        rt = (instruction >> 16) & (unsigned int) 31;
                        rs = (instruction >> 21) & (unsigned int) 31;

                        // Register file read should happen here

                        data.instruction = instruction;
                        pushQueue(&decode_queue, data);
                    }

                    break;

                default:
                    op_type = NONE;
                    op_code = instruction >> 26;

                    if(has_functional_unit){ // Temporary, should verify specific functional unit
                        printDebugMessage("Has free ADD/LOGIC function unit, decoding");

                        instruction = popQueue(&instruction_queue).instruction;

                        immediate = (uint16_t)(instruction & (unsigned int)65535);
                        rt = (instruction >> 16) & (unsigned int)31;
                        rs = (instruction >> 21) & (unsigned int)31;

                        // Register file read should happen here

                        data.instruction = instruction;
                        pushQueue(&decode_queue, data);

                        has_functional_unit = 0; // Temporary
                    }
                    else{
                        printDebugMessage("No free ADD/LOGIC function unit, decoding");
                        has_functional_unit = 1;

                        is_decode = 0;
                        return;
                    }

                    break;
            }
        }

        is_decode = 0;
    }
    else{
        printDebugMessage("---Execution stage (ALU Start)---");

        if(!decode_queue.size){
            printDebugMessage("Decode queue is empety, skipping");

            is_decode = 1;
            return;
        }

        while(decode_queue.size){
            data = popQueue(&decode_queue);

            printDebugMessage("Sending instruction to ALU");
            // Should initiates the alu for each instruction
        }

        is_decode = 1;
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