/* Mips32 4K simulator assembly translator helper functions
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 17/08/2019 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/parser.h"

int current_inst = 0;
int inst_size = 0;

int current_label_defs = 0;
int label_defs_size = 0;

int current_uses = 0;
int uses_size = 0;

int inst_lines_size = 0;
int current_line = 0;

label_use_t *label_uses = NULL;
label_def_t *label_defs = NULL;
unsigned int *instructions = NULL;
int *inst_lines = NULL;

int realloc_step = 1024;

int ok = 1;

int parseInput(char *file_name, int *n_inst, unsigned int **insts, char ***insts_strs) {
    int i, j;
    char *line = NULL;
    size_t len = 0;

    yyin = fopen(file_name, "r");

    if (!yyin) {
        printf("Can't open input file '%s'\n", file_name);
        return 0;
    }

    yyparse();

    /* Reads the instruction lines */

    if (current_inst == 0) return 0;

    rewind(yyin);
    *insts_strs = malloc(sizeof(char *) * current_line);

    i = 1;
    j = 0;
    while (getline(&line, &len, yyin) != -1) {
        len = strlen(line);

        if (i == inst_lines[j]) {
            (*insts_strs)[j] = malloc(sizeof(char) * (len + 1));

            if (line[len - 1] == '\n') {
                line[len - 1] = '\0';
            } else {
                line[len] = '\0';
            }

            strcpy((*insts_strs)[j], line);
            j = j + 1;
        }

        i = i + 1;
    }

    free(line);
    fclose(yyin);
    free(inst_lines);

    *n_inst = current_inst;
    *insts = instructions;

    return ok;
}

void addLine(int line) {
    if (current_line == inst_lines_size) {
        inst_lines = realloc(inst_lines, sizeof(int) * realloc_step);
        inst_lines_size = inst_lines_size + realloc_step;
    }

    inst_lines[current_line] = line;
    current_line = current_line + 1;
}

void addInst() {
    if (current_inst == inst_size) {
        instructions = realloc(instructions, sizeof(unsigned int) * realloc_step);
        inst_size = inst_size + realloc_step;
    }
}

void add3RegIns(unsigned int op_code, unsigned int rd, unsigned int rs, unsigned int rt) {
    addInst();

    rd = rd << (unsigned int) 11;
    rt = rt << (unsigned int) 16;
    rs = rs << (unsigned int) 21;

    instructions[current_inst] = rs | rt | rd | op_code;

    current_inst = current_inst + 1;
}

void add2RegIns(unsigned int op_code, unsigned int rs, unsigned int rt) {
    addInst();

    rt = rt << (unsigned int) 16;
    rs = rs << (unsigned int) 21;

    instructions[current_inst] = rs | rt | op_code;

    current_inst = current_inst + 1;
}

void add1RegIns(unsigned int op_code, unsigned int rd) {
    addInst();

    if ((op_code == 8) || (op_code == 17) || (op_code == 19)) rd = rd << (unsigned int) 21;
    else rd = rd << (unsigned int) 11;

    instructions[current_inst] = rd | op_code;

    current_inst = current_inst + 1;
}


void add2RegImmIns(unsigned int op_code, unsigned int rt, unsigned int rs, int16_t immediate) {
    addInst();

    rt = rt << (unsigned int) 16;
    rs = rs << (unsigned int) 21;

    instructions[current_inst] = op_code | rs | rt | (uint16_t) immediate;

    current_inst = current_inst + 1;
}

void add1RegImmIns(unsigned int op_code, unsigned int rt, int16_t immediate) {
    addInst();

    rt = rt << (unsigned int) 16;

    instructions[current_inst] = op_code | rt | (uint16_t) immediate;

    current_inst = current_inst + 1;
}


void add2RegOffsetIns(unsigned int op_code, unsigned int rs, unsigned int rt) {
    addInst();

    rt = rt << (unsigned int) 16;
    rs = rs << (unsigned int) 21;

    instructions[current_inst] = op_code | rs | rt;

    current_inst = current_inst + 1;
}

void add1RegOffsetIns(unsigned int op_code, unsigned int rs) {
    addInst();

    rs = rs << (unsigned int) 21;

    instructions[current_inst] = op_code | rs;

    current_inst = current_inst + 1;
}

void addOffsetIns(unsigned int op_code) {
    addInst();

    instructions[current_inst] = op_code;

    current_inst = current_inst + 1;
}

void addSyscall(unsigned int op_code) {
    addInst();

    instructions[current_inst] = op_code;

    current_inst = current_inst + 1;
}

void addLabel(char *label, int inst) {
    if (current_label_defs == label_defs_size) {
        label_defs = realloc(label_defs, sizeof(label_def_t) * realloc_step);
        label_defs_size = label_defs_size + realloc_step;
    }

    label_defs[current_label_defs].label = label;
    label_defs[current_label_defs].inst = inst;

    current_label_defs = current_label_defs + 1;
}

void addLabelUse(int loc, int inst) {
    if (current_uses == uses_size) {
        label_uses = realloc(label_uses, sizeof(label_use_t) * realloc_step);
        uses_size = uses_size + realloc_step;
    }

    label_uses[current_uses].label = label_defs + loc;
    label_uses[current_uses].inst = inst;

    current_uses = current_uses + 1;
}

int newLabel(char *label) {
    int loc;

    loc = findLabel(label);
    if (loc != -1) {
        if (label_defs[loc].inst != -1) {
            printf("Label already defined: %s, stopping file read.\n", label);

            ok = 0;
            free(label);
            endParse();

            return 0;
        } else {
            free(label);

            label_defs[loc].inst = current_inst;

            return 1;
        }
    }

    addLabel(label, current_inst);

    return 1;
}

void useLabel(char *label) {
    int loc;

    loc = findLabel(label);

    if (loc == -1) {
        addLabel(label, -1);
        loc = current_label_defs - 1;
    } else {
        free(label);
    }

    addLabelUse(loc, current_inst);
}

int findLabel(char *label) {
    int i, found = -1;

    for (i = 0; i < current_label_defs; i++) {
        if (!strcmp(label_defs[i].label, label)) {
            found = i;
            break;
        }
    }

    return found;
}

void endParse() {
    int i, offset32;
    uint16_t inst1, inst2, offset16;

    if (ok) {
        for (i = 0; i < current_uses; i++) {
            if (label_uses[i].label->inst == -1) {
                printf("Undefined label usage: %s, aborting\n", label_uses[i].label->label);
                ok = 0;
                break;
            }

            if (instructions[label_uses[i].inst] == 134217728) {
                offset32 = ((unsigned int) label_uses[i].label->inst & (unsigned int) 67108863);

                instructions[label_uses[i].inst] = instructions[label_uses[i].inst] | (unsigned int) offset32;
            } else {
                inst1 = (uint16_t) label_uses[i].label->inst;
                inst2 = (uint16_t) label_uses[i].inst;

                offset16 = inst1 - inst2;
                instructions[label_uses[i].inst] = instructions[label_uses[i].inst] | offset16;
            }

        }
    }

    for (i = 0; i < current_label_defs; i++) {
        free(label_defs[i].label);
    }

    free(label_uses);
    free(label_defs);
}