/* Mips32 4K simulator assembly translator header
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 19/03/2019 */

#ifndef MIPS324K_SIM_PARSER_H
#define MIPS324K_SIM_PARSER_H

#include <stdio.h>

extern int yylineno;
extern int current_inst;
extern FILE *yyin;

// Lexical parser function
int yylex();
// Syntax parser function
int yyparse();
// Syntax parser error function
void yyerror(char *s);

void add3RegIns(int op_code, int rd, int rs, int rt);
void add2RegIns(int op_code, int rs, int rt);
void add1RegIns(int op_code, int rd);

void add2RegImmIns(int op_code, int rt, int rs, int immediate);
void add1RegImmIns(int op_code, int rt, int immediate);

void add2RegOffsetIns(int op_code, int rs, int rt, int offset);
void add1RegOffsetIns(int op_code, int rs, int offset);
void addOffsetIns(int op_code, int offset);


#endif //MIPS324K_SIM_PARSER_H