/* Mips32 4K simulator ALU header file
   Authors: Cristofer Oswald
   Created: 16/05/2019
   Edited: 27/05/2019 */

#ifndef MIPS324K_SIM_ALU_H
#define MIPS324K_SIM_ALU_H

#define NUM_FU_MUL 2 //2
#define NUM_FU_DIV 2 //2
#define NUM_FU_SUB 1 //1
#define NUM_FU_ADD 1 //1

#define HI_REG 32
#define LO_REG 33

typedef struct functional_unit functional_unit_t;
typedef struct instruction_data instruction_data_t;

/**
 * Scoreboarding funciontal unit structure
 */
struct functional_unit{
    instruction_data_t *instruction;
    int busy;
    int op;
    int fi, fj, fk;
    int dj, dk;
    functional_unit_t *qj, *qk;
    int ri, rj, rk;
    int cicles_to_end;
};

/*
 * These maps are needed to map the instrucion code to the instrucion function array
 */
extern int mul_map[];
extern int div_map[];
extern int sub_map[];
extern int add_map[];

/* Number of cicles needed for each instruction */
extern int cicles_mul[];
extern int cicles_div[];
extern int cicles_sub[];
extern int cicles_add[];

/**
 * Initiates the ALU. Should be called in the simulator initialization.
 */
void initAlu();

/**
 * Verifies if a register has no functional unit writing to it.
 * @param r The register to be verified
 * @return 1 if the register is free, else 0
 */
int isRegFree(int r);

void alocReg(int r, functional_unit_t* f);

void freeReg(int r);

/**
 * Verifies if there a is MUL functional unit free
 * @return
 */
functional_unit_t *hasFuMul();

/**
 * Verifies if there a is DIV functional unit free
 * @return
 */
functional_unit_t *hasFuDiv();

/**
 * Verifies if there a is SUB functional unit free
 * @return
 */
functional_unit_t *hasFuSub();

/**
 * Verifies if there a is ADD functional unit free
 * @return
 */
functional_unit_t *hasFuAdd();

/**
 * Runs a cicle for each busy functional unit
 */
int runAlu();

#endif //MIPS324K_SIM_ALU_H
