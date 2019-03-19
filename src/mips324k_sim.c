/* Mips32 4K simulator main file
   Authors: Cristofer Oswald
   Created: 19/03/2019
   Edited: 19/03/2019 */

#include <stdio.h>

#include "include/mips324k_sim.h"
#include "parsers/include/parser.h"

int prog_mem[1024];

int main(int argc, char** argv){
    int i;

    if(argc != 2){
        printf("Invalid arguments, use: \'mips324k_sim input_file.asm\'\n");

        return 1;
    }
    else{
        yyin = fopen(argv[1], "r");

        if(!yyin){
            printf("Can't open \'%s\' input file\n", argv[1]);

            return 1;
        }
    }

    yyparse();

    fclose(yyin);

    printf("Total de instruções: %d.\nPrintando instruções:\n", current_inst);

    for(i = 0; i < current_inst; i++){
        printf("%X\n", prog_mem[i]);
    }

    return 0;
}
