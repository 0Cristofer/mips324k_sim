/* Mips32 4K simulator assembly translator helper functions
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 20/03/2019 */

#include <string.h>
#include <stdlib.h>

#include "include/parser.h"
#include "../include/mips324k_sim.h"

int current_inst = 0;
int inst_size = 0;

int current_label_defs = 0;
int label_defs_size = 0;

int current_uses = 0;
int uses_size = 0;

int realloc_step = 1024;

label_use_t *label_uses = NULL;
label_def_t *label_defs = NULL;

void addInst(){
    if(current_inst == inst_size){
        prog_mem = realloc(prog_mem, sizeof(int) * realloc_step);
        inst_size = inst_size + realloc_step;
    }
}

void add3RegIns(int op_code, int rd, int rs, int rt){
    addInst();

    rd = rd << 11;
    rt = rt << 16;
    rs = rs << 21;

    prog_mem[current_inst] = rs | rt | rd | op_code;

    current_inst = current_inst + 1;
}

void add2RegIns(int op_code, int rs, int rt){
    addInst();

    rt = rt << 16;
    rs = rs << 21;

    prog_mem[current_inst] = rs | rt | op_code;

    current_inst = current_inst + 1;
}

void add1RegIns(int op_code, int rd){
    addInst();

    if((op_code == 8) || (op_code == 17) ||(op_code == 19)) rd = rd << 21;
    else rd = rd << 11;;

    prog_mem[current_inst] = rd | op_code;

    current_inst = current_inst + 1;
}


void add2RegImmIns(int op_code, int rt, int rs, int immediate){
    addInst();

    immediate = immediate & 65535;
    rt = rt << 16;
    rs = rs << 21;

    prog_mem[current_inst] = op_code | rs | rt | immediate;

    current_inst = current_inst + 1;
}

void add1RegImmIns(int op_code, int rt, int immediate){
    addInst();

    immediate = immediate & 65535;
    rt = rt << 16;

    prog_mem[current_inst] = op_code | rt | immediate;

    current_inst = current_inst + 1;
}


void add2RegOffsetIns(int op_code, int rs, int rt) {
    addInst();

    rt = rt << 16;
    rs = rs << 21;

    prog_mem[current_inst] =  op_code | rs | rt;

    current_inst = current_inst + 1;
}

void add1RegOffsetIns(int op_code, int rs) {
    addInst();

    rs = rs << 21;

    prog_mem[current_inst] =  op_code | rs;

    current_inst = current_inst + 1;
}

void addOffsetIns(int op_code) {
    addInst();

    prog_mem[current_inst] = op_code;

    current_inst = current_inst + 1;
}

void addLabel(char* label, int inst){
    if(current_label_defs == label_defs_size){
        label_defs = realloc(label_defs, sizeof(label_def_t) * realloc_step);
        label_defs_size = label_defs_size + realloc_step;
    }

    label_defs[current_label_defs].label = label;
    label_defs[current_label_defs].inst = inst;

    current_label_defs = current_label_defs + 1;
}

void addLabelUse(int loc, int inst){
    if(current_uses == uses_size){
        label_uses = realloc(label_uses, sizeof(label_use_t) * realloc_step);
        uses_size = uses_size + realloc_step;
    }

    label_uses[current_uses].label = label_defs+loc;
    label_uses[current_uses].inst = inst;

    current_uses = current_uses + 1;
}

int newLabel(char* label){
    int loc;

    loc = findLabel(label);
    if(loc != -1) {
        if (label_defs[loc].inst != -1){
            printf("Label already defined: %s, stopping file read.\n", label);

            ok = 0;
            free(label);
            endParse();

            return 0;
        }
        else {
            free(label);

            label_defs[loc].inst = current_inst;

            return 1;
        }
    }

    addLabel(label, current_inst);

    return 1;
}

void useLabel(char *label){
    int loc;

    loc = findLabel(label);

    if(loc == -1){
        addLabel(label, -1);
        loc = current_label_defs - 1;
    }
    else{
        free(label);
    }

    addLabelUse(loc, current_inst);
}

int findLabel(char *label){
    int i, found = -1;

    for(i = 0; i < current_label_defs; i++){
        if(!strcmp(label_defs[i].label, label)){
            found = i;
            break;
        }
    }

    return found;
}

void endParse(){
    int i, offset;

    if(ok) {
        for (i = 0; i < current_uses; i++) {
            if (label_uses[i].label->inst == -1) {
                printf("Undefined label usage: %s, aborting\n", label_uses[i].label->label);
                ok = 0;
                break;
            }

            offset = (label_uses[i].label->inst - label_uses[i].inst) ; // Ver truncagem 16 bits e 18 bits
            prog_mem[label_uses[i].inst] = prog_mem[label_uses[i].inst] | offset;
        }
    }

    for (i = 0; i < current_label_defs; i++) {
        free(label_defs[i].label);
    }

    free(label_uses);
    free(label_defs);
}