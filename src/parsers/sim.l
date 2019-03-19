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

"add"      {return ADD;}
"addi"     {return ADDI;}
"and"      {return AND;}
"andi"     {return ANDI;}
"b"        {return B;}
"beq"      {return BEQ;}
"beql"     {return BEQL;}
"bgez"     {return BGEZ;}
"bgtz"     {return BGTZ;}
"blez"     {return BLEZ;}
"bltz"     {return BLTZ;}
"bne"      {return BNE;}
"div"      {return DIV;}
"j"        {return J;}
"jr"       {return JR;}
"lui"      {return LUI;}
"madd"     {return MADD;}
"mfhi"     {return MFHI;}
"mflo"     {return MFLO;}
"movn"     {return MOVN;}
"movz"     {return MOVZ;}
"msub"     {return MSUB;}
"mthi"     {return MTHI;}
"mtlo"     {return MTLO;}
"mul"      {return MUL;}
"mult"     {return MULT;}
"nop"      {return NOP;}
"nor"      {return NOR;}
"or"       {return OR;}
"ori"      {return ORI;}
"sub"      {return SUB;}
"syscall"  {return SYSCALL;}
"xor"      {return XOR;}
"xori"     {return XORI;}


"$zero"       {return REG;}
"$at"         {return REG;}
"$v"[0-1]     {return REG;}
"$a"[0-3]     {return REG;}
"$t"[0-9]     {return REG;}
"$s"[0-7]     {return REG;}
"$k"[0-1]     {return REG;}
"$gp"         {return REG;}
"$sp"         {return REG;}
"$fp"|"$s8"   {return REG;}
"$ra"         {return REG;}

[0-9]+        {return IMMEDIATE;}

","           {return COMMA;}
"\n"          {yylineno = yylineno + 1; return NEWLINE;}

[ \t]
"#".*
<<EOF>> {printf("a\n");return END_FILE;}
. {lexicalError(yytext);}

%%

void lexicalError(char *s){
  printf("Lexical error: unrecognized character %s at line %d\n", s, yylineno);
}

void yyerror(char *s){
  printf("Syntax error at line %d. Message: %s, Character: %s\n", yylineno, s, yytext);
}
