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

    unsigned int instruction, op_type, op_code, rd = 0, rt = 0, rs = 0, offset = 0;
    int has_functional_unit = 1;
    uint16_t immediate = 0;
    instruction_data_t *inst_data;
    functional_unit_t *f = NULL;

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
                                case NOR:
                                case OR:
                                case XOR:
                                case MOVN:
                                case MOVZ:
                                case MFHI:
                                case MFLO:
                                    f = hasFuAdd();
                                    has_functional_unit = (f != NULL) && isRegFree(rd);
                                    if (has_functional_unit) {
                                        printDebugMessage(
                                                "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                        alocReg(rd, f);
                                        f->busy = 1;
                                        f->cicles_to_end = cicles_add[add_map[op_code]];
                                    }
                                    else
                                        printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                    break;

                                case MTHI:
                                    f = hasFuAdd();
                                    has_functional_unit = (f != NULL) && isRegFree(HI_REG);
                                    if (has_functional_unit) {
                                        printDebugMessage(
                                                "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                        alocReg(HI_REG, f);
                                        f->busy = 1;
                                        f->cicles_to_end = cicles_add[add_map[op_code]];
                                    }
                                    else
                                        printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                    break;
                                case MTLO:
                                    f = hasFuAdd();
                                    has_functional_unit = (f != NULL) && isRegFree(LO_REG);
                                    if (has_functional_unit) {
                                        printDebugMessage(
                                                "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                        alocReg(LO_REG, f);
                                        f->busy = 1;
                                        f->cicles_to_end = cicles_add[add_map[op_code]];
                                    }
                                    else
                                        printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                    break;

                                case SUB:
                                    f = hasFuSub();
                                    has_functional_unit = (f != NULL) && isRegFree(rd);
                                    if (has_functional_unit) {
                                        printDebugMessage("Has free SUB function unit, decoding");
                                        alocReg(rd, f);
                                        f->busy = 1;
                                        f->cicles_to_end = cicles_sub[sub_map[op_code]];
                                    }
                                    else
                                        printDebugMessage("No free SUB function unit, stopping");

                                    break;

                                case MULT:
                                    f = hasFuMul();
                                    has_functional_unit = (f != NULL) && isRegFree(HI_REG) && isRegFree(LO_REG);
                                    if (has_functional_unit) {
                                        printDebugMessage("Has free MUL function unit (with dest), decoding");
                                        alocReg(HI_REG, f);
                                        alocReg(LO_REG, f);
                                        f->busy = 1;
                                        f->cicles_to_end = cicles_mul[mul_map[op_code]];
                                    }
                                    else
                                        printDebugMessage("No free MUL function unit (with dest), stopping");

                                    break;

                                case DIV:
                                    f = hasFuDiv();
                                    has_functional_unit = (f != NULL) && isRegFree(HI_REG) && isRegFree(LO_REG);
                                    if (has_functional_unit) {
                                        printDebugMessage("Has free DIV function unit (with dest), decoding");
                                        alocReg(HI_REG, f);
                                        alocReg(LO_REG, f);
                                        f->busy = 1;
                                        f->cicles_to_end = cicles_div[div_map[op_code]];
                                    }
                                    else
                                        printDebugMessage("No free DIV function unit (with dest), stopping");

                                    break;

                                default:
                                    printDebugError("Execution Stage", "Op code not found");
                                    has_functional_unit = 0;
                                    error();

                                    break;
                            }

                            if (has_functional_unit) {
                                inst_data->f = f;
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

                            f = hasFuAdd();
                            has_functional_unit = (f != NULL);

                            if (has_functional_unit) {
                                printDebugMessage("Has free ADD/LOGIC/MOVE function unit (without dest), decoding");
                                f->busy = 1;
                                f->cicles_to_end = cicles_add[add_map[op_code]];
                                inst_data->f = f;

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
                                printDebugMessage("No free ADD/LOGIC/MOVE function unit (without dest), stopping");

                            break;

                        case OP_DECODE_2:
                            op_type = SPECIAL2;
                            op_code = instruction & (unsigned int) 63;
                            rd = (instruction >> (unsigned int)11) & (unsigned int) 31;

                            f = hasFuMul();
                            has_functional_unit = (f != NULL) && isRegFree(HI_REG) && isRegFree(LO_REG);

                            if(op_code == MUL) has_functional_unit = has_functional_unit && isRegFree(rd);

                            if (has_functional_unit) {
                                printDebugMessage("Has free MUL function unit (with dest), decoding");
                                alocReg(HI_REG, f);
                                alocReg(LO_REG, f);
                                if(op_code == MUL) alocReg(rd, f);
                                f->busy = 1;
                                f->cicles_to_end = cicles_mul[mul_map[op_code]];
                                inst_data->f = f;

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
                                printDebugMessage("No free MUL function unit (with dest), decoding");

                            break;

                        default:
                            op_type = NONE;
                            op_code = instruction >> (unsigned int)26;
                            rt = (instruction >> (unsigned int)16) & (unsigned int) 31;

                            switch (op_code){
                                case ADDI:
                                case ANDI:
                                case ORI:
                                case XORI:
                                case LUI:
                                    f = hasFuAdd();
                                    has_functional_unit = (f != NULL) && isRegFree(rt);
                                    if (has_functional_unit) {
                                        printDebugMessage(
                                                "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                        alocReg(rt, f);
                                        f->busy = 1;
                                    }
                                    else
                                        printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                    break;

                                case BEQ:
                                case BEQL:
                                case BGTZ:
                                case BLEZ:
                                case BNE:
                                    f = hasFuAdd();
                                    has_functional_unit = (f != NULL);
                                    if (has_functional_unit) {
                                        printDebugMessage(
                                                "Has free ADD/LOGIC/MOVE function unit (without dest), decoding");
                                        f->busy = 1;
                                    }
                                    else
                                        printDebugMessage("No free ADD/LOGIC/MOVE function unit (without dest), stopping");

                                    break;

                                default:
                                    printDebugError("Execution Stage", "Op code not found");
                                    has_functional_unit = 0;
                                    error();

                                    break;
                            }


                            if (has_functional_unit) {
                                inst_data->f = f;
                                f->cicles_to_end = cicles_add[add_map[op_code]];

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
            while (decode_queue.size && (!has_error)) {
                data = popQueue(&decode_queue);//decode_queue.head->data;

                switch (data.instruction->op_type){
                    case OP_DECODE_0:
                        switch (data.instruction->op_code) {
                            case ADD:
                            case AND:
                            case NOR:
                            case OR:
                            case XOR:
                            case MOVN:
                            case MOVZ:
                                if(isRegFree(data.instruction->rs) && isRegFree(data.instruction->rt)) {
                                    printDebugMessage("Sending instruction (2 src) to ALU (ADD/LOGIC/MOVE)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fi = data.instruction->rd;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->fk = data.instruction->rt;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->rk = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                    data.instruction->f->dk = registers[data.instruction->rt];
                                }
                                else
                                    printDebugMessage("Registers no ready (2 src) to ALU (ADD/LOGIC/MOVE)");

                                break;

                            case MTHI:
                                if(isRegFree(data.instruction->rs)) {
                                    printDebugMessage("Sending instruction (1 src) to ALU (ADD/LOGIC/MOVE)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fi = HI_REG;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                }
                                else
                                    printDebugMessage("Registers no ready (1 src) to ALU (ADD/LOGIC/MOVE)");
                            case MTLO:
                                if(isRegFree(data.instruction->rs)) {
                                    printDebugMessage("Sending instruction (1 src) to ALU (ADD/LOGIC/MOVE)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fi = LO_REG;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                }
                                else
                                    printDebugMessage("Registers no ready (1 src) to ALU (ADD/LOGIC/MOVE)");

                                break;

                            case MFHI:
                                if(isRegFree(HI_REG)) {
                                    printDebugMessage("Sending instruction (1 src) to ALU (ADD/LOGIC/MOVE)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fi = data.instruction->rd;
                                    data.instruction->f->fj = HI_REG;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->dj = registers[HI_REG];
                                }
                                else
                                    printDebugMessage("Registers no ready (1 src) to ALU (ADD/LOGIC/MOVE)");

                                break;
                            case MFLO:
                                if(isRegFree(LO_REG)) {
                                    printDebugMessage("Sending instruction (1 src) to ALU (ADD/LOGIC/MOVE)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fi = data.instruction->rd;
                                    data.instruction->f->fj = LO_REG;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->dj = registers[LO_REG];
                                }
                                else
                                    printDebugMessage("Registers no ready (1 src) to ALU (ADD/LOGIC/MOVE)");

                                break;

                            case SUB:
                                if(isRegFree(data.instruction->rs) && isRegFree(data.instruction->rt)) {
                                    printDebugMessage("Sending instruction (2 src) to ALU (SUB)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fi = data.instruction->rd;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->fk = data.instruction->rt;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->rk = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                    data.instruction->f->dk = registers[data.instruction->rt];
                                }
                                else
                                    printDebugMessage("Registers no ready (2 src) to ALU (SUB)");

                                break;

                            case MULT:
                                if(isRegFree(data.instruction->rs) && isRegFree(data.instruction->rt)) {
                                    printDebugMessage("Sending instruction (2 src) to ALU (MUL)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->fk = data.instruction->rt;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->rk = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                    data.instruction->f->dk = registers[data.instruction->rt];
                                }
                                else
                                    printDebugMessage("Registers no ready (2 src) to ALU (MUL)");

                                break;

                            case DIV:
                                if(isRegFree(data.instruction->rs) && isRegFree(data.instruction->rt)) {
                                    printDebugMessage("Sending instruction (2 src) to ALU (DIV)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->fk = data.instruction->rt;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->rk = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                    data.instruction->f->dk = registers[data.instruction->rt];
                                }
                                else
                                    printDebugMessage("Registers no ready (2 src) to ALU (DIV)");

                                break;

                            default:
                                printDebugError("Execution Stage", "Op code not found");
                                error();

                                break;
                        }
                        break;
                    case OP_DECODE_1:
                        if(isRegFree(data.instruction->rs)) {
                            printDebugMessage("Sending instruction (1 src) to ALU (ADD/LOGIC/MOVE)");
                            data.instruction->f->instruction = data.instruction;
                            data.instruction->f->op = data.instruction->op_code;
                            data.instruction->f->fj = data.instruction->rs;
                            data.instruction->f->rj = 0;
                            data.instruction->f->dj = registers[data.instruction->rs];
                        }
                        else
                            printDebugMessage("Registers no ready (2 src) to ALU (ADD/LOGIC/MOVE)");

                        break;
                    case OP_DECODE_2:
                        if(isRegFree(data.instruction->rs) && isRegFree(data.instruction->rt)) {
                            printDebugMessage("Sending instruction (2 src) to ALU (MUL)");
                            data.instruction->f->instruction = data.instruction;
                            data.instruction->f->op = data.instruction->op_code;
                            if(data.instruction->op_code == MUL) data.instruction->f->fi = data.instruction->rd;
                            data.instruction->f->fj = data.instruction->rs;
                            data.instruction->f->fk = data.instruction->rt;
                            data.instruction->f->rj = 0;
                            data.instruction->f->rk = 0;
                            data.instruction->f->dj = registers[data.instruction->rs];
                            data.instruction->f->dk = registers[data.instruction->rt];
                        }
                        else
                            printDebugMessage("Registers no ready (2 src) to ALU (MUL)");

                        break;
                    default:
                        switch (data.instruction->op_code){
                            case ADDI:
                            case ANDI:
                            case ORI:
                            case XORI:
                            case LUI:
                                if(isRegFree(data.instruction->rs)) {
                                    printDebugMessage("Sending instruction (1 src + imm) to ALU (ADD/LOGIC/MOVE)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fi = data.instruction->rt;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->rk = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                    data.instruction->f->dk = data.instruction->imm;
                                }
                                else
                                    printDebugMessage("Registers no ready (1 src + imm) to ALU (ADD/LOGIC/MOVE)");

                                break;

                            case BEQ:
                            case BEQL:
                            case BNE:
                                if(isRegFree(data.instruction->rs) && isRegFree(data.instruction->rt)) {
                                    printDebugMessage("Sending instruction (2 src) to ALU (ADD/LOGIC/MOVE)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->fk = data.instruction->rt;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->rk = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                    data.instruction->f->dk = registers[data.instruction->rt];
                                }
                                else
                                    printDebugMessage("Registers no ready (2 src) to ALU (ADD/LOGIC/MOVE)");

                                break;
                            case BGTZ:
                            case BLEZ:
                                if(isRegFree(data.instruction->rs)) {
                                    printDebugMessage("Sending instruction (1 src) to ALU (ADD/LOGIC/MOVE)");
                                    data.instruction->f->instruction = data.instruction;
                                    data.instruction->f->op = data.instruction->op_code;
                                    data.instruction->f->fj = data.instruction->rs;
                                    data.instruction->f->rj = 0;
                                    data.instruction->f->rk = 0;
                                    data.instruction->f->dj = registers[data.instruction->rs];
                                }
                                else
                                    printDebugMessage("Registers no ready (1 src) to ALU (ADD/LOGIC/MOVE)");

                                break;

                            default:
                                printDebugError("Execution Stage", "Op code not found");
                                has_functional_unit = 0;
                                error();

                                break;
                        }
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