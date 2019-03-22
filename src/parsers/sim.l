/* Mips32 4K simulator assembly translator lexer
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 21/03/2019 */

%option noyywrap nodefault

%{
  #include <stdio.h>
  #include <string.h>
  #include <stdint.h>

  #include "src/parsers/include/parser.h"
  #include "sim.tab.h"

  int yylineno = 1;
  int yyleng = 0;

  void lexicalError(char *s);
%}

%%

"add"      {yylval.code = 32; return ADD;}
"addi"     {yylval.code = 536870912; return ADDI;}
"and"      {yylval.code = 36; return AND;}
"andi"     {yylval.code = 805306368; return ANDI;}
"b"        {yylval.code = 268435456; return B;}
"beq"      {yylval.code = 268435456; return BEQ;}
"beql"     {yylval.code = 1342177280; return BEQL;}
"bgez"     {yylval.code = 67174400; return BGEZ;}
"bgtz"     {yylval.code = 469762048; return BGTZ;}
"blez"     {yylval.code = 402653184; return BLEZ;}
"bltz"     {yylval.code = 67108864; return BLTZ;}
"bne"      {yylval.code = 335544320; return BNE;}
"div"      {yylval.code = 26; return DIV;}
"j"        {yylval.code = 134217728; return J;}
"jr"       {yylval.code = 8; return JR;}
"lui"      {yylval.code = 1006632960; return LUI;}
"madd"     {yylval.code = 1879048192; return MADD;}
"mfhi"     {yylval.code = 16; return MFHI;}
"mflo"     {yylval.code = 18; return MFLO;}
"movn"     {yylval.code = 11; return MOVN;}
"movz"     {yylval.code = 10; return MOVZ;}
"msub"     {yylval.code = 1879048196; return MSUB;}
"mthi"     {yylval.code = 17; return MTHI;}
"mtlo"     {yylval.code = 19; return MTLO;}
"mul"      {yylval.code = 1879048194; return MUL;}
"mult"     {yylval.code = 24; return MULT;}
"nop"      {yylval.code = 0; return NOP;}
"nor"      {yylval.code = 39; return NOR;}
"or"       {yylval.code = 37; return OR;}
"ori"      {yylval.code = 872415232; return ORI;}
"sub"      {yylval.code = 34; return SUB;}
"syscall"  {yylval.code = 12; return SYSCALL;}
"xor"      {yylval.code = 38; return XOR;}
"xori"     {yylval.code = 939524096; return XORI;}


"$zero"|"$0"      {yylval.code = 0; return REG;}
"$at"         {yylval.code = 1; return REG;}
"$v"[0-1]     {yylval.code = 2+atoi(yytext+2); return REG;}
"$a"[0-3]     {yylval.code = 4+atoi(yytext+2); return REG;}
"$t"[0-7]     {yylval.code = 8+atoi(yytext+2); return REG;}
"$s"[0-7]     {yylval.code = 16+atoi(yytext+2); return REG;}
"$t"[8-9]     {yylval.code = 16+atoi(yytext+2); return REG;}
"$k"[0-1]     {yylval.code = 26+atoi(yytext+2); return REG;}
"$gp"         {yylval.code = 28; return REG;}
"$sp"         {yylval.code = 29; return REG;}
"$fp"|"$s8"   {yylval.code = 30; return REG;}
"$ra"         {yylval.code = 31; return REG;}

[-+]?[0-9]+        {yylval.imm = (int16_t) atoi(yytext); return IMMEDIATE;}

","           {return COMMA;}
":"           {return COLON;}
"\n"          {yylineno = yylineno + 1; return NEWLINE;}

[ \t]
"#".*

([a-zA-Z]|_)([a-zA-Z]|_|[0-9])* {
                                  yylval.text = malloc(sizeof(char) * (yyleng + 1));
                                  strcpy(yylval.text, yytext);
                                  return LABEL;
                                }

<<EOF>>       {return END_FILE;}

. {lexicalError(yytext);}

%%

void lexicalError(char *s){
  printf("Lexical error: unrecognized character %s at line %d\n", s, yylineno);
}

void yyerror(char *s){
  printf("Syntax error at line %d. Message: %s, Character: %s\n", yylineno, s, yytext);
}
