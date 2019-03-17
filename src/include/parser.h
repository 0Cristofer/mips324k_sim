/* Mips32 4K simulator assembly translator header
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 17/03/2019 */

#ifndef MIPS324K_SIM_PARSER_H
#define MIPS324K_SIM_PARSER_H

extern int yylineno;
extern FILE *yyin;

// Lexical parser function
int yylex();
// Syntax parser function
int yyparse();
// Syntax parser error function
void yyerror(char *s);

#endif //MIPS324K_SIM_PARSER_H