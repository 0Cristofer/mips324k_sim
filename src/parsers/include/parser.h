/* Mips32 4K simulator assembly translator header
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 30/05/2019 */

#ifndef MIPS324K_SIM_PARSER_H
#define MIPS324K_SIM_PARSER_H

#include <stdio.h>

/* Bison/Flex definitions */

extern int yylineno;
extern int yyleng;
extern int current_inst;
extern FILE *yyin;

int yylex();

int yyparse();

void yyerror(char *s);

/* Parser helper data structures/definitions */

typedef struct {
    char *label;
    int inst;
} label_def_t;

typedef struct {
    label_def_t *label;
    int inst;
} label_use_t;

/**
 * Parses the input file, converting the assembly code to MIPS32 4K binary instructions
 * @param file_name Input file name
 * @param n_inst At exit, will be set to the number of instructions read
 * @param insts At exit, will be an array containing the instructions. Should be freed.
 * @param insts_strs At exit, will be set an array containing the each instruction line
 * @return 1 on success, else 0
 */
int parseInput(char *file_name, int *n_inst, unsigned int **insts, char ***insts_strs);

void add3RegIns(unsigned int op_code, unsigned int rd, unsigned int rs, unsigned int rt);

void add2RegIns(unsigned int op_code, unsigned int rs, unsigned int rt);

void add1RegIns(unsigned int op_code, unsigned int rd);

void add2RegImmIns(unsigned int op_code, unsigned int rt, unsigned int rs, int16_t immediate);

void add1RegImmIns(unsigned int op_code, unsigned int rt, int16_t immediate);

void add2RegOffsetIns(unsigned int op_code, unsigned int rs, unsigned int rt);

void add1RegOffsetIns(unsigned int op_code, unsigned int rs);

void addOffsetIns(unsigned int op_code);

void addSyscall(unsigned int op_code);

int newLabel(char *label);

void useLabel(char *label);

int findLabel(char *label);

void addLine(int line);

void endParse();

#endif //MIPS324K_SIM_PARSER_H