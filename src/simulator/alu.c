/* Mips32 4K simulator ALU implementation file
   Authors: Cristofer Oswald
   Created: 16/05/2019
   Edited: 27/05/2019 */

#include <stdlib.h>

#include "include/alu.h"
#include "include/simulator.h"
#include "include/util.h"

functional_unit_t *reg_status[NUM_REGISTERS]; // Register status for scoreboarding, points to a functional unit

/* Functional units */
functional_unit_t fu_mul[NUM_FU_MUL];
functional_unit_t fu_div[NUM_FU_DIV];
functional_unit_t fu_sub[NUM_FU_SUB];
functional_unit_t fu_add[NUM_FU_ADD];

/*
 * These maps are need to map the instrucion code to the instrucion function array
 */
int mul_map[] = {2, -1, 1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0};
int div_map[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                 -1, 0};
int sub_map[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                 -1, -1, -1, -1, -1, -1, -1, -1, -1, 0};
int add_map[] = {12, 11, -1, -1, 15, 19, 18, 17, 13, -1, 5, 4, 14, 21, 22, 20, 2, 6, 3, 7, 16, -1, -1, -1, -1, -1,
                 -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, 1, 9, 10, 8};

void add(int rd, int rs, int rt){
    printDebugMessage("Running ADD");

    registers[rd] = registers[rs] + registers[rt];
}

void addi(int rd, int rs, int rt){}

void and(int rd, int rs, int rt){
    printDebugMessage("Running AND");

    registers[rd] = registers[rs] & registers[rt];
}

void andi(int rd, int rs, int rt){}

void beq(int rd, int rs, int rt){}

void beql(int rd, int rs, int rt){}

void bgez(int rd, int rs, int rt){}

void bgtz(int rd, int rs, int rt){}

void blez(int rd, int rs, int rt){}

void bltz(int rd, int rs, int rt){}

void bne(int rd, int rs, int rt){}

void dv(int rd, int rs, int rt){
    printDebugMessage("Running DIV");

    registers[LO_REG] = registers[rs] / registers[rt];
    registers[HI_REG] = registers[rs] % registers[rt];
}

void lui(int rd, int rs, int rt){
    printDebugMessage("Running LUI");
}

void madd(int rd, int rs, int rt){}

void mfhi(int rd, int rs, int rt){
    printDebugMessage("Running MFHI");

    registers[rd] = registers[HI_REG];
}

void mflo(int rd, int rs, int rt){
    printDebugMessage("Running MFLO");

    registers[rd] = registers[LO_REG];
}

void movn(int rd, int rs, int rt){
    printDebugMessage("Running MOVN");

    if(registers[rt]) registers[rd] = registers[rs];
}

void movz(int rd, int rs, int rt){
    printDebugMessage("Running MOVZ");

    if(!registers[rt]) registers[rd] = registers[rs];
}

void msub(int rd, int rs, int rt){}

void mthi(int rd, int rs, int rt){
    printDebugMessage("Running MTHI");

    registers[HI_REG] = registers[rs];
}

void mtlo(int rd, int rs, int rt){
    printDebugMessage("Running MTLO");

    registers[LO_REG] = registers[rs];
}

void mul(int rd, int rs, int rt){}

void mult(int rd, int rs, int rt){}

void nop(int rd, int rs, int rt){}

void nor(int rd, int rs, int rt){
    printDebugMessage("Running NOR");

    registers[rd] = ~(registers[rs] | registers[rt]);
}

void or(int rd, int rs, int rt){
    printDebugMessage("Running OR");

    registers[rd] = registers[rs] | registers[rt];
}

void ori(int rd, int rs, int rt){}

void sub(int rd, int rs, int rt){
    printDebugMessage("Running SUB");

    registers[rd] = registers[rs] - registers[rt];
}

void scall(int rd, int rs, int rt){}

void xor(int rd, int rs, int rt){
    printDebugMessage("Running XOR");

    registers[rd] = registers[rs] ^ registers[rt];
}

void xori(int rd, int rs, int rt){}

void (*inst_fun_mul[])(int, int, int) = {mult, mul, madd, msub};
void (*inst_fun_div[])(int, int, int) = {dv};
void (*inst_fun_sub[])(int, int, int) = {sub};
void (*inst_fun_add[])(int, int, int) = {add, and, mfhi, mflo, movn, movz, mthi, mtlo, nor, or, xor, bgez, bltz, addi,
                            andi, beq, beql, bgtz, blez, bne, lui, ori, xori};

/* Number of cicles needed for each instruction */
int cicles_mul[] = {4, 4, 4, 4};
int cicles_div[] = {4};
int cicles_sub[] = {2};
int cicles_add[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

void initAlu(){
    int i;

    for(i = 0; i < NUM_REGISTERS; i++){
        reg_status[i] = NULL;
    }

    for(i = 0; i < NUM_FU_MUL; i++){
        fu_mul[i].busy = 0;
        fu_mul[i].op = -1;
        fu_mul[i].fi = -1;
        fu_mul[i].fj = -1;
        fu_mul[i].fk = -1;
        fu_mul[i].qj = NULL;
        fu_mul[i].qk = NULL;
        fu_mul[i].rj = 0;
        fu_mul[i].rk = 0;
        fu_mul[i].cicles_to_end = 0;
    }

    for(i = 0; i < NUM_FU_DIV; i++){
        fu_div[i].busy = 0;
        fu_div[i].op = -1;
        fu_div[i].fi = -1;
        fu_div[i].fj = -1;
        fu_div[i].fk = -1;
        fu_div[i].qj = NULL;
        fu_div[i].qk = NULL;
        fu_div[i].rj = 0;
        fu_div[i].rk = 0;
        fu_div[i].cicles_to_end = 0;
    }

    for(i = 0; i < NUM_FU_SUB; i++){
        fu_sub[i].busy = 0;
        fu_sub[i].op = -1;
        fu_sub[i].fi = -1;
        fu_sub[i].fj = -1;
        fu_sub[i].fk = -1;
        fu_sub[i].qj = NULL;
        fu_sub[i].qk = NULL;
        fu_sub[i].rj = 0;
        fu_sub[i].rk = 0;
        fu_sub[i].cicles_to_end = 0;
    }

    for(i = 0; i < NUM_FU_ADD; i++){
        fu_add[i].busy = 0;
        fu_add[i].op = -1;
        fu_add[i].fi = -1;
        fu_add[i].fj = -1;
        fu_add[i].fk = -1;
        fu_add[i].qj = NULL;
        fu_add[i].qk = NULL;
        fu_add[i].rj = 0;
        fu_add[i].rk = 0;
        fu_add[i].cicles_to_end = 0;
    }
}

int isRegFree(int r){
    return reg_status[r] == NULL;
}

int hasFuMul() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_MUL) && (!has)); i++) {
        has = !fu_mul[i].busy;
        f = fu_mul+i;
    }

    if(has) f->busy = 1;

    return has;
}

int hasFuDiv() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_DIV) && (!has)); i++) {
        has = !fu_div[i].busy;
        f = fu_div+i;
    }

    if(has) f->busy = 1;

    return has;
}

int hasFuSub() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_SUB) && (!has)); i++) {
        has = !fu_sub[i].busy;
        f = fu_sub+i;
    }

    if(has) f->busy = 1;

    return has;
}

int hasFuAdd() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_ADD) && (!has)); i++) {
        has = !fu_add[i].busy;
        f = fu_add+i;
    }

    if(has) f->busy = 1;

    return has;
}

void allocFuMul(instruction_data_t *instruction){
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_MUL) && (!has)); i++) {
        has = fu_mul[i].busy && (fu_mul[i].op == -1);
        f = fu_mul+i;
    }

    if(has){
        f->instruction = instruction;
        f->op = instruction->op_code;
        f->fi = instruction->rd;
        f->fj = instruction->rs;
        f->fk = instruction->rt;
        f->qj = reg_status[f->fj];
        f->qk = reg_status[f->fk];
        f->rj = isRegFree(f->fj);
        f->rk = isRegFree(f->fk);
    }
    else {
        error();
        printDebugError("ALU stage", "No free MUL FU to allocate");
    }
}

void allocFuDiv(instruction_data_t *instruction){
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_DIV) && (!has)); i++) {
        has = fu_div[i].busy && (fu_div[i].op == -1);
        f = fu_div+i;
    }

    if(has){
        f->instruction = instruction;
        f->op = instruction->op_code;
        f->fi = instruction->rd;
        f->fj = instruction->rs;
        f->fk = instruction->rt;
        f->qj = reg_status[f->fj];
        f->qk = reg_status[f->fk];
        f->rj = isRegFree(f->fj);
        f->rk = isRegFree(f->fk);
    }
    else {
        error();
        printDebugError("ALU stage", "No free DIV FU to allocate");
    }
}

void allocFuSub(instruction_data_t *instruction){
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_SUB) && (!has)); i++) {
        has = fu_sub[i].busy && (fu_sub[i].op == -1);
        f = fu_sub+i;
    }

    if(has){
        f->instruction = instruction;
        f->op = instruction->op_code;
        f->fi = instruction->rd;
        f->fj = instruction->rs;
        f->fk = instruction->rt;
        f->qj = reg_status[f->fj];
        f->qk = reg_status[f->fk];
        f->rj = isRegFree(f->fj);
        f->rk = isRegFree(f->fk);
    }
    else {
        error();
        printDebugError("ALU stage", "No free SUB FU to allocate");
    }
}

void allocFuAdd(instruction_data_t *instruction){
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_ADD) && (!has)); i++) {
        has = fu_add[i].busy && (fu_add[i].op == -1);
        f = fu_add+i;
    }

    if(has){
        f->instruction = instruction;
        f->op = instruction->op_code;
        f->fi = instruction->rd;
        f->fj = instruction->rs;
        f->fk = instruction->rt;
        f->qj = reg_status[f->fj];
        f->qk = reg_status[f->fk];
        f->rj = isRegFree(f->fj);
        f->rk = isRegFree(f->fk);
    }
    else {
        error();
        printDebugError("ALU stage", "No free ADD FU to allocate");
    }
}

/**
 * Runs a cicle of a given MUL functional unit
 * @param i The index of the functional unit to be ran
 */
void runMul(int i){
    if(fu_mul[i].rj && fu_mul[i].rk)
        (*inst_fun_mul[mul_map[fu_mul[i].op]])(fu_mul[i].fi, fu_mul[i].fj, fu_mul[i].fk);
}

/**
 * Runs a cicle of a given DIV functional unit
 * @param i The index of the functional unit to be ran
 */
void runDiv(int i){
    if(fu_div[i].rj && fu_div[i].rk)
        (*inst_fun_div[div_map[fu_div[i].op]])(fu_div[i].fi, fu_div[i].fj, fu_div[i].fk);

}

/**
 * Runs a cicle of a given SUB functional unit
 * @param i The index of the functional unit to be ran
 */
void runSub(int i){
    if(fu_sub[i].rj && fu_sub[i].rk)
        (*inst_fun_sub[sub_map[fu_sub[i].op]])(fu_sub[i].fi, fu_sub[i].fj, fu_sub[i].fk);

}

/**
 * Runs a cicle of a given ADD functional unit
 * @param i The index of the functional unit to be ran
 */
void runAdd(int i){
    if(fu_add[i].rj && fu_add[i].rk)
        (*inst_fun_add[add_map[fu_add[i].op]])(fu_add[i].fi, fu_add[i].fj, fu_add[i].fk);
}

void runAlu(){
    int i;

    for(i = 0; i < NUM_FU_MUL; i++) {
        if(!fu_mul[i].busy) continue;

        runMul(i);

        /* Temporary */
        fu_mul[i].busy = 0;
        fu_mul[i].op = -1;
        free(fu_mul[i].instruction);
        /* Temporary */
    }

    for(i = 0; i < NUM_FU_DIV; i++) {
        if(!fu_div[i].busy) continue;

        runDiv(i);

        /* Temporary */
        fu_div[i].busy = 0;
        fu_div[i].op = -1;
        free(fu_div[i].instruction);
        /* Temporary */
    }

    for(i = 0; i < NUM_FU_SUB; i++) {
        if(!fu_sub[i].busy) continue;

        runSub(i);

        /* Temporary */
        fu_sub[i].busy = 0;
        fu_sub[i].op = -1;
        free(fu_sub[i].instruction);
        /* Temporary */
    }

    for(i = 0; i < NUM_FU_ADD; i++) {
        if(!fu_add[i].busy) continue;

        runAdd(i);
        /* Temporary */
        fu_add[i].busy = 0;
        fu_add[i].op = -1;
        free(fu_add[i].instruction);
        /* Temporary */
    }
}