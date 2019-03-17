/* Mips32 4K simulator assembly translator
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 17/03/2019 */

%option noyywrap nodefault

%{
  #include <stdio.h>

  #include "src/include/parser.h"
  #include "sim.tab.h"

  int yylineno = 1;

  void lexicalError(char *s);
%}

%%

"add"    {return ADD;}
"$t"[0-7] {return REG;}

","      {return COMMA;}

"\n" {yylineno = yylineno + 1;}
[ \t]
"//".*

. {lexicalError(yytext);}

%%

void lexicalError(char *s){
  printf("Lexical error: unrecognized character %s at line %d\n", s, yylineno);
}

void yyerror(char *s){
  printf("Syntax error at line %d. Message: %s, Character: %s\n", yylineno, s, yytext);
}
