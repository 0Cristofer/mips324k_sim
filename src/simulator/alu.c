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

int mul_map[] = {2, -1, 1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0};
int div_map[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                 -1, 0};
int sub_map[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                 -1, -1, -1, -1, -1, -1, -1, -1, -1, 0};
int add_map[] = {12, 11, -1, -1, 15, 19, 18, 17, 13, -1, 5, 4, 14, 21, 22, 20, 2, 6, 3, 7, 16, -1, -1, -1, -1, -1,
                 -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, 1, 9, 10, 8};

int cicles_mul[] = {4, 4, 4, 4};
int cicles_div[] = {4};
int cicles_sub[] = {2};
int cicles_add[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3,
                    2, 2, 2, 2, 2, 2, 2, 2, 2};

int add(int rs, int rt){
    printDebugMessage("Running ADD");

    return rs + rt;
}

int and(int rs, int rt){
    printDebugMessage("Running AND");

    return rs & rt;
}


int beq(int rs, int rt){
    printDebugMessage("Running BEQ");

    return rs == rt;
}

int beql(int rs, int rt){
    printDebugMessage("Running BEQL");

    return rs == rt;
}

int bgez(int rs, int rt){
    printDebugMessage("Running BGEZ");

    return (rs > 0) || (rs == 0);
}

int bgtz(int rs, int rt){
    printDebugMessage("Running BGTZ");

    return rs > 0;
}

int blez(int rs, int rt){
    printDebugMessage("Running BLEZ");

    return (rs < 0) || (rs == 0);
}

int bltz(int rs, int rt){
    printDebugMessage("Running BLTZ");

    return rs < 0;
}

int bne(int rs, int rt){
    printDebugMessage("Running BNE");

    return rs != rt;
}

/* TODO */
int dv(int rs, int rt){
    printDebugMessage("Running DIV");

    registers[LO_REG] = rs / rt;
    registers[HI_REG] = rs % rt;

    return rs / rt;
}

int lui(int rs, int rt){
    printDebugMessage("Running LUI");

    return rs;
}

/* TODO */
int madd(int rs, int rt){
    printDebugMessage("Running MADD");

    return rs;
}

/* TODO */
int mfhi(int rs, int rt){
    printDebugMessage("Running MFHI");

    return rs;
}

/* TODO */
int mflo(int rs, int rt){
    printDebugMessage("Running MFLO");

    return rs;
}

/* TODO */
int movn(int rs, int rt){
    printDebugMessage("Running MOVN");

    return rs;
}

/* TODO */
int movz(int rs, int rt){
    printDebugMessage("Running MOVZ");

    return rs;
}

/* TODO */
int msub(int rs, int rt){
    printDebugMessage("Running MSUB");

    return rs;
}

/* TODO */
int mthi(int rs, int rt){
    printDebugMessage("Running MTHI");

    return rs;
}

/* TODO */
int mtlo(int rs, int rt){
    printDebugMessage("Running MTLO");

    return rs;
}

int mul(int rs, int rt){
    printDebugMessage("Running MUL");

    return rs * rt;
}

/* TODO */
int mult(int rs, int rt){
    printDebugMessage("Running MULT");

    return rs;
}

int nor(int rs, int rt){
    printDebugMessage("Running NOR");

    return ~(rs | rt);
}

int or(int rs, int rt){
    printDebugMessage("Running OR");

    return rs | rt;
}

int sub(int rs, int rt){
    printDebugMessage("Running SUB");

    return rs - rt;
}

int xor(int rs, int rt){
    printDebugMessage("Running XOR");

    return rs ^ rt;
}


int (*inst_fun_mul[])(int, int) = {mult, mul, madd, msub};
int (*inst_fun_div[])(int, int) = {dv};
int (*inst_fun_sub[])(int, int) = {sub};
int (*inst_fun_add[])(int, int) = {add, and, mfhi, mflo, movn, movz, mthi, mtlo, nor, or, xor, bgez, bltz, add,
                            and, beq, beql, bgtz, blez, bne, lui, or, xor};

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

void alocReg(int r, functional_unit_t* f){
    reg_status[r] = f;
}

void freeReg(int r){
    reg_status[r] = NULL;
}

functional_unit_t *hasFuMul() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_MUL) && (!has)); i++) {
        has = !fu_mul[i].busy;
        f = fu_mul+i;
    }

    if(!has) f = NULL;

    return f;
}

functional_unit_t *hasFuDiv() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_DIV) && (!has)); i++) {
        has = !fu_div[i].busy;
        f = fu_div+i;
    }

    if(!has) f = NULL;

    return f;
}

functional_unit_t *hasFuSub() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_SUB) && (!has)); i++) {
        has = !fu_sub[i].busy;
        f = fu_sub+i;
    }

    if(!has) f = NULL;

    return f;
}

functional_unit_t *hasFuAdd() {
    int i, has = 0;
    functional_unit_t *f = NULL;

    for (i = 0; ((i < NUM_FU_ADD) && (!has)); i++) {
        has = !fu_add[i].busy;
        f = fu_add+i;
    }

    if(!has) f = NULL;

    return f;
}

/**
 * Runs a cicle of a given MUL functional unit
 * @param i The index of the functional unit to be ran
 */
void runMul(int i){
    if(fu_mul[i].cicles_to_end == 1){
        (*inst_fun_mul[mul_map[fu_mul[i].op]])(fu_mul[i].dj, fu_mul[i].dk);

        // Depois de executar, escrever no ROB

        fu_mul[i].busy = 0;
    }
    else{
        fu_mul[i].cicles_to_end = fu_mul[i].cicles_to_end - 1;
    }
}

/**
 * Runs a cicle of a given DIV functional unit
 * @param i The index of the functional unit to be ran
 */
void runDiv(int i){
    if(fu_div[i].cicles_to_end == 1){
        (*inst_fun_div[div_map[fu_div[i].op]])(fu_div[i].dj, fu_div[i].dk);

        // Depois de executar, escrever no ROB

        fu_div[i].busy = 0;
    }
    else{
        fu_div[i].cicles_to_end = fu_div[i].cicles_to_end - 1;
    }
}

/**
 * Runs a cicle of a given SUB functional unit
 * @param i The index of the functional unit to be ran
 */
void runSub(int i){
    if(fu_sub[i].cicles_to_end == 1){
        (*inst_fun_sub[sub_map[fu_sub[i].op]])(fu_sub[i].dj, fu_sub[i].dk);

        // Depois de executar, escrever no ROB

        fu_sub[i].busy = 0;
    }
    else{
        fu_sub[i].cicles_to_end = fu_sub[i].cicles_to_end - 1;
    }
}

/**
 * Runs a cicle of a given ADD functional unit
 * @param i The index of the functional unit to be ran
 */
void runAdd(int i){
    int res;
    if(fu_add[i].cicles_to_end == 1){
        res = (*inst_fun_add[add_map[fu_add[i].op]])(fu_add[i].dj, fu_add[i].dk);

        // Depois de executar, escrever no ROB

        fu_add[i].busy = 0;
        registers[fu_add[i].fi] = res;
        reg_status[fu_add[i].fi] = NULL;
    }
    else{
        fu_add[i].cicles_to_end = fu_add[i].cicles_to_end - 1;
    }
}

int runAlu(){
    int i, ran = 0;

    printDebugMessage("Running ALU");

    for(i = 0; i < NUM_FU_MUL; i++) {
        if(!fu_mul[i].busy) continue;

        ran = 1;

        if(fu_mul[i].rj && fu_mul[i].rk)
            runMul(i);
    }

    for(i = 0; i < NUM_FU_DIV; i++) {
        if(!fu_div[i].busy) continue;

        ran = 1;

        if(fu_div[i].rj && fu_div[i].rk)
            runDiv(i);
    }

    for(i = 0; i < NUM_FU_SUB; i++) {
        if(!fu_sub[i].busy) continue;

        ran = 1;

        if(fu_sub[i].rj && fu_sub[i].rk)
            runSub(i);
    }

    for(i = 0; i < NUM_FU_ADD; i++) {
        if(!fu_add[i].busy) continue;

        ran = 1;

        if(fu_add[i].rj && fu_add[i].rk)
            runAdd(i);
    }

    return ran;
}