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
        (*inst_fun_mul[mul_map[fu_mul[i].op]])(fu_mul[i].fi, fu_mul[i].fj, fu_mul[i].fk);

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
        (*inst_fun_div[div_map[fu_div[i].op]])(fu_div[i].fi, fu_div[i].fj, fu_div[i].fk);

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
        (*inst_fun_sub[sub_map[fu_sub[i].op]])(fu_sub[i].fi, fu_sub[i].fj, fu_sub[i].fk);

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
    if(fu_add[i].cicles_to_end == 1){
        (*inst_fun_add[add_map[fu_add[i].op]])(fu_add[i].fi, fu_add[i].fj, fu_add[i].fk);

        // Depois de executar, escrever no ROB

        fu_add[i].busy = 0;
        reg_status[fu_add[i].fi] = NULL;
    }
    else{
        fu_add[i].cicles_to_end = fu_add[i].cicles_to_end - 1;
    }
}

void runAlu(){
    int i;

    for(i = 0; i < NUM_FU_MUL; i++) {
        if(!fu_mul[i].busy) continue;

        runMul(i);
    }

    for(i = 0; i < NUM_FU_DIV; i++) {
        if(!fu_div[i].busy) continue;

        runDiv(i);
    }

    for(i = 0; i < NUM_FU_SUB; i++) {
        if(!fu_sub[i].busy) continue;

        runSub(i);
    }

    for(i = 0; i < NUM_FU_ADD; i++) {
        if(!fu_add[i].busy) continue;

        runAdd(i);
    }
}