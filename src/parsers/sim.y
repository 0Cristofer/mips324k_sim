/* Mips32 4K simulator assembly translator
   Authors: Bruno Cesar, Cristofer Oswald and Narcizo Gabriel
   Created: 9/9/2018
   Edited: 18/10/2018 */

%{
  #include <stdio.h>

  #include "src/include/parser.h"
%}

%token ADD ADDI AND ANDI
%token B BEQ BEQL BGEZ BGTZ BLEZ BLTZ BNE
%token DIV J JR LUI
%token MADD MFHI MFLO MOVN MOVZ MSUB MTHI MTLO MUL MULT
%token NOP NOR OR ORI SUB SYSCALL XOR XORI

%token REG COMMA NEWLINE
%token IMMEDIATE

%start program

%%
program:
  instruction_list
;

instruction_list:
  instruction
  |instruction_list instruction
;

instruction:
  command NEWLINE
  |NEWLINE
;

command:
  three_reg_inst
  |two_reg_inst
  |one_reg_inst
  |two_reg_imm_inst
  |one_reg_imm_inst
  |j
  |nop
  |syscall
;

three_reg:
  ADD
  |AND
  |MOVN
  |MOVZ
  |MUL
  |NOR
  |OR
  |SUB
  |XOR
;

three_reg_inst:
  three_reg REG COMMA REG COMMA REG
;

two_reg:
  DIV
  |MADD
  |MSUB
  |MULT
;

two_reg_inst:
  two_reg REG COMMA REG
;

one_reg:
  MFHI
  |MFLO
  |MTHI
  |MTLO
;

one_reg_inst:
  one_reg REG
;

two_reg_imm:
  ADDI
  |ANDI
  |BEQ
  |BEQL
  |BNE
  |ORI
  |XORI
;

two_reg_imm_inst:
  two_reg_imm REG COMMA REG COMMA IMMEDIATE
;

one_reg_imm:
  BGEZ
  |BGTZ
  |BLEZ
  |BLTZ
  |LUI
;

one_reg_imm_inst:
  one_reg_imm REG COMMA IMMEDIATE
;

j:
  J IMMEDIATE

nop:
  NOP
;

syscall:
  SYSCALL
;

%%

int main(int argc, char** argv){

  if(argc != 2){
    printf("Invalid arguments, use: \'mips324k_sim input_file.s\'\n");

    return 1;
  }
  else{
    yyin = fopen(argv[1], "r");

    if(!yyin){
      printf("Can't open \'%s\' input file", argv[1]);

      return 1;
    }
  }

  yyparse();

  fclose(yyin);

  return 0;
}
