/* Mips32 4K simulator helper functions
   Authors: Cristofer Oswald
   Created: 21/03/2019
   Edited: 21/03/2019 */

#include <stdio.h>

#include "include/util.h"

void initArgs(args_t* args) {
    args->help = 0;
    args->input_name = NULL;
    args->binary_output_name = NULL;
    args->detail = 0;
}

int readArgs(args_t* args, int argc, char** argv) {
    int opt, ok = 1;

    struct option long_options[] = {
            {"help",    no_argument, NULL, 'h'},
            {"input", required_argument, NULL, 'i'},
            {"output", required_argument, NULL, 'o'},
            {"detail", no_argument, NULL, 'd'},
            {NULL, 0, NULL, 0}
    };

    opterr = 0;
    initArgs(args);

    while ((opt = getopt_long(argc, argv, ARGSSTR, long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                args->help = 1;
                break;
            case 'i':
                args->input_name = optarg;
                break;
            case 'o':
                args->binary_output_name = optarg;
                break;
            case 'd':
                args->detail = 1;
                break;
            case '?':
                printf("Unknown option: \"%s\", use '-h' for help. Aborting.", argv[optind-1]);
                ok = 0;
            default:
                break;
        }
    }

    if((args->input_name == NULL) && (!args->help) && ok) {
        printf("No input file, stopping run.\n");
        ok = 0;
    }

    return ok;
}

void writeBinary(char *binary_ouput_name, int total_instructions, unsigned int *prog_mem) {
    int i;
    FILE* output_b;

    output_b = fopen(binary_ouput_name, "w");

    if (!output_b) {
        printf("Failed to open file %s to write, aborting binary write.\n", binary_ouput_name);
        return;
    }

    for(i = 0; i < total_instructions; i++) {
        fprintf(output_b, "%X\n", prog_mem[i]);
    }

    fclose(output_b);
}

void printHelp() {
    printf("\t\tMIPS32 4K simulator\n");
    printf("Usage: ./mips324k_sim -i <prog>.asm [-o <prog>.s] [--detail] [-h]\n");
    printf("Options:\n");
    printf("\t-i (--input) The MIPS32 4K assembly program to be simulated\n");
    printf("\t-o (--output) The binary program simulated\n");
    printf("\t-d (--detail) Generates detailed output\n");
    printf("\t-h (--help) Shows this message\n");
}