/* Mips32 4K simulator assembly translator syntax
   Author: Cristofer Oswald
   Created: 17/03/2019
   Edited: 30/05/2019 */

%{
  #include <stdint.h>

  #include "src/parsers/include/parser.h"
%}

%union{
  unsigned int code;
  int16_t imm;
  char *text;
}

%token ADD ADDI AND ANDI
%token B BEQ BEQL BGEZ BGTZ BLEZ BLTZ BNE
%token DIV J LUI
%token MADD MFHI MFLO MOVN MOVZ MSUB MTHI MTLO MUL MULT
%token NOP NOR OR ORI SUB SYSCALL XOR XORI

%token REG LABEL IMMEDIATE

%token COMMA COLON NEWLINE
%token END_FILE 0

%type <code> three_reg two_reg one_reg
%type <code> two_reg_imm one_reg_imm
%type <code> two_reg_label one_reg_label one_label
%type <code> nop syscall
%type <code> reg
%type <imm> immediate

%start program

%%

program:
  instruction_list {endParse();}
;

instruction_list:
  instruction
  |instruction instruction_list
;

instruction:
  command NEWLINE
  |command END_FILE
  |label command NEWLINE
  |label command END_FILE
  |label NEWLINE
  |label END_FILE
  |NEWLINE
;

command:
  three_reg_inst {addLine(yylineno);}
  |two_reg_inst {addLine(yylineno);}
  |one_reg_inst {addLine(yylineno);}
  |two_reg_imm_inst {addLine(yylineno);}
  |one_reg_imm_inst {addLine(yylineno);}
  |two_reg_label_inst {addLine(yylineno);}
  |one_reg_label_inst {addLine(yylineno);}
  |one_label_inst {addLine(yylineno);}
  |other_inst {addLine(yylineno);}
;

three_reg:
  ADD   {$$ = yylval.code;}
  |AND  {$$ = yylval.code;}
  |MOVN {$$ = yylval.code;}
  |MOVZ {$$ = yylval.code;}
  |MUL  {$$ = yylval.code;}
  |NOR  {$$ = yylval.code;}
  |OR   {$$ = yylval.code;}
  |SUB  {$$ = yylval.code;}
  |XOR  {$$ = yylval.code;}
;

three_reg_inst:
  three_reg reg COMMA reg COMMA reg {add3RegIns($1, $2, $4, $6);}
;

two_reg:
  DIV   {$$ = yylval.code;}
  |MADD {$$ = yylval.code;}
  |MSUB {$$ = yylval.code;}
  |MULT {$$ = yylval.code;}
;

two_reg_inst:
  two_reg reg COMMA reg {add2RegIns($1, $2, $4);}
;

one_reg:
  MFHI {$$ = yylval.code;}
  |MFLO {$$ = yylval.code;}
  |MTHI {$$ = yylval.code;}
  |MTLO {$$ = yylval.code;}
;

one_reg_inst:
  one_reg reg {add1RegIns($1, $2);}
;

two_reg_imm:
  ADDI  {$$ = yylval.code;}
  |ANDI {$$ = yylval.code;}
  |ORI  {$$ = yylval.code;}
  |XORI {$$ = yylval.code;}
;

two_reg_imm_inst:
  two_reg_imm reg COMMA reg COMMA immediate {add2RegImmIns($1, $2, $4, $6);}
;

one_reg_imm:
  LUI {$$ = yylval.code;}
;

one_reg_imm_inst:
  one_reg_imm reg COMMA immediate {add1RegImmIns($1, $2, $4);}
;

two_reg_label:
  BEQ   {$$ = yylval.code;}
  |BEQL {$$ = yylval.code;}
  |BNE  {$$ = yylval.code;}
;

two_reg_label_inst:
  two_reg_label reg COMMA reg COMMA label_use {add2RegOffsetIns($1, $2, $4);}
;

one_reg_label:
  BGEZ  {$$ = yylval.code;}
  |BGTZ {$$ = yylval.code;}
  |BLEZ {$$ = yylval.code;}
  |BLTZ {$$ = yylval.code;}
;

one_reg_label_inst:
  one_reg_label reg COMMA label_use {add1RegOffsetIns($1, $2);}
;

one_label:
  B   {$$ = yylval.code;}
  |J  {$$ = yylval.code;}
;

one_label_inst:
  one_label label_use {addOffsetIns($1);}
;

other_inst:
  nop {add3RegIns($1, 0, 0, 0);}
  |syscall {addSyscall($1);}
;

nop:
  NOP {$$ = yylval.code;}
;

syscall:
  SYSCALL {$$ = yylval.code;}
;

reg:
  REG {$$ = yylval.code;}
;

immediate:
  IMMEDIATE {$$ = yylval.code;}
;

label:
  LABEL COLON {if(!newLabel(yylval.text)) YYABORT;}
;

label_use:
  LABEL {useLabel(yylval.text);}
;

%%
