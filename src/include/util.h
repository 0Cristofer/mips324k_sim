/* Mips32 4K simulator helper functions
   Authors: Cristofer Oswald
   Created: 21/03/2019
   Edited: 22/08/2019 */

#ifndef MIPS324K_SIM_UTIL_H
#define MIPS324K_SIM_UTIL_H

#include <getopt.h>

#define ARGSSTR "hbi:o:d"

extern FILE *out_file;

typedef struct {
    int help;
    int detail;
    int debug;
    char *input_name;
    char *binary_output_name;
} args_t;

/**
 * Sets up the argument structure based on command-line arguments
 * @param args Pointer to argument structure to be set
 * @param argc Argument count from system
 * @param argv Argument verbose from system
 * @return 1 on success, else 0
 */
int readArgs(args_t *args, int argc, char **argv);

/**
 * Writes a text file with all instructions in hexadecimal
 * @param binary_ouput_name The output file name
 * @param total_instructions The number of instructions
 * @param prog_mem The instructions to be written
 */
void writeBinary(char *binary_ouput_name, int total_instructions, unsigned int *prog_mem);

void writeProgramToOutput(char *input_name);

void writeBinaryToOutput(int total_instructions, unsigned int *insts);

void printCycles(int cycles);

/**
 * Prints the default help message
 */
void printHelp();

char *getFileName(const char *filename);

void startOutFile(char *prog_name);

void endOutFile();

#endif //MIPS324K_SIM_UTIL_H
