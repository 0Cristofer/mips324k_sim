/* Mips32 4K simulator main file
   Authors: Cristofer Oswald
   Created: 19/03/2019
   Edited: 20/03/2019 */

#include <stdio.h>
#include <stdlib.h>

#include "include/mips324k_sim.h"
#include "parsers/include/parser.h"

int ok = 1;

int *prog_mem = NULL;

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

    if(!ok) printf("Failed to read input file, aborting simulation.\n");
    else {
        printf("Total de instructions: %d.\nPrinting instructions:\n", current_inst);

        for (i = 0; i < current_inst; i++) {
            printf("%X\n", prog_mem[i]);
        }
    }

    clearAll();

    return 0;
}

void clearAll(){
    free(prog_mem);
}