/* Mips32 4K simulator assembly translator
   Authors: Bruno Cesar, Cristofer Oswald and Narcizo Gabriel
   Created: 9/9/2018
   Edited: 18/10/2018 */

%{
  #include <stdio.h>

  #include "src/include/parser.h"
%}

%token ADD REG COMMA

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
  add
;

add:
  ADD REG COMMA REG COMMA REG;

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
