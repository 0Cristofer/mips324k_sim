/* Mips32 4K simulator helper functions
   Authors: Cristofer Oswald
   Created: 21/03/2019
   Edited: 21/03/2019 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/util.h"

FILE *out_file;

void initArgs(args_t *args) {
    args->help = 0;
    args->input_name = NULL;
    args->binary_output_name = NULL;
    args->detail = 0;
    args->debug = 0;
}

int readArgs(args_t *args, int argc, char **argv) {
    int opt, ok = 1;

    struct option long_options[] = {
            {"help",   no_argument,       NULL, 'h'},
            {"input",  required_argument, NULL, 'i'},
            {"output", required_argument, NULL, 'o'},
            {"detail", no_argument,       NULL, 'd'},
            {"debug",  no_argument,       NULL, 'b'},
            {NULL, 0,                     NULL, 0}
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
            case 'b':
                args->debug = 1;
                break;
            case '?':
                printf("Unknown option: \"%s\", use '-h' for help. Aborting.", argv[optind - 1]);
                ok = 0;
            default:
                break;
        }
    }

    if ((args->input_name == NULL) && (!args->help) && ok) {
        printf("No input file, stopping run.\n");
        ok = 0;
    }

    return ok;
}

char *getFileName(const char *filename) {
    char c, *name;
    int start = 0, end, i = 0;

    c = filename[0];

    while (c != '\0') {
        if (c == '.')
            break;

        if (c == '/')
            start = i + 1;

        i++;
        c = filename[i];
    }

    end = i;

    name = malloc(end - start + 1 * sizeof(char));

    for (i = 0; i < end - start; i++) {
        name[i] = filename[i + start];
    }
    name[end - start] = '\0';

    return name;
}

void startOutFile(char *prog_name) {
    char *name;

    name = malloc((strlen(prog_name) + 5) * sizeof(char));
    name[0] = '\0';
    name = strcat(name, prog_name);
    name = strcat(name, ".out");

    out_file = fopen(name, "w");

    if (out_file == NULL) {
        printf("Não foi possível criar arquivo de saída, escrevendo na saída padrão.\n");
        out_file = stdin;
    }

    free(name);
}

void endOutFile() {
    if (out_file != stdin) fclose(out_file);
}

void writeBinary(char *binary_ouput_name, int total_instructions, unsigned int *prog_mem) {
    int i;
    FILE *output_b;

    output_b = fopen(binary_ouput_name, "w");

    if (!output_b) {
        printf("Failed to open file %s to write, aborting binary write.\n", binary_ouput_name);
        return;
    }

    for (i = 0; i < total_instructions; i++) {
        fprintf(output_b, "%X\n", prog_mem[i]);
    }

    fclose(output_b);
}

void writeProgramToOutput(char *input_name) {
    char *line = NULL;
    size_t len = 0;
    FILE *input_file;

    input_file = fopen(input_name, "r");

    fprintf(out_file, "Programa:\n");

    while (getline(&line, &len, input_file) != -1) {
        fprintf(out_file, "\t%s", line);
    }

    fprintf(out_file, "\n\n");

    free(line);
    fclose(input_file);
}

void writeBinaryToOutput(int total_instructions, unsigned int *insts) {
    int i;
    fprintf(out_file, "Binário:\n");

    for (i = 0; i < total_instructions; ++i) {
        fprintf(out_file, "\t%X\n", insts[i]);
    }

    fprintf(out_file, "\n");
}

void printHelp() {
    printf("\t\tMIPS32 4K simulator\n");
    printf("Usage: ./mips324k_sim -i <prog>.asm [-o <prog>.s] [--detail] [-h]\n");
    printf("Options:\n");
    printf("\t-i (--input) The MIPS32 4K assembly program to be simulated\n");
    printf("\t-o (--output) The binary program simulated\n");
    printf("\t-d (--detail) Generates detailed output\n");
    printf("\t-b (--debug) Shows debug messages\n");
    printf("\t-h (--help) Shows this message\n");
}