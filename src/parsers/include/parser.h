/* Mips32 4K simulator assembly translator header
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 20/03/2019 */

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

typedef struct{
    char *label;
    int inst;
}label_def_t;

typedef struct{
    label_def_t* label;
    int inst;
}label_use_t;

extern label_def_t *label_defs;
extern label_use_t *label_uses;
extern int inst_size;
extern int current_uses;
extern int uses_size;
extern int current_label_defs;
extern int label_defs_size;
extern int realloc_step;

void add3RegIns(int op_code, int rd, int rs, int rt);
void add2RegIns(int op_code, int rs, int rt);
void add1RegIns(int op_code, int rd);

void add2RegImmIns(int op_code, int rt, int rs, int immediate);
void add1RegImmIns(int op_code, int rt, int immediate);

void add2RegOffsetIns(int op_code, int rs, int rt);
void add1RegOffsetIns(int op_code, int rs);
void addOffsetIns(int op_code);

int newLabel(char* label);
void useLabel(char *label);
int findLabel(char *label);
void endParse();

#endif //MIPS324K_SIM_PARSER_H