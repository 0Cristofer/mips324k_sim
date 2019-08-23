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

int main(int argc, char **argv) {
    int i, total_instructions, ok;
    int unsigned *insts;
    char *prog_name;

    args_t args;

    printf("\t\tMIPS32 4K simulator\n\n");

    ok = readArgs(&args, argc, argv);

    if (!ok) return 0;

    if (args.help) {
        printHelp();
        return 0;
    }

    if (args.debug) printf("\tReading input assembly file.\n\n");

    ok = parseInput(args.input_name, &total_instructions, &insts, &inst_strs);
    if (!ok) {
        printf("Failed to read input file, aborting simulation.\n");
        return 0;
    }

    prog_name = getFileName(args.input_name);
    startOutFile(prog_name);

    writeProgramToOutput(args.input_name);

    writeBinaryToOutput(total_instructions, insts);

    if (args.debug) printf("\tTranslation done. Starting simulation...\n\n");

    startSimulation(insts, (unsigned int) total_instructions, args.debug, inst_strs, args.detail, out_file);

    if (args.debug) printf("\tEnd of simulation.\n");

    if (args.binary_output_name != NULL) {
        if (args.debug) printf("\tWriting binary output.\n");
        writeBinary(args.binary_output_name, total_instructions, insts);
    }

    if (args.debug) printf("\tCleaning up...\n");
    endOutFile();
    for (i = 0; i < total_instructions; i++) free(inst_strs[i]);
    free(inst_strs);
    free(insts);
    free(prog_name);

    if (args.debug) printf("\tEnd.\n");

    return 0;
}