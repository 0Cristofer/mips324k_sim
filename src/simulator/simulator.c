/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 17/04/2019 */

#include "include/simulator.h"
#include "include/branchComponent.h"
#include "include/util.h"
#include "include/alu.h"

#include <stdlib.h>
#include <stdint.h>

int debug;
int has_error = 0;

unsigned int pc;
queue_t instruction_queue;
queue_t decode_queue;

void startSimulation(unsigned int *insts, unsigned int num_insts, int b){
    int i;

    debug = b;
    instructions = insts;
    running = 1;
    num_instructions = num_insts;
    pc = 0;

    initQueue(&instruction_queue);
    initQueue(&decode_queue);
    initAlu();
    initBranchPredictor();

    for(i = 0; i < NUM_REGISTERS; i++){
        registers[i] = 1;
    }

    clock();

    printDebugMessageInt("Register t3", registers[11]);

    cleanup();
}

void clock(){
    int cycle = 0;

    while(running){
        printDebugMessageInt("************** Cycle ************", cycle);

        pipeline();

        cycle++;
    }
}

void pipeline(){
    writeback();
    alignAccumulate();
    memory();
    execution();
    instruction();
}

void instruction(){
    int next_pc;
    queue_data_t data;

    if(has_error) return;

    printDebugMessage("---Instruction stage---");

    if(instruction_queue.size){ // If there is elements in the instruction queue, do nothing
        printDebugMessage("Instruction queue has elements, skipping fetch");
        return;
    }

    if(pc == num_instructions){
        printDebugMessage("PC at the end of program, skipping fetch");
        return;
    }

    printDebugMessage("Fetching instructions");

    while(instruction_queue.size < MAX_INST_QUEUE_SIZE){
        if(pc == num_instructions){
            printDebugMessage("Out of instructions. Stopping fetch");
            return;
        }

        data.instruction = malloc(sizeof(instruction_data_t));

        data.instruction->instruction = instructions[pc];
        data.instruction->speculative_insts = NULL;
        data.instruction->is_speculate = 0;
        data.instruction->pc = pc;

        printDebugMessageInt("\tFetched instruction", pc);

        pushQueue(&instruction_queue, data);

        next_pc = branchComponent(pc);

        updatePc(next_pc); // The PC increment will be given from the branch component
    }
}

void execution(){
    static int is_decode = 1;
    queue_data_t data;

    unsigned int instruction, op_type, op_code, rd = 0, rt = 0, rs = 0, offset = 0, code = 0;
    int has_functional_unit = 1;
    uint16_t immediate = 0;
    instruction_data_t *inst_data;

    if(has_error) return;

    if((!instruction_queue.size) && (!decode_queue.size) && (pc == num_instructions)) running = 0; // Temporary

    if(is_decode){
        printDebugMessage("---Execution stage (Decode/Reg)---");

        if(!instruction_queue.size){
            printDebugMessage("Instruction queue is empety, skipping");
        }
        else {
            while ((decode_queue.size < MAX_DECODE_QUEUE_SIZE) && has_functional_unit) {
                if (!instruction_queue.size) {
                    printDebugMessage("Out of instructions. Stopping decode");
                    break;
                }
                else {
                    inst_data = instruction_queue.head->data.instruction;
                    instruction = inst_data->instruction;

                    // Decodification
                    switch (instruction >> (unsigned int)26) {
                        case OP_DECODE_0:
                            op_type = SPECIAL;
                            op_code = instruction & (unsigned int) 63;
                            rd = (instruction >> (unsigned int)11) & (unsigned int) 31;

                            switch (op_code) {
                                case ADD:
                                case AND:
                                case MOVN:
                                case MOVZ:
                                case MFHI:
                                case MFLO:
                                case MTHI:
                                case MTLO:
                                case NOP:
                                case NOR:
                                case OR:
                                case XOR:
                                    has_functional_unit = hasFuAdd() && isRegFree(rd);
                                    if (has_functional_unit)
                                        printDebugMessage("Has free ADD/LOGIC/MOVE function unit, decoding");
                                    else
                                        printDebugMessage("No free ADD/LOGIC/MOVE function unit, stopping");

                                    break;

                                case SUB:
                                    has_functional_unit = hasFuSub() && isRegFree(rd);
                                    if (has_functional_unit)
                                        printDebugMessage("Has free SUB function unit, decoding");
                                    else
                                        printDebugMessage("No free SUB function unit, stopping");

                                    break;

                                case MULT:
                                    has_functional_unit = hasFuMul() && isRegFree(rd);
                                    if (has_functional_unit)
                                        printDebugMessage("Has free MUL function unit, decoding");
                                    else
                                        printDebugMessage("No free MUL function unit, stopping");

                                    break;

                                case DIV:
                                    has_functional_unit = hasFuDiv() && isRegFree(rd);
                                    if (has_functional_unit)
                                        printDebugMessage("Has free DIV function unit, decoding");
                                    else
                                        printDebugMessage("No free DIV function unit, stopping");

                                    break;

                                case SYSCALL:
                                    printDebugMessage("Syscall, decoding");

                                    code = instruction >> (unsigned int)6;
                                    instruction = popQueue(&instruction_queue).instruction->instruction;

                                    break;

                                default:
                                    printDebugError("Execution Stage", "Op code not found");
                                    has_functional_unit = 0;
                                    error();

                                    break;
                            }

                            if (has_functional_unit) {
                                instruction = popQueue(&instruction_queue).instruction->instruction;

                                rt = (instruction >> (unsigned int)16) & (unsigned int) 31;
                                rs = (instruction >> (unsigned int)21) & (unsigned int) 31;

                                data.instruction = inst_data;
                                data.instruction->instruction = instruction;
                                data.instruction->op_type = op_type;
                                data.instruction->op_code = op_code;
                                data.instruction->rd = rd;
                                data.instruction->rt = rt;
                                data.instruction->rs = rs;

                                pushQueue(&decode_queue, data);
                            }

                            break;

                        case OP_DECODE_1:
                            op_type = REGIMM;
                            op_code = ((instruction >> (unsigned int)16) & (unsigned int) 31);

                            has_functional_unit = hasFuAdd();

                            if (has_functional_unit) {
                                printDebugMessage("Has free ADD/LOGIC/MOVE function unit, decoding");

                                instruction = popQueue(&instruction_queue).instruction->instruction;

                                offset = (uint16_t) instruction & (unsigned int) 65535;
                                rs = (instruction >> (unsigned int)21) & (unsigned int) 31;

                                data.instruction = inst_data;
                                data.instruction->instruction = instruction;
                                data.instruction->op_type = op_type;
                                data.instruction->op_code = op_code;
                                data.instruction->rs = rs;
                                data.instruction->imm = offset;

                                pushQueue(&decode_queue, data);
                            }
                            else
                                printDebugMessage("No free ADD/LOGIC/MOVE function unit, stopping");

                            break;

                        case OP_DECODE_2:
                            op_type = SPECIAL2;
                            op_code = instruction & (unsigned int) 63;
                            rd = (instruction >> (unsigned int)11) & (unsigned int) 31;

                            has_functional_unit = hasFuMul() && isRegFree(rd);

                            if (has_functional_unit) {
                                printDebugMessage("Has free MUL function unit, decoding");

                                instruction = popQueue(&instruction_queue).instruction->instruction;

                                rt = (instruction >> (unsigned int)16) & (unsigned int) 31;
                                rs = (instruction >> (unsigned int)21) & (unsigned int) 31;

                                data.instruction = inst_data;
                                data.instruction->instruction = instruction;
                                data.instruction->op_type = op_type;
                                data.instruction->op_code = op_code;
                                data.instruction->rd = rd;
                                data.instruction->rt = rt;
                                data.instruction->rs = rs;

                                pushQueue(&decode_queue, data);
                            }
                            else
                                printDebugMessage("No free MUL function unit, decoding");

                            break;

                        default:
                            op_type = NONE;
                            op_code = instruction >> (unsigned int)26;
                            rt = (instruction >> (unsigned int)16) & (unsigned int) 31;

                            has_functional_unit = hasFuAdd() && isRegFree(rd);

                            if (has_functional_unit) {
                                printDebugMessage("Has free ADD/LOGIC/MOVE function unit, decoding");

                                instruction = popQueue(&instruction_queue).instruction->instruction;

                                immediate = (uint16_t) (instruction & (unsigned int) 65535);
                                rs = (instruction >> (unsigned int)21) & (unsigned int) 31;

                                data.instruction = inst_data;
                                data.instruction->instruction = instruction;
                                data.instruction->op_type = op_type;
                                data.instruction->op_code = op_code;
                                data.instruction->rt = rt;
                                data.instruction->rs = rs;
                                data.instruction->imm = immediate;

                                pushQueue(&decode_queue, data);
                            }
                            else
                                printDebugMessage("No free ADD/LOGIC/MOVE function unit, stopping");

                            break;
                    }
                }
            }
        }
        is_decode = 0;
    }
    else{
        printDebugMessage("---Execution stage (ALU Start)---");

        if(!decode_queue.size){
            printDebugMessage("Decode queue is empety, skipping");
        }
        else {
            while (decode_queue.size) {
                data = popQueue(&decode_queue);

                switch (data.instruction->op_type){
                    case OP_DECODE_0:
                        switch (data.instruction->op_code) {
                            case ADD:
                            case AND:
                            case MOVN:
                            case MOVZ:
                            case MFHI:
                            case MFLO:
                            case MTHI:
                            case MTLO:
                            case NOP:
                            case NOR:
                            case OR:
                            case XOR:
                                printDebugMessage("Sending instruction to ALU (ADD/LOGIC/MOVE)");
                                allocFuAdd(data.instruction);
                                break;

                            case SUB:
                                printDebugMessage("Sending instruction to ALU (SUB)");
                                allocFuSub(data.instruction);
                                break;

                            case MULT:
                                printDebugMessage("Sending instruction to ALU (MUL)");
                                allocFuMul(data.instruction);
                                break;

                            case DIV:
                                printDebugMessage("Sending instruction to ALU (DIV)");
                                allocFuDiv(data.instruction);
                                break;

                            case SYSCALL:
                                printDebugMessage("Sending instruction to ALU (SYSCALL)");

                                break;

                            default:
                                printDebugError("Execution Stage", "Op code not found");
                                error();

                                break;
                        }
                        break;
                    case OP_DECODE_1:
                        printDebugMessage("Sending instruction to ALU (ADD/LOGIC/MOVE)");
                        allocFuAdd(data.instruction);
                        break;
                    case OP_DECODE_2:
                        printDebugMessage("Sending instruction to ALU (MUL)");
                        allocFuMul(data.instruction);
                        break;
                    default:
                        printDebugMessage("Sending instruction to ALU (ADD/LOGIC/MOVE)");
                        allocFuAdd(data.instruction);
                        break;
                }
            }

        }
        runAlu();


        is_decode = 1;
    }
}

void memory(){
    if(has_error) return;
    printDebugMessage("---Memory Stage---");
}

void alignAccumulate(){
    if(has_error) return;
    printDebugMessage("---Allign/Accumulate Stage---");
}

void writeback(){
    if(has_error) return;
    printDebugMessage("---Writeback Stage---");
}

void updatePc(int next_pc){
    pc = pc + next_pc;
}

void cleanup(){
    clearQueue(&instruction_queue);
    clearQueue(&decode_queue);
}

void error(){
    has_error = 1;
    running = 0;
    printDebugMessage("An error happend, stopping.");
}