/* Mips32 4K simulator main file
   Authors: Cristofer Oswald
   Created: 19/03/2019
   Edited: 21/03/2019 */

#include <stdio.h>
#include <stdlib.h>

#include "include/mips324k_sim.h"
#include "include/util.h"
#include "parsers/include/parser.h"
#include "simulator/include/simulator.h"

int main(int argc, char** argv){
    int i, total_instructions, ok;
    int unsigned *insts;
    char **inst_strs;

    args_t args;

    ok = readArgs(&args, argc, argv);

    if(!ok) return 0;

    if(args.help){
        printHelp();
        return 0;
    }

    ok = parseInput(args.input_name, &total_instructions, &insts, &inst_strs);
    if(!ok){
        printf("Failed to read input file, aborting simulation.\n");
        return 0;
    }

    writeProg(total_instructions, inst_strs);

    writeHexa(total_instructions, insts);

    startSimulation(insts, (unsigned int) total_instructions, args.debug, inst_strs, args.detail);

    if(args.binary_output_name != NULL){
        writeBinary(args.binary_output_name, total_instructions, insts);
    }

    for(i = 0; i < total_instructions; i++) free(inst_strs[i]);
    free(inst_strs);
    free(insts);

    return 0;
}