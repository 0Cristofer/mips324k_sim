/* Mips32 4K simulator util functions
   Authors: Cristofer Oswald
   Created: 02/04/2019
   Edited: 02/04/2019 */

#include <stdio.h>

#include "include/simulator.h"

char*register_names[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                         "s0", "s1", "s2", "s3", "s4", "s5", "s5", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra", "hi", "lo"};

char stage_strings[7][200];
int buffer_position[7];
int current_stage;

void printAll(){
    int i;
    for ( i = 6; i >= 0 ; --i) {
        printf("%s", stage_strings[i]);
    }
};

void resetAll(){
    int i;

    for (i = 0; i < 7; ++i) {
        stage_strings[i][0] = '\0';
        buffer_position[i] = 0;
    }

    current_stage = 0;
}

void nextPrintStage(){
    current_stage++;
}

void printDebugMessage(char *message){
    if(!debug) return;
    printf("DEBUG: %s\n", message);
}

void printDebugMessageInt(char *message, int d){
    if(!debug) return;
    printf("DEBUG: %s, %d\n", message, d);
}

void printDebugError(char *stage, char *message){
    if(!debug) return;
    fprintf(stderr, "ERROR: stage - %s, message: %s\n", stage, message);
}

void printDebugRegister(enum register_name reg){
    if(!debug) return;
    printf("DEBUG: Printing register %s, %d\n", register_names[reg], registers[reg]);
}

void percentagePrediction(int total, int mistakes, int hits){
    printf("Previsão:\n");
    printf("\tTotal de saltos: \t%.2d \n", total);
    if (total != 0){
        printf("\tAcertos: \t\t\t%.2d (%2.2f%%) \n", hits , ((float) hits/ (float) total) * 100);
        printf("\tErros: \t\t\t\t%.2d (%2.2f%%) \n\n", mistakes, ((float) mistakes/ (float) total) * 100);
    } else {
        printf("\tAcertos: \t\t\t00 (00.00%%) \n");
        printf("\tErros: \t\t\t\t00 (00.00%%) \n\n");

    }
}

void percentageInstruction(int total, int effected){
    printf("Instruções:\n");
    printf("\tEmitidas: \t\t\t%.2d \n", total);
    printf("\tEfetivadas: \t\t%.2d (%2.2f%%) \n\n", effected, ((float) effected/ (float) total) * 100);
}


void printCycles(int cycles) {
    printf("Ciclos:\n");
    printf("\t%d ciclos\n\n", cycles);
}

void printCurrentCycle(int cycle) {
    if (is_detail) printf("Ciclo %d:\n", cycle);
}

void printInstruction(char *inst_str){
    if (is_detail)
        buffer_position[current_stage] +=
                sprintf(stage_strings[current_stage]+buffer_position[current_stage], "\t\t%s\n", inst_str);
}

void printStageHeader(char *stage){
    if (is_detail)
        buffer_position[current_stage] +=
                sprintf(stage_strings[current_stage]+buffer_position[current_stage], "\t%s\n", stage);
}

void printRegisterName(enum register_name reg){
    if (is_detail)
        buffer_position[current_stage] +=
                sprintf(stage_strings[current_stage]+buffer_position[current_stage], "\t\t\t%s\n", register_names[reg]);
}

void printNewLine(){
    if (is_detail)
        buffer_position[current_stage] +=
                sprintf(stage_strings[current_stage]+buffer_position[current_stage],"\n");
}

void printRegistersContent(){
    int i;

    printf ("Conteúdo dos registradores:\n");
    for (i = 0; i < NUM_REGISTERS; i++) printf("\t%s: %d\n", register_names[i], registers[i]);
    printf("\n");
}