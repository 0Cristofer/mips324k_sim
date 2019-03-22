/* Mips32 4K simulator main file
   Authors: Cristofer Oswald
   Created: 19/03/2019
   Edited: 21/03/2019 */

#include <stdio.h>
#include <stdlib.h>

#include "include/mips324k_sim.h"
#include "include/util.h"
#include "parsers/include/parser.h"

int main(int argc, char** argv){
    int i, total_instructions, ok;
    int unsigned *prog_mem;
    char **inst_strs;

    args_t args;

    printf("\t\tMIPS32 4K simulator\n");

    ok = readArgs(&args, argc, argv);

    if(!ok) return 0;

    if(args.help){
        printHelp();
        return 0;
    }

    printf("\tReading input assembly file.\n");

    ok = parseInput(args.input_name, &total_instructions, &prog_mem, &inst_strs);
    if(!ok){
        printf("Failed to read input file, aborting simulation.\n");
        return 0;
    }

    printf("\tTranslation done. Starting simulation...\n");


    printf("\tEnd of simulation.\n");

    if(args.binary_output_name != NULL){
        printf("\tWriting binary output,\n");
        writeBinary(args.binary_output_name, total_instructions, prog_mem);
    }

    printf("\tWriting simulation ouput.\n");

    printf("\tCleaning up...\n");
    for(i = 0; i < total_instructions; i++) free(inst_strs[i]);
    free(inst_strs);
    free(prog_mem);

    printf("\tEnd.\n");
    return 0;
}