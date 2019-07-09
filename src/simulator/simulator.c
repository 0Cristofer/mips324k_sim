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
queue_t rob_queue;

void startSimulation(unsigned int *insts, unsigned int num_insts, int b){
    debug = b;
    instructions = insts;
    running = 1;
    num_instructions = num_insts;
    pc = 0;

    initQueue(&instruction_queue);
    initQueue(&rob_queue);
    initAlu();
    initBranchPredictor();

    registers[ZERO] = 0;
    registers[T0] = 0;

    clock();

    printRegister(T2);

    cleanup();
}

void clock(){
    int cycle = 0;

    while(running && (!has_error)){
        printDebugMessageInt("************** Cycle ************", cycle);

        pipeline();

        cycle++;
    }
}

void pipeline(){
    effect();
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
        data.instruction->is_ready = 0;
        data.instruction->pc = pc;
        data.instruction->discard = 0;

        printDebugMessageInt("\tFetched instruction", pc);

        pushQueue(&instruction_queue, data);

        next_pc = branchComponent(pc);

        updatePc(next_pc); // The PC increment will be given from the branch component
    }
}

void execution(){
    queue_data_t rob_data;

    unsigned int instruction, op_type, op_code, rd = 0, rt = 0, rs = 0, offset = 0;
    int has_functional_unit = 1;
    uint16_t immediate = 0;
    instruction_data_t *inst_data;
    functional_unit_t *f = NULL;

    if(has_error) return;

    printDebugMessage("---Execution stage---");

    // Always run the ALU, if queues are empety and nothing ran, stop running
    running = runAlu() || (!((!instruction_queue.size) && (pc == num_instructions) && (!rob_queue.size)));

    if(!instruction_queue.size){
        printDebugMessage("Instruction queue is empety, skipping");
    }
    else {
        while (has_functional_unit && (rob_queue.size < MAX_ROB_QUEUE_SIZE)){
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
                        rt = (instruction >> (unsigned int)16) & (unsigned int) 31;
                        rs = (instruction >> (unsigned int)21) & (unsigned int) 31;

                        switch (op_code) {
                            case ADD:
                            case AND:
                            case NOR:
                            case OR:
                            case XOR:
                            case MOVN:
                            case MOVZ:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rd, WRITE) && isRegFree(rs, READ) &&
                                        isRegFree(rt, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding", inst_data->pc);

                                    f->fi = rd;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->dk = readReg(rt);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(rd);
                                }
                                else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping", inst_data->pc);

                                break;

                            case MFHI:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rd, WRITE) && isRegFree(HI_REG, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding", inst_data->pc);
                                    f->fi = rd;
                                    f->fj = HI_REG;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(HI_REG);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(rd);
                                }
                                else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping", inst_data->pc);

                                break;

                            case MFLO:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rd, WRITE) && isRegFree(LO_REG, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding", inst_data->pc);
                                    f->fi = rd;
                                    f->fj = LO_REG;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(LO_REG);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(rd);
                                }
                                else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping", inst_data->pc);

                                break;

                            case MTHI:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(HI_REG, WRITE) && isRegFree(rs, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding", inst_data->pc);
                                    f->fi = HI_REG;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(HI_REG);
                                }
                                else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping", inst_data->pc);

                                break;

                            case MTLO:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(LO_REG, WRITE) && isRegFree(rs, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding", inst_data->pc);
                                    f->fi = LO_REG;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(LO_REG);
                                }
                                else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping", inst_data->pc);

                                break;

                            case SUB:
                                f = hasFuSub();
                                has_functional_unit = (f != NULL) && isRegFree(rd, WRITE) && isRegFree(rs, READ) &&
                                        isRegFree(rt, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free SUB function unit (with dest), decoding", inst_data->pc);
                                    f->fi = rd;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->dk = readReg(rt);
                                    f->cicles_to_end = cicles_sub[sub_map[op_code]];

                                    alocReg(rd);
                                }
                                else
                                    printDebugMessageInt("No free SUB function unit (with dest), stopping", inst_data->pc);

                                break;

                            case MULT:
                                f = hasFuMul();
                                has_functional_unit = (f != NULL) && isRegFree(HI_REG, WRITE) && isRegFree(LO_REG, WRITE)
                                                                  && isRegFree(rs, READ) && isRegFree(rt, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt("Has free MUL function unit (with dest), decoding", inst_data->pc);
                                    f->fi = -1;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->dk = readReg(rt);
                                    f->cicles_to_end = cicles_mul[mul_map[op_code]];

                                    alocReg(HI_REG);
                                    alocReg(LO_REG);
                                }
                                else
                                    printDebugMessageInt("No free MUL function unit (with dest), stopping", inst_data->pc);

                                break;

                            case DIV:
                                f = hasFuDiv();
                                has_functional_unit = (f != NULL) && isRegFree(HI_REG, WRITE) && isRegFree(LO_REG, WRITE)
                                                                  && isRegFree(rs, READ) && isRegFree(rt, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt("Has free DIV function unit (with dest), decoding", inst_data->pc);
                                    f->fi = -1;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->dk = readReg(rt);
                                    f->cicles_to_end = cicles_div[div_map[op_code]];

                                    alocReg(HI_REG);
                                    alocReg(LO_REG);
                                }
                                else
                                    printDebugMessageInt("No free DIV function unit (with dest), stopping", inst_data->pc);

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

                            f->busy = 1;
                            f->instruction = inst_data;
                            f->op = op_code;

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rd  = f->fi;
                            inst_data->rs = f->fj;
                            inst_data->rt = f->fk;

                            if((op_code == MOVN) || (op_code == MOVZ) || (op_code == MTHI) || (op_code == MTLO)
                               || (op_code == MFHI) || (op_code == MFLO))
                                inst_data->write_flag = IS_MOVE;
                            else if((op_code == DIV) || (op_code == MULT))
                                inst_data->write_flag = IS_MUL;
                            else
                                inst_data->write_flag = IS_NORMAL;

                            rob_data.entry = malloc(sizeof(rob_entry_t));
                            rob_data.entry->instruction = inst_data;

                            rob_data.entry->state = NOT_READY;
                            inst_data->entry = rob_data.entry;

                            pushQueue(&rob_queue, rob_data);
                        }

                        break;

                    case OP_DECODE_1:
                        op_type = REGIMM;
                        op_code = ((instruction >> (unsigned int)16) & (unsigned int) 31);
                        rs = (instruction >> (unsigned int)21) & (unsigned int) 31;

                        f = hasFuAdd();
                        has_functional_unit = (f != NULL) && isRegFree(rs, READ);

                        if (has_functional_unit) {
                            printDebugMessageInt("Has free ADD/LOGIC/MOVE function unit (without dest), decoding", inst_data->pc);
                            inst_data->f = f;

                            instruction = popQueue(&instruction_queue).instruction->instruction;

                            offset = (uint16_t) instruction & (unsigned int) 65535;

                            f->busy = 1;
                            f->instruction = inst_data;
                            f->op = op_code;

                            f->fi = -1;
                            f->fj = rs;
                            f->fk = -1;
                            f->rj = 1;
                            f->rk = 1;
                            f->dj = readReg(rs);
                            f->cicles_to_end = cicles_add[add_map[op_code]];

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rs = rs;
                            inst_data->imm = offset;
                            inst_data->write_flag = 1;
                            inst_data->write_flag = IS_BRANCH;

                            rob_data.entry = malloc(sizeof(rob_entry_t));
                            rob_data.entry->instruction = inst_data;

                            rob_data.entry->state = NOT_READY;
                            inst_data->entry = rob_data.entry;

                            pushQueue(&rob_queue, rob_data);
                        }
                        else
                            printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (without dest), stopping", inst_data->pc);

                        break;

                    case OP_DECODE_2:
                        op_type = SPECIAL2;
                        op_code = instruction & (unsigned int) 63;
                        rt = (instruction >> (unsigned int)16) & (unsigned int) 31;
                        rs = (instruction >> (unsigned int)21) & (unsigned int) 31;
                        rd = (instruction >> (unsigned int)11) & (unsigned int) 31;

                        f = hasFuMul();
                        has_functional_unit = (f != NULL) && isRegFree(HI_REG, WRITE) && isRegFree(LO_REG, WRITE) &&
                                isRegFree(rs, READ) && isRegFree(rt, READ);

                        if(op_code == MUL) has_functional_unit = has_functional_unit && isRegFree(rd, WRITE);

                        if (has_functional_unit) {
                            printDebugMessageInt("Has free MUL function unit (with dest), decoding", inst_data->pc);
                            inst_data->f = f;

                            instruction = popQueue(&instruction_queue).instruction->instruction;

                            f->busy = 1;
                            f->instruction = inst_data;
                            f->op = op_code;
                            if(op_code == MUL) f->fi = rd;
                            else f->fi = -1;
                            f->fj = rs;
                            f->fk = rt;
                            f->rj = 1;
                            f->rk = 1;
                            f->dj = readReg(rs);
                            f->dk = readReg(rt);
                            f->cicles_to_end = cicles_mul[mul_map[op_code]];

                            alocReg(HI_REG);
                            alocReg(LO_REG);
                            if(op_code == MUL) alocReg(rd);

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rd = f->fi;
                            inst_data->rs = rs;
                            inst_data->rt = rt;
                            inst_data->write_flag = IS_MUL;

                            rob_data.entry = malloc(sizeof(rob_entry_t));
                            rob_data.entry->instruction = inst_data;

                            rob_data.entry->state = NOT_READY;
                            inst_data->entry = rob_data.entry;

                            pushQueue(&rob_queue, rob_data);
                        }
                        else
                            printDebugMessageInt("No free MUL function unit (with dest), decoding", inst_data->pc);

                        break;

                    default:
                        op_type = NONE;
                        op_code = instruction >> (unsigned int)26;
                        rs = (instruction >> (unsigned int)21) & (unsigned int) 31;
                        rt = (instruction >> (unsigned int)16) & (unsigned int) 31;
                        immediate = (uint16_t) (instruction & (unsigned int) 65535);

                        switch (op_code){
                            case ADDI:
                            case ANDI:
                            case ORI:
                            case XORI:
                            case LUI:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rt, WRITE) && isRegFree(rs, READ);
                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC function unit (with dest), decoding", inst_data->pc);
                                    f->fi = rt;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->dk = immediate;

                                    alocReg(rt);
                                }
                                else
                                    printDebugMessageInt("No free ADD/LOGIC function unit (with dest), stopping", inst_data->pc);

                                break;

                            case BEQ:
                            case BEQL:
                            case BNE:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rs, READ) && isRegFree(rt, READ);
                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC function unit (without dest), decoding", inst_data->pc);

                                    f->fi = -1;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->dk = readReg(rt);
                                }
                                else
                                    printDebugMessageInt("No free ADD/LOGIC function unit (without dest), stopping", inst_data->pc);

                                break;

                            case BGTZ:
                            case BLEZ:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rs, READ);
                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC function unit (without dest), decoding", inst_data->pc);

                                    f->fi = -1;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                }
                                else
                                    printDebugMessageInt("No free ADD/LOGIC function unit (without dest), stopping", inst_data->pc);

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

                            f->busy = 1;
                            f->instruction = inst_data;
                            f->op = op_code;

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rd  = f->fi;
                            inst_data->rs = f->fj;
                            inst_data->rt = f->fk;
                            inst_data->imm = immediate;

                            if((op_code == BEQ) || (op_code == BEQL) || (op_code == BNE) || (op_code == BGTZ)
                               || (op_code == BLEZ))
                                inst_data->write_flag = IS_BRANCH;
                            else
                                inst_data->write_flag = IS_NORMAL;

                            rob_data.entry = malloc(sizeof(rob_entry_t));
                            rob_data.entry->instruction = inst_data;

                            rob_data.entry->state = NOT_READY;
                            inst_data->entry = rob_data.entry;

                            pushQueue(&rob_queue, rob_data);
                        }

                        break;
                }
            }
        }
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

    write();
}

void effect(){
    queue_data_t data;

    if(has_error) return;
    printDebugMessage("---Effect Stage---");

    while(rob_queue.size){
        data = rob_queue.head->data;

        if(data.entry->state == READY){
            if(!data.entry->instruction->is_speculate){
                data = popQueue(&rob_queue);

                if(!data.entry->instruction->discard) {
                    if(data.entry->out_reg != IS_BRANCH)  // Mudara para um switch case com as outras flags
                        registers[data.entry->out_reg] = data.entry->data;

                    printDebugMessageInt("Commited instruction", data.entry->instruction->pc);
                }
                else
                    printDebugMessageInt("Discarded instruction", data.entry->instruction->pc);

                if(data.entry->out_reg != IS_BRANCH) // Mudara para um switch case com as outras flags
                    freeReg(data.entry->out_reg);

                free(data.entry->instruction);
                free(data.entry);
            }
            else{
                printDebugMessageInt("Instruction is speculative", data.entry->instruction->pc);
                break;
            }
        }
        else {
            printDebugMessageInt("Instruction is not ready", data.entry->instruction->pc);
            break;
        }
    }
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