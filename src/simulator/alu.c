/* Mips32 4K simulator ALU implementation file
   Authors: Cristofer Oswald
   Created: 16/05/2019
   Edited: 22/08/2019 */

#include <stdlib.h>

#include "include/alu.h"
#include "include/simulator.h"
#include "include/util.h"
#include "include/branchComponent.h"

register_data_t reg_status[NUM_REGISTERS]; // Register status for scoreboarding, points to a functional unit

/* Functional units */
functional_unit_t fu_mul[NUM_FU_MUL];
functional_unit_t fu_div[NUM_FU_DIV];
functional_unit_t fu_sub[NUM_FU_SUB];
functional_unit_t fu_add[NUM_FU_ADD];

int mul_map[] = {2, -1, 1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0};
int div_map[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                 -1, 0};
int sub_map[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                 -1, -1, -1, -1, -1, -1, -1, -1, -1, 0};
int add_map[] = {12, 11, -1, -1, 15, 19, 18, 17, 13, -1, 5, 4, 14, 21, 22, 20, 2, 6, 3, 7, 16, -1, -1, -1, -1, -1,
                 -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, 1, 9, 10, 8};

int cicles_mul[] = {C_MULT, C_MUL, C_MADD, C_MSUB};
int cicles_div[] = {C_DIV};
int cicles_sub[] = {C_SUB};
int cicles_add[] = {C_ADD, C_AND, C_MFHI, C_MFLO, C_MOVN, C_MOVZ, C_MTHI, C_MTLO, C_NOR, C_OR, C_XOR, C_BGEZ, C_BLTZ,
                    C_ADD,
                    C_AND, C_BEQ, C_BEQL, C_BGTZ, C_BLEZ, C_BNE, C_LUI, C_OR, C_XOR};

int total_mistakes = 0;
int total_hits = 0;

int add(int rs, int rt) {
    printDebugMessage("Running ADD");

    return rs + rt;
}

int and(int rs, int rt) {
    printDebugMessage("Running AND");

    return ((unsigned int) rs) & ((unsigned int) rt);
}


int beq(int rs, int rt) {
    printDebugMessage("Running BEQ");

    return rs == rt;
}

int beql(int rs, int rt) {
    printDebugMessage("Running BEQL");

    return rs == rt;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

int bgez(int rs, int rt) {
    printDebugMessage("Running BGEZ");

    return (rs > 0) || (rs == 0);
}

int bgtz(int rs, int rt) {
    printDebugMessage("Running BGTZ");

    return rs > 0;
}

int blez(int rs, int rt) {
    printDebugMessage("Running BLEZ");

    return (rs < 0) || (rs == 0);
}

int bltz(int rs, int rt) {
    printDebugMessage("Running BLTZ");

    return rs < 0;
}

#pragma clang diagnostic pop

int bne(int rs, int rt) {
    printDebugMessage("Running BNE");

    return rs != rt;
}

long dv(long rs, long rt) {
    printDebugMessage("Running DIV");

    return ((rs % rt) << 32) | (rs / rt);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

int lui(int rs, int rt) {
    printDebugMessage("Running LUI");

    return rt;
}

#pragma clang diagnostic pop

long madd(long rs, long rt) {
    printDebugMessage("Running MADD");

    return rs * rt;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

int mfhi(int rs, int rt) {
    printDebugMessage("Running MFHI");

    return rs;
}

int mflo(int rs, int rt) {
    printDebugMessage("Running MFLO");

    return rs;
}

int movn(int rs, int rt) {
    printDebugMessage("Running MOVN");

    return rt != 0;
}

int movz(int rs, int rt) {
    printDebugMessage("Running MOVZ");

    return rt == 0;
}

#pragma clang diagnostic pop

long msub(long rs, long rt) {
    printDebugMessage("Running MSUB");

    return rs * rt;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

int mthi(int rs, int rt) {
    printDebugMessage("Running MTHI");

    return rs;
}

int mtlo(int rs, int rt) {
    printDebugMessage("Running MTLO");

    return rs;
}

#pragma clang diagnostic pop

long mul(long rs, long rt) {
    printDebugMessage("Running MUL");

    return rs * rt;
}

long mult(long rs, long rt) {
    printDebugMessage("Running MULT");

    return rs * rt;
}

int nor(int rs, int rt) {
    printDebugMessage("Running NOR");

    return ~(((unsigned int) rs) | ((unsigned int) rt));
}

int or(int rs, int rt) {
    printDebugMessage("Running OR");

    return ((unsigned int) rs) | ((unsigned int) rt);
}

int sub(int rs, int rt) {
    printDebugMessage("Running SUB");

    return rs - rt;
}

int xor(int rs, int rt) {
    printDebugMessage("Running XOR");

    return ((unsigned int) rs) ^ ((unsigned int) rt);
}


long (*inst_fun_mul[])(long, long) = {mult, mul, madd, msub};

long (*inst_fun_div[])(long, long) = {dv};

int (*inst_fun_sub[])(int, int) = {sub};

int (*inst_fun_add[])(int, int) = {add, and, mfhi, mflo, movn, movz, mthi, mtlo, nor, or, xor, bgez, bltz, add,
                                   and, beq, beql, bgtz, blez, bne, lui, or, xor};

void initAlu() {
    int i;

    for (i = 0; i < NUM_REGISTERS; i++) {
        reg_status[i].status = FREE;
        reg_status[i].entry = NULL;
    }

    for (i = 0; i < NUM_FU_MUL; i++) {
        fu_mul[i].busy = 0;
        fu_mul[i].op = -1;
        fu_mul[i].fi = -1;
        fu_mul[i].fj = -1;
        fu_mul[i].fk = -1;
        fu_mul[i].rj = 0;
        fu_mul[i].rk = 0;
        fu_mul[i].cicles_to_end = 0;
    }

    for (i = 0; i < NUM_FU_DIV; i++) {
        fu_div[i].busy = 0;
        fu_div[i].op = -1;
        fu_div[i].fi = -1;
        fu_div[i].fj = -1;
        fu_div[i].fk = -1;
        fu_div[i].rj = 0;
        fu_div[i].rk = 0;
        fu_div[i].cicles_to_end = 0;
    }

    for (i = 0; i < NUM_FU_SUB; i++) {
        fu_sub[i].busy = 0;
        fu_sub[i].op = -1;
        fu_sub[i].fi = -1;
        fu_sub[i].fj = -1;
        fu_sub[i].fk = -1;
        fu_sub[i].rj = 0;
        fu_sub[i].rk = 0;
        fu_sub[i].cicles_to_end = 0;
    }

    for (i = 0; i < NUM_FU_ADD; i++) {
        fu_add[i].busy = 0;
        fu_add[i].op = -1;
        fu_add[i].fi = -1;
        fu_add[i].fj = -1;
        fu_add[i].fk = -1;
        fu_add[i].rj = 0;
        fu_add[i].rk = 0;
        fu_add[i].cicles_to_end = 0;
    }
}

int isRegFree(int r, enum register_access type) {
    if (type == READ) return (reg_status[r].status == FREE) || (reg_status[r].status == BYPASS);
    else return reg_status[r].status == FREE;
}

int readReg(int r) {
    if (reg_status[r].status == FREE) {
        printDebugMessageInt("Normal register read", r);
        return registers[r];
    } else if (reg_status[r].status == BYPASS) {
        printDebugMessageInt("Bypass register read", r);

        if (reg_status[r].entry->instruction->write_flag == IS_HILO) {
            if (r == HI_REG)
                return reg_status[r].entry->hi;
            else
                return reg_status[r].entry->lo;
        }

        return reg_status[r].entry->data;
    } else return 0;
}

void alocReg(int r) {
    reg_status[r].status = USED;
}

void freeReg(int r) {
    reg_status[r].status = FREE;
    reg_status[r].entry = NULL;
}

functional_unit_t *hasFuMul() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_MUL) && (!has)); i++) {
        has = !fu_mul[i].busy;
        f = fu_mul + i;
    }

    if (!has) f = NULL;

    return f;
}

functional_unit_t *hasFuDiv() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_DIV) && (!has)); i++) {
        has = !fu_div[i].busy;
        f = fu_div + i;
    }

    if (!has) f = NULL;

    return f;
}

functional_unit_t *hasFuSub() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_SUB) && (!has)); i++) {
        has = !fu_sub[i].busy;
        f = fu_sub + i;
    }

    if (!has) f = NULL;

    return f;
}

functional_unit_t *hasFuAdd() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_ADD) && (!has)); i++) {
        has = !fu_add[i].busy;
        f = fu_add + i;
    }

    if (!has) f = NULL;

    return f;
}

/**
 * Runs a cicle of a given MUL functional unit
 * @param i The index of the functional unit to be ran
 */
void runMul(int i) {
    queue_data_t allign_data;
    fu_mul[i].cicles_to_end = fu_mul[i].cicles_to_end - 1;

    if (!fu_mul[i].cicles_to_end) {
        fu_mul[i].ri = (*inst_fun_mul[mul_map[fu_mul[i].op]])(fu_mul[i].dj, fu_mul[i].dk);

        allign_data.f = fu_mul + i;

        pushQueue(&allign_queue, allign_data);
    }
}

/**
 * Runs a cicle of a given DIV functional unit
 * @param i The index of the functional unit to be ran
 */
void runDiv(int i) {
    queue_data_t allign_data;
    fu_div[i].cicles_to_end = fu_div[i].cicles_to_end - 1;

    if (!fu_div[i].cicles_to_end) {
        fu_div[i].ri = (*inst_fun_div[div_map[fu_div[i].op]])(fu_div[i].dj, fu_div[i].dk);

        allign_data.f = fu_div + i;

        pushQueue(&allign_queue, allign_data);
    }
}

/**
 * Runs a cicle of a given SUB functional unit
 * @param i The index of the functional unit to be ran
 */
void runSub(int i) {
    fu_sub[i].cicles_to_end = fu_sub[i].cicles_to_end - 1;

    if (!fu_sub[i].cicles_to_end) {
        fu_sub[i].ri = (*inst_fun_sub[sub_map[fu_sub[i].op]])(fu_sub[i].dj, fu_sub[i].dk);

        fu_sub[i].instruction->is_ready = 1;
    }
}

/**
 * Runs a cicle of a given ADD functional unit
 * @param i The index of the functional unit to be ran
 */
void runAdd(int i) {
    fu_add[i].cicles_to_end = fu_add[i].cicles_to_end - 1;

    if (!fu_add[i].cicles_to_end) {
        fu_add[i].ri = (*inst_fun_add[add_map[fu_add[i].op]])(fu_add[i].dj, fu_add[i].dk);


        fu_add[i].instruction->is_ready = 1;
    }
}

int runAlu() {
    int i, ran = 0;

    printDebugMessage("Running ALU");
    printStageHeader("Executando:");

    for (i = 0; i < NUM_FU_MUL; i++) {
        if (!fu_mul[i].busy) continue;

        printInstruction(inst_strs[fu_mul[i].instruction->pc]);

        ran = 1;

        if (fu_mul[i].rj && fu_mul[i].rk)
            runMul(i);
    }

    for (i = 0; i < NUM_FU_DIV; i++) {
        if (!fu_div[i].busy) continue;

        printInstruction(inst_strs[fu_div[i].instruction->pc]);

        ran = 1;

        if (fu_div[i].rj && fu_div[i].rk)
            runDiv(i);
    }

    for (i = 0; i < NUM_FU_SUB; i++) {
        if (!fu_sub[i].busy) continue;

        printInstruction(inst_strs[fu_sub[i].instruction->pc]);

        ran = 1;

        if (fu_sub[i].rj && fu_sub[i].rk)
            runSub(i);
    }

    for (i = 0; i < NUM_FU_ADD; i++) {
        if (!fu_add[i].busy) continue;

        printInstruction(inst_strs[fu_add[i].instruction->pc]);

        ran = 1;

        if (fu_add[i].rj && fu_add[i].rk)
            runAdd(i);
    }

    printNewLine();

    return ran;
}

void write() {
    int i;
    linked_list_t *speculative_inst;

    for (i = 0; i < NUM_FU_MUL; i++) {
        if (!fu_mul[i].busy) continue;

        if (fu_mul[i].instruction->is_ready) {
            printDebugMessage("Writing instruction to ROB (MUL)");

            printInstruction(inst_strs[fu_mul[i].instruction->pc]);

            fu_mul[i].busy = 0;

            if (fu_mul[i].instruction->write_flag == IS_HILO) {
                fu_mul[i].instruction->entry->out_reg = IS_HILO;

                fu_mul[i].instruction->entry->hi = fu_mul[i].hi;
                fu_mul[i].instruction->entry->lo = fu_mul[i].lo;

                if (fu_mul[i].instruction->discard || fu_mul[i].instruction->is_speculate) {
                    reg_status[HI_REG].status = USED;
                    reg_status[LO_REG].status = USED;
                } else {
                    reg_status[HI_REG].status = BYPASS;
                    reg_status[LO_REG].status = BYPASS;

                    reg_status[HI_REG].entry = fu_mul[i].instruction->entry;
                    reg_status[LO_REG].entry = fu_mul[i].instruction->entry;
                }
            } else {
                fu_mul[i].instruction->entry->out_reg = fu_mul[i].fi;
                fu_mul[i].instruction->entry->data = fu_mul[i].ri;

                freeReg(HI_REG);
                freeReg(LO_REG);

                if (fu_mul[i].instruction->discard || fu_mul[i].instruction->is_speculate)
                    reg_status[fu_mul[i].fi].status = USED;
                else
                    reg_status[fu_mul[i].fi].status = BYPASS;

                reg_status[fu_mul[i].fi].entry = fu_mul[i].instruction->entry;
            }

            fu_mul[i].instruction->entry->state = READY;
        }
    }

    for (i = 0; i < NUM_FU_DIV; i++) {
        if (!fu_div[i].busy) continue;

        if (fu_div[i].instruction->is_ready) {
            printDebugMessage("Writing instruction to ROB (DIV)");
            printInstruction(inst_strs[fu_div[i].instruction->pc]);

            fu_div[i].busy = 0;
            fu_div[i].instruction->entry->out_reg = IS_HILO;
            fu_div[i].instruction->entry->hi = fu_div[i].hi;
            fu_div[i].instruction->entry->lo = fu_div[i].lo;

            if (fu_div[i].instruction->discard || fu_div[i].instruction->is_speculate) {
                reg_status[HI_REG].status = USED;
                reg_status[LO_REG].status = USED;
            } else {
                reg_status[HI_REG].status = BYPASS;
                reg_status[LO_REG].status = BYPASS;

                reg_status[HI_REG].entry = fu_div[i].instruction->entry;
                reg_status[LO_REG].entry = fu_div[i].instruction->entry;
            }

            fu_div[i].instruction->entry->state = READY;
        }
    }

    for (i = 0; i < NUM_FU_SUB; i++) {
        if (!fu_sub[i].busy) continue;

        if (fu_sub[i].instruction->is_ready) {
            printDebugMessage("Writing instruction to ROB (SUB)");
            printInstruction(inst_strs[fu_sub[i].instruction->pc]);

            fu_sub[i].busy = 0;
            fu_sub[i].instruction->entry->out_reg = fu_sub[i].fi;
            fu_sub[i].instruction->entry->data = fu_sub[i].ri;
            fu_sub[i].instruction->entry->state = READY;

            if (fu_sub[i].instruction->discard || fu_sub[i].instruction->is_speculate)
                reg_status[fu_sub[i].fi].status = USED;
            else
                reg_status[fu_sub[i].fi].status = BYPASS;

            reg_status[fu_sub[i].fi].entry = fu_sub[i].instruction->entry;
        }
    }

    for (i = 0; i < NUM_FU_ADD; i++) {
        if (!fu_add[i].busy) continue;

        if (fu_add[i].instruction->is_ready) {
            printDebugMessage("Writing instruction to ROB (ADD)");
            printInstruction(inst_strs[fu_add[i].instruction->pc]);

            fu_add[i].busy = 0;

            if (fu_add[i].instruction->write_flag == IS_BRANCH) {
                fu_add[i].instruction->entry->out_reg = IS_BRANCH;
                speculative_inst = fu_add[i].instruction->speculative_insts;

                if (!fu_add[i].instruction->discard) updateBht(fu_add[i].instruction->imm, fu_add[i].ri);

                while (speculative_inst != NULL) {
                    speculative_inst->data->is_speculate = 0;

                    if (fu_add[i].instruction->discard)
                        speculative_inst->data->discard = 1;
                    else
                        speculative_inst->data->discard = fu_add[i].ri != fu_add[i].instruction->taken;

                    if ((!speculative_inst->data->discard) && speculative_inst->data->is_ready) {
                        if (speculative_inst->data->entry) {
                            switch (speculative_inst->data->entry->out_reg) {
                                case IS_BRANCH:
                                    break;
                                case IS_HILO:
                                    reg_status[HI_REG].status = BYPASS;
                                    reg_status[LO_REG].status = BYPASS;
                                    reg_status[HI_REG].entry = speculative_inst->data->entry;
                                    reg_status[LO_REG].entry = speculative_inst->data->entry;
                                default:
                                    reg_status[speculative_inst->data->entry->out_reg].status = BYPASS;
                                    reg_status[speculative_inst->data->entry->out_reg].entry =
                                            speculative_inst->data->entry;
                            }
                        }
                    }

                    speculative_inst = speculative_inst->next;
                }

                if (fu_add[i].ri != fu_add[i].instruction->taken) {
                    if (fu_add[i].ri) {
                        if (!fu_add[i].instruction->discard)
                            pc = fu_add[i].instruction->pc + ((int16_t) fu_add[i].instruction->imm);
                    } else {
                        if (!fu_add[i].instruction->discard)
                            pc = fu_add[i].instruction->pc + 1;
                    }
                    total_mistakes++;

                    current_branch_inst = NULL;
                } else {
                    if (current_branch_inst == fu_add[i].instruction)
                        current_branch_inst = NULL;
                    total_hits++;
                }

                clearList(fu_add[i].instruction->speculative_insts);
            } else if (fu_add[i].instruction->write_flag == IS_MOVE) {
                fu_add[i].instruction->entry->out_reg = fu_add[i].fi;

                if (fu_add[i].ri) fu_add[i].instruction->entry->data = fu_add[i].dj;
                else fu_add[i].instruction->entry->data = registers[fu_add[i].fi];

                if (fu_add[i].instruction->discard || fu_add[i].instruction->is_speculate)
                    reg_status[fu_add[i].fi].status = USED;
                else
                    reg_status[fu_add[i].fi].status = BYPASS;

                reg_status[fu_add[i].fi].entry = fu_add[i].instruction->entry;

            } else {
                fu_add[i].instruction->entry->out_reg = fu_add[i].fi;
                fu_add[i].instruction->entry->data = fu_add[i].ri;

                if (fu_add[i].instruction->discard || fu_add[i].instruction->is_speculate)
                    reg_status[fu_add[i].fi].status = USED;
                else
                    reg_status[fu_add[i].fi].status = BYPASS;

                reg_status[fu_add[i].fi].entry = fu_add[i].instruction->entry;
            }

            fu_add[i].instruction->entry->state = READY;

        }
    }
}

void printBypass() {
    int i;

    for (i = 0; i < NUM_REGISTERS; ++i) {
        if (reg_status[i].status == BYPASS) {
            printRegisterName(i);
        }
    }
}