/* Mips32 4K simulator assembly translator helper functions
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 19/03/2019 */

#include "../include/mips324k_sim.h"

int current_inst = 0;

void add3RegIns(int op_code, int rd, int rs, int rt){
    rd = rd << 11;
    rt = rt << 16;
    rs = rs << 21;

    prog_mem[current_inst] = rs | rt | rd | op_code;

    current_inst = current_inst + 1;
}

void add2RegIns(int op_code, int rs, int rt){
    rt = rt << 16;
    rs = rs << 21;

    prog_mem[current_inst] = rs | rt | op_code;

    current_inst = current_inst + 1;
}

void add1RegIns(int op_code, int rd){
    if((op_code == 8) || (op_code == 17) ||(op_code == 19)) rd = rd << 21;
    else rd = rd << 11;;

    prog_mem[current_inst] = rd | op_code;

    current_inst = current_inst + 1;
}


void add2RegImmIns(int op_code, int rt, int rs, int immediate){
    immediate = immediate & 65535; // Truncagem para 16 bits
    rt = rt << 16;
    rs = rs << 21;

    prog_mem[current_inst] = op_code | rs | rt | immediate;

    current_inst = current_inst + 1;
}

void add1RegImmIns(int op_code, int rt, int immediate){
    immediate = immediate & 65535; // Truncagem para 16 bits
    rt = rt << 16;

    prog_mem[current_inst] = op_code | rt | immediate;

    current_inst = current_inst + 1;
}


void add2RegOffsetIns(int op_code, int rs, int rt, int offset){
    offset = offset & 65535; // Truncagem para 16 bits, talvez precise de deslocar 2 para a esquerda
    rt = rt << 16;
    rs = rs << 21;

    prog_mem[current_inst] =  op_code | rs | rt | offset;

    current_inst = current_inst + 1;
}

void add1RegOffsetIns(int op_code, int rs, int offset){
    offset = offset & 65535; // Truncagem para 16 bits, talvez precise de deslocar 2 para a esquerda
    rs = rs << 21;

    prog_mem[current_inst] =  op_code | rs | offset;

    current_inst = current_inst + 1;
}

void addOffsetIns(int op_code, int offset){
    offset = offset & 65535; // Truncagem para 16 bits, talvez precise de deslocar 2 para a esquerda

    prog_mem[current_inst] = op_code | offset;

    current_inst = current_inst + 1;
}