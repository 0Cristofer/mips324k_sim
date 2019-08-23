/* Mips32 4K simulator ALU header file
   Authors: Cristofer Oswald
   Created: 16/05/2019
   Edited: 06/07/2019 */

#ifndef MIPS324K_SIM_ALU_H
#define MIPS324K_SIM_ALU_H

#define NUM_FU_MUL 2
#define NUM_FU_DIV 2
#define NUM_FU_SUB 1
#define NUM_FU_ADD 1

#define C_ADD 2
#define C_AND 1
#define C_MFHI 1
#define C_MFLO 1
#define C_MOVN 1
#define C_MOVZ 1
#define C_MTHI 1
#define C_MTLO 1
#define C_NOR 1
#define C_OR 1
#define C_XOR 1
#define C_BGEZ 1
#define C_BLTZ 1
#define C_BEQ 1
#define C_BEQL 1
#define C_BGTZ 1
#define C_BLEZ 1
#define C_BNE 1
#define C_LUI 1
#define C_SUB 2
#define C_DIV 10
#define C_MULT 12
#define C_MUL 10
#define C_MADD 14
#define C_MSUB 14

#define HI_REG 32
#define LO_REG 33

typedef struct functional_unit functional_unit_t;
typedef struct instruction_data instruction_data_t;
typedef struct rob_entry rob_entry_t;
typedef struct register_data register_data_t;

/**
 * Scoreboarding funciontal unit structure
 */
struct functional_unit {
    instruction_data_t *instruction;
    int busy;
    int op;
    int fi, fj, fk;
    int dj, dk, hi, lo;
    long ri;
    int rj, rk;
    int cicles_to_end;
};

enum register_status {
    FREE, USED, BYPASS
};
enum register_access {
    READ, WRITE
};

struct register_data {
    enum register_status status;
    rob_entry_t *entry;
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
int isRegFree(int r, enum register_access type);

int readReg(int r);

void alocReg(int r);

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

void printBypass();

#endif //MIPS324K_SIM_ALU_H
