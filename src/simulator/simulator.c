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
    int i;

    debug = b;
    instructions = insts;
    running = 1;
    num_instructions = num_insts;
    pc = 0;

    initQueue(&instruction_queue);
    initQueue(&rob_queue);
    initAlu();
    initBranchPredictor();

    for(i = 0; i < NUM_REGISTERS; i++){
        registers[i] = 0;
    }

    clock();

    printDebugMessageInt("Register v0", registers[2]);

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
        data.instruction->is_branch = 0;
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
                                has_functional_unit = (f != NULL) && isRegFree(rd) && isRegFree(rs) && isRegFree(rt);

                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                    alocReg(rd, f);

                                    f->fi = rd;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    f->dk = registers[rt];
                                    f->cicles_to_end = cicles_add[add_map[op_code]];
                                }
                                else
                                    printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                break;

                            case MFHI:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rd) && isRegFree(HI_REG);

                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                    alocReg(rd, f);

                                    f->fi = rd;
                                    f->fj = HI_REG;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[HI_REG];
                                    f->cicles_to_end = cicles_add[add_map[op_code]];
                                }
                                else
                                    printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                break;

                            case MFLO:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rd) && isRegFree(LO_REG);

                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                    alocReg(rd, f);

                                    f->fi = rd;
                                    f->fj = LO_REG;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[LO_REG];
                                    f->cicles_to_end = cicles_add[add_map[op_code]];
                                }
                                else
                                    printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                break;

                            case MTHI:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(HI_REG) && isRegFree(rs);

                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                    alocReg(HI_REG, f);

                                    f->fi = HI_REG;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    f->cicles_to_end = cicles_add[add_map[op_code]];
                                }
                                else
                                    printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                break;

                            case MTLO:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(LO_REG) && isRegFree(rs);

                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding");
                                    alocReg(LO_REG, f);

                                    f->fi = LO_REG;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    f->cicles_to_end = cicles_add[add_map[op_code]];
                                }
                                else
                                    printDebugMessage("No free ADD/LOGIC/MOVE function unit (with dest), stopping");

                                break;

                            case SUB:
                                f = hasFuSub();
                                has_functional_unit = (f != NULL) && isRegFree(rd) && isRegFree(rs) && isRegFree(rt);

                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free SUB function unit (with dest), decoding");
                                    alocReg(rd, f);

                                    f->fi = rd;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    f->dk = registers[rt];
                                    f->cicles_to_end = cicles_sub[sub_map[op_code]];
                                }
                                else
                                    printDebugMessage("No free SUB function unit (with dest), stopping");

                                break;

                            case MULT:
                                f = hasFuMul();
                                has_functional_unit = (f != NULL) && isRegFree(HI_REG) && isRegFree(LO_REG)
                                                                  && isRegFree(rs) && isRegFree(rt);

                                if (has_functional_unit) {
                                    printDebugMessage("Has free MUL function unit (with dest), decoding");
                                    alocReg(HI_REG, f);
                                    alocReg(LO_REG, f);

                                    f->fi = -1;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    f->dk = registers[rt];
                                    f->cicles_to_end = cicles_mul[mul_map[op_code]];
                                }
                                else
                                    printDebugMessage("No free MUL function unit (with dest), stopping");

                                break;

                            case DIV:
                                f = hasFuDiv();
                                has_functional_unit = (f != NULL) && isRegFree(HI_REG) && isRegFree(LO_REG)
                                                                  && isRegFree(rs) && isRegFree(rt);

                                if (has_functional_unit) {
                                    printDebugMessage("Has free DIV function unit (with dest), decoding");
                                    alocReg(HI_REG, f);
                                    alocReg(LO_REG, f);

                                    f->fi = -1;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    f->dk = registers[rt];
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

                            f->busy = 1;
                            f->instruction = inst_data;
                            f->op = op_code;

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rd  = f->fi;
                            inst_data->rs = f->fj;
                            inst_data->rt = f->fk;

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
                        has_functional_unit = (f != NULL) && isRegFree(rs);

                        if (has_functional_unit) {
                            printDebugMessage("Has free ADD/LOGIC/MOVE function unit (without dest), decoding");
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
                            f->dj = registers[rs];
                            f->cicles_to_end = cicles_add[add_map[op_code]];

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rs = rs;
                            inst_data->imm = offset;
                            inst_data->is_branch = 1;

                            rob_data.entry = malloc(sizeof(rob_entry_t));
                            rob_data.entry->instruction = inst_data;

                            rob_data.entry->state = NOT_READY;
                            inst_data->entry = rob_data.entry;

                            pushQueue(&rob_queue, rob_data);
                        }
                        else
                            printDebugMessage("No free ADD/LOGIC/MOVE function unit (without dest), stopping");

                        break;

                    case OP_DECODE_2:
                        op_type = SPECIAL2;
                        op_code = instruction & (unsigned int) 63;
                        rt = (instruction >> (unsigned int)16) & (unsigned int) 31;
                        rs = (instruction >> (unsigned int)21) & (unsigned int) 31;
                        rd = (instruction >> (unsigned int)11) & (unsigned int) 31;
                        immediate = (uint16_t) (instruction & (unsigned int) 65535);

                        f = hasFuMul();
                        has_functional_unit = (f != NULL) && isRegFree(HI_REG) && isRegFree(LO_REG) && isRegFree(rs)
                                                                                                       && isRegFree(rt);

                        if(op_code == MUL) has_functional_unit = has_functional_unit && isRegFree(rd);

                        if (has_functional_unit) {
                            printDebugMessage("Has free MUL function unit (with dest), decoding");
                            alocReg(HI_REG, f);
                            alocReg(LO_REG, f);
                            if(op_code == MUL) alocReg(rd, f);

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
                            f->dj = registers[rs];
                            f->dk = registers[rt];
                            f->cicles_to_end = cicles_mul[mul_map[op_code]];

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rd = f->fi;
                            inst_data->rs = rs;
                            inst_data->rt = rt;

                            rob_data.entry = malloc(sizeof(rob_entry_t));
                            rob_data.entry->instruction = inst_data;

                            rob_data.entry->state = NOT_READY;
                            inst_data->entry = rob_data.entry;

                            pushQueue(&rob_queue, rob_data);
                        }
                        else
                            printDebugMessage("No free MUL function unit (with dest), decoding");

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
                                has_functional_unit = (f != NULL) && isRegFree(rt) && isRegFree(rs);
                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free ADD/LOGIC function unit (with dest), decoding");
                                    alocReg(rt, f);

                                    f->fi = rt;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    f->dk = immediate;
                                }
                                else
                                    printDebugMessage("No free ADD/LOGIC function unit (with dest), stopping");

                                break;

                            case BEQ:
                            case BEQL:
                            case BNE:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rs) && isRegFree(rt);
                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free ADD/LOGIC function unit (without dest), decoding");

                                    f->fi = -1;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    f->dk = registers[rt];
                                    inst_data->is_branch = 1;
                                }
                                else
                                    printDebugMessage("No free ADD/LOGIC function unit (without dest), stopping");

                                break;

                            case BGTZ:
                            case BLEZ:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rs);
                                if (has_functional_unit) {
                                    printDebugMessage(
                                            "Has free ADD/LOGIC function unit (without dest), decoding");

                                    f->fi = -1;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = registers[rs];
                                    inst_data->is_branch = 1;
                                }
                                else
                                    printDebugMessage("No free ADD/LOGIC function unit (without dest), stopping");

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

    // Always run the ALU, if queues are empety and nothing ran, stop running
    running = runAlu() || (!((!instruction_queue.size) && (pc == num_instructions) && (!rob_queue.size)));
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