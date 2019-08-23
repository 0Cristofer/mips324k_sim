/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 22/08/2019 */

#include "include/simulator.h"
#include "include/branchComponent.h"
#include "include/util.h"
#include "include/alu.h"

#include <stdlib.h>
#include <stdint.h>

int debug;
int has_error = 0;
int cycle = 0;
int is_detail;
int total_emited = 0;
int total_effected = 0;

unsigned int pc;
queue_t instruction_queue;
queue_t rob_queue;
queue_t allign_queue;
char **inst_strs;

void startSimulation(unsigned int *insts, unsigned int num_insts, int b, char **insts_strs, int d, FILE *out_file) {
    debug = b;
    instructions = insts;
    running = 1;
    num_instructions = num_insts;
    pc = 0;
    inst_strs = insts_strs;
    is_detail = d;

    setOutFile(out_file);

    initQueue(&instruction_queue);
    initQueue(&rob_queue);
    initQueue(&allign_queue);
    initAlu();
    initBranchPredictor();

    registers[ZERO] = 0;
    registers[T0] = 0;

    resetAll();
    clock();

    printDebugMessage("\tWriting simulation ouput.\n");

    printAll();
    percentageInstruction(total_emited, total_effected);
    printRegistersContent();

    cleanup();
}

void clock() {
    while (running && (!has_error)) {
        printDebugMessageInt("************** Cycle ************", cycle);

        pipeline();

        cycle++;
    }

    percentagePrediction(total_jumps, total_mistakes, total_hits);
    printCycles(cycle);
}

void pipeline() {
    effect();
    nextPrintStage();
    writeback();
    nextPrintStage();
    alignAccumulate();
    nextPrintStage();
    memory();
    nextPrintStage();
    execution();
    nextPrintStage();
    instruction();
    nextPrintStage();
    printBypassing();
    nextPrintStage();
    printCurrentCycle(cycle);
    printAllStages();
    resetAll();
}

void instruction() {
    int next_pc;
    queue_data_t data;

    if (has_error) return;

    printDebugMessage("---Instruction stage---");
    printStageHeader("Busca:");

    if (instruction_queue.size) { // If there is elements in the instruction queue, do nothing
        printDebugMessage("Instruction queue has elements, skipping fetch");
        printNewLine();
        return;
    }

    if (pc == num_instructions) {
        printDebugMessage("PC at the end of program, skipping fetch");
        printNewLine();
        return;
    }

    printDebugMessage("Fetching instructions");

    while (instruction_queue.size < MAX_INST_QUEUE_SIZE) {
        if (pc == num_instructions) {
            printDebugMessage("Out of instructions. Stopping fetch");
            printNewLine();
            return;
        }

        data.instruction = malloc(sizeof(instruction_data_t));

        data.instruction->instruction = instructions[pc];
        data.instruction->speculative_insts = NULL;
        data.instruction->is_speculate = 0;
        data.instruction->is_ready = 0;
        data.instruction->pc = pc;
        data.instruction->discard = 0;
        data.instruction->entry = NULL;

        printDebugMessageInt("\tFetched instruction", pc);

        printInstruction(inst_strs[data.instruction->pc]);
        pushQueue(&instruction_queue, data);

        next_pc = branchComponent(pc);

        updatePc(next_pc); // The PC increment will be given from the branch component
    }

    printNewLine();
}

void execution() {
    queue_data_t rob_data;

    unsigned int instruction, op_type, op_code, rd = 0, rt = 0, rs = 0, offset = 0;
    int issued = 0, has_functional_unit = 1;
    uint16_t immediate = 0;
    instruction_data_t *inst_data;
    functional_unit_t *f = NULL;

    if (has_error) return;

    printDebugMessage("---Execution stage---");

    printStageHeader("Emição:");

    if (!instruction_queue.size) {
        printDebugMessage("Instruction queue is empety, skipping");
    } else {
        while (has_functional_unit && (rob_queue.size < MAX_ROB_QUEUE_SIZE) && (issued < MAX_ISSUED_INST)) {
            if (!instruction_queue.size) {
                printDebugMessage("Out of instructions. Stopping decode");
                break;
            } else {
                inst_data = instruction_queue.head->data.instruction;
                instruction = inst_data->instruction;

                // Decodification
                switch (instruction >> (unsigned int) 26) {
                    case OP_DECODE_0:
                        op_type = SPECIAL;
                        op_code = instruction & (unsigned int) 63;
                        rd = (instruction >> (unsigned int) 11) & (unsigned int) 31;
                        rt = (instruction >> (unsigned int) 16) & (unsigned int) 31;
                        rs = (instruction >> (unsigned int) 21) & (unsigned int) 31;

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
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding",
                                            inst_data->pc);

                                    f->fi = rd;
                                    f->fj = rs;
                                    f->fk = rt;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->dk = readReg(rt);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(rd);
                                } else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping",
                                                         inst_data->pc);

                                break;

                            case MFHI:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rd, WRITE) && isRegFree(HI_REG, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding",
                                            inst_data->pc);
                                    f->fi = rd;
                                    f->fj = HI_REG;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(HI_REG);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(rd);
                                } else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping",
                                                         inst_data->pc);

                                break;

                            case MFLO:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(rd, WRITE) && isRegFree(LO_REG, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding",
                                            inst_data->pc);
                                    f->fi = rd;
                                    f->fj = LO_REG;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(LO_REG);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(rd);
                                } else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping",
                                                         inst_data->pc);

                                break;

                            case MTHI:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(HI_REG, WRITE) && isRegFree(rs, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding",
                                            inst_data->pc);
                                    f->fi = HI_REG;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(HI_REG);
                                } else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping",
                                                         inst_data->pc);

                                break;

                            case MTLO:
                                f = hasFuAdd();
                                has_functional_unit = (f != NULL) && isRegFree(LO_REG, WRITE) && isRegFree(rs, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt(
                                            "Has free ADD/LOGIC/MOVE function unit (with dest), decoding",
                                            inst_data->pc);
                                    f->fi = LO_REG;
                                    f->fj = rs;
                                    f->fk = -1;
                                    f->rj = 1;
                                    f->rk = 1;
                                    f->dj = readReg(rs);
                                    f->cicles_to_end = cicles_add[add_map[op_code]];

                                    alocReg(LO_REG);
                                } else
                                    printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (with dest), stopping",
                                                         inst_data->pc);

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
                                } else
                                    printDebugMessageInt("No free SUB function unit (with dest), stopping",
                                                         inst_data->pc);

                                break;

                            case MULT:
                                f = hasFuMul();
                                has_functional_unit =
                                        (f != NULL) && isRegFree(HI_REG, WRITE) && isRegFree(LO_REG, WRITE)
                                        && isRegFree(rs, READ) && isRegFree(rt, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt("Has free MUL function unit (with dest), decoding",
                                                         inst_data->pc);
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
                                } else
                                    printDebugMessageInt("No free MUL function unit (with dest), stopping",
                                                         inst_data->pc);

                                break;

                            case DIV:
                                f = hasFuDiv();
                                has_functional_unit =
                                        (f != NULL) && isRegFree(HI_REG, WRITE) && isRegFree(LO_REG, WRITE)
                                        && isRegFree(rs, READ) && isRegFree(rt, READ);

                                if (has_functional_unit) {
                                    printDebugMessageInt("Has free DIV function unit (with dest), decoding",
                                                         inst_data->pc);
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
                                } else
                                    printDebugMessageInt("No free DIV function unit (with dest), stopping",
                                                         inst_data->pc);

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
                            printInstruction(inst_strs[inst_data->pc]);

                            total_emited++;
                            issued = issued + 1;

                            f->busy = 1;
                            f->instruction = inst_data;
                            f->op = op_code;

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rd = f->fi;
                            inst_data->rs = f->fj;
                            inst_data->rt = f->fk;

                            if ((op_code == MOVN) || (op_code == MOVZ))
                                inst_data->write_flag = IS_MOVE;
                            else if ((op_code == DIV) || (op_code == MULT))
                                inst_data->write_flag = IS_HILO;
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
                        op_code = ((instruction >> (unsigned int) 16) & (unsigned int) 31);
                        rs = (instruction >> (unsigned int) 21) & (unsigned int) 31;

                        f = hasFuAdd();
                        has_functional_unit = (f != NULL) && isRegFree(rs, READ);

                        if (has_functional_unit) {
                            printDebugMessageInt("Has free ADD/LOGIC/MOVE function unit (without dest), decoding",
                                                 inst_data->pc);
                            inst_data->f = f;

                            instruction = popQueue(&instruction_queue).instruction->instruction;
                            printInstruction(inst_strs[inst_data->pc]);
                            total_emited++;
                            issued = issued + 1;

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
                        } else
                            printDebugMessageInt("No free ADD/LOGIC/MOVE function unit (without dest), stopping",
                                                 inst_data->pc);

                        break;

                    case OP_DECODE_2:
                        op_type = SPECIAL2;
                        op_code = instruction & (unsigned int) 63;
                        rt = (instruction >> (unsigned int) 16) & (unsigned int) 31;
                        rs = (instruction >> (unsigned int) 21) & (unsigned int) 31;
                        rd = (instruction >> (unsigned int) 11) & (unsigned int) 31;

                        f = hasFuMul();
                        has_functional_unit = (f != NULL) && isRegFree(HI_REG, WRITE) && isRegFree(LO_REG, WRITE) &&
                                              isRegFree(rs, READ) && isRegFree(rt, READ);

                        if (op_code == MUL) has_functional_unit = has_functional_unit && isRegFree(rd, WRITE);

                        if (has_functional_unit) {
                            printDebugMessageInt("Has free MUL function unit (with dest), decoding", inst_data->pc);
                            inst_data->f = f;

                            instruction = popQueue(&instruction_queue).instruction->instruction;
                            printInstruction(inst_strs[inst_data->pc]);
                            total_emited++;
                            issued = issued + 1;

                            f->busy = 1;
                            f->instruction = inst_data;
                            f->op = op_code;
                            if (op_code == MUL) f->fi = rd;
                            else f->fi = -1;
                            f->fj = rs;
                            f->fk = rt;
                            f->rj = 1;
                            f->rk = 1;
                            if (op_code != MUL) {
                                f->hi = readReg(HI_REG);
                                f->lo = readReg(LO_REG);
                            }
                            f->dj = readReg(rs);
                            f->dk = readReg(rt);
                            f->cicles_to_end = cicles_mul[mul_map[op_code]];

                            alocReg(HI_REG);
                            alocReg(LO_REG);
                            if (op_code == MUL) alocReg(rd);

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rd = f->fi;
                            inst_data->rs = rs;
                            inst_data->rt = rt;
                            if (op_code != MUL)
                                inst_data->write_flag = IS_HILO;
                            else
                                inst_data->write_flag = IS_MUL;

                            rob_data.entry = malloc(sizeof(rob_entry_t));
                            rob_data.entry->instruction = inst_data;

                            rob_data.entry->state = NOT_READY;
                            inst_data->entry = rob_data.entry;

                            pushQueue(&rob_queue, rob_data);
                        } else
                            printDebugMessageInt("No free MUL function unit (with dest), decoding", inst_data->pc);

                        break;

                    default:
                        op_type = NONE;
                        op_code = instruction >> (unsigned int) 26;
                        rs = (instruction >> (unsigned int) 21) & (unsigned int) 31;
                        rt = (instruction >> (unsigned int) 16) & (unsigned int) 31;
                        immediate = (uint16_t) (instruction & (unsigned int) 65535);

                        switch (op_code) {
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
                                } else
                                    printDebugMessageInt("No free ADD/LOGIC function unit (with dest), stopping",
                                                         inst_data->pc);

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
                                } else
                                    printDebugMessageInt("No free ADD/LOGIC function unit (without dest), stopping",
                                                         inst_data->pc);

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
                                } else
                                    printDebugMessageInt("No free ADD/LOGIC function unit (without dest), stopping",
                                                         inst_data->pc);

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
                            printInstruction(inst_strs[inst_data->pc]);
                            total_emited++;
                            issued = issued + 1;

                            f->busy = 1;
                            f->instruction = inst_data;
                            f->op = op_code;

                            inst_data->instruction = instruction;
                            inst_data->op_type = op_type;
                            inst_data->op_code = op_code;
                            inst_data->rd = f->fi;
                            inst_data->rs = f->fj;
                            inst_data->rt = f->fk;
                            inst_data->imm = immediate;

                            if ((op_code == BEQ) || (op_code == BEQL) || (op_code == BNE) || (op_code == BGTZ)
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

    printNewLine();

    // Always run the ALU, if queues are empety and nothing ran, stop running
    running = runAlu() || (!((!instruction_queue.size) && (pc == num_instructions) && (!rob_queue.size)));
}

void memory() {
    if (has_error) return;
    printStageHeader("Busca da memória:");
    printDebugMessage("---Memory Stage---");
    printNewLine();
}

void alignAccumulate() {
    queue_data_t data;
    long hilo;

    if (has_error) return;

    printDebugMessage("---Allign/Accumulate Stage---");
    printStageHeader("Alinhamento:");

    while (allign_queue.size) {
        data = popQueue(&allign_queue);
        printInstruction(inst_strs[data.f->instruction->pc]);

        switch (data.f->op) {
            case MULT:
            case DIV:
                printDebugMessage("Alligning MULT/DIV");
                data.f->hi = data.f->ri >> 32;
                data.f->lo = (data.f->ri << 32) >> 32;
                break;
            case MUL:
                printDebugMessage("Alligning MUL");
                data.f->ri = (data.f->ri << 32) >> 32;
                break;
            case MADD:
                printDebugMessage("Alligning MADD");
                hilo = (((long) data.f->hi) << 32) | data.f->lo;
                data.f->ri = hilo + data.f->ri;
                data.f->hi = data.f->ri >> 32;
                data.f->lo = (data.f->ri << 32) >> 32;
                break;
            case MSUB:
                printDebugMessage("Alligning MSUB");
                hilo = (((long) data.f->hi) << 32) | data.f->lo;
                data.f->ri = hilo - data.f->ri;
                data.f->hi = data.f->ri >> 32;
                data.f->lo = (data.f->ri << 32) >> 32;
                break;
        }

        data.f->instruction->is_ready = 1;
    }

    printNewLine();
}

void writeback() {
    if (has_error) return;

    printDebugMessage("---Writeback Stage---");
    printStageHeader("Escrita:");

    write();

    printNewLine();
}

void effect() {
    int effected = 0;
    queue_data_t data;

    if (has_error) return;
    printDebugMessage("---Effect Stage---");
    printStageHeader("Efetivando:");

    while (rob_queue.size && (effected < MAX_EFFECT_INST)) {
        data = rob_queue.head->data;

        if (data.entry->state == READY) {
            if (!data.entry->instruction->is_speculate) {
                data = popQueue(&rob_queue);

                if (!data.entry->instruction->discard) {
                    total_effected++;
                    printInstruction(inst_strs[data.entry->instruction->pc]);
                    switch (data.entry->out_reg) {
                        case IS_BRANCH:
                            break;
                        case IS_HILO:
                            registers[HI_REG] = data.entry->hi;
                            registers[LO_REG] = data.entry->lo;
                            break;
                        default:
                            registers[data.entry->out_reg] = data.entry->data;
                            break;
                    }

                    printDebugMessageInt("Commited instruction", data.entry->instruction->pc);
                } else
                    printDebugMessageInt("Discarded instruction", data.entry->instruction->pc);

                switch (data.entry->out_reg) {
                    case IS_BRANCH:
                        break;
                    case IS_HILO:
                        freeReg(HI_REG);
                        freeReg(LO_REG);
                        break;
                    default:
                        freeReg(data.entry->out_reg);
                        break;
                }

                free(data.entry->instruction);
                free(data.entry);
            } else {
                printDebugMessageInt("Instruction is speculative", data.entry->instruction->pc);
                break;
            }

            effected = effected + 1;
        } else {
            printDebugMessageInt("Instruction is not ready", data.entry->instruction->pc);
            break;
        }
    }

    printNewLine();
}

void printBypassing() {
    printStageHeader("Bypassing:");
    printBypass();
    printNewLine();
}

void updatePc(int next_pc) {
    pc = pc + next_pc;
}

void cleanup() {
    clearQueue(&instruction_queue);
    clearQueue(&rob_queue);
    clearQueue(&allign_queue);
}

void error() {
    has_error = 1;
    running = 0;
    printDebugMessage("An error happend, stopping.");
}