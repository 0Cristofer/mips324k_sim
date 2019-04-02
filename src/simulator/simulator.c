/* Mips32 4K simulator implementations file
   Authors: Cristofer Oswald
   Created: 29/03/2019
   Edited: 02/04/2019 */

#include "include/simulator.h"
#include "include/util.h"

#include <stdlib.h>
#include <stdint.h>

int debug;
int has_error = 0;

void startSimulation(unsigned int *insts, unsigned int num_insts, int b){
    int i;

    debug = b;
    instructions = insts;
    running = 1;
    num_instructions = num_insts;
    pc = 0;

    for(i = 0; i < NUM_REGISTERS; i++){
        registers[i] = 0;
    }

    clock();
}

void clock(){
    while(running){
        pipeline();
    }
}

void pipeline(){
    write();
    align();
    memory();
    execution();
    instruction();
}

void instruction(){
    unsigned int instruction, op_type, op_code, rd = 0, rt = 0, rs = 0, offset = 0, code = 0;
    uint16_t immediate = 0;

    if(has_error) return;

    instruction = instructions[pc];

    printDebugMessageInt("Decoding instruction", pc);

    // Decodification
    switch (instruction >> 26){
        case OP_DECODE_0:
            op_type = SPECIAL;

            op_code =  instruction & (unsigned int)63; // Get only first 6 bits

            if(op_code == SYSCALL){
                code = instruction >> 6;
            }
            else {

                rd = (instruction >> 11) & (unsigned int) 31; // Get only first 5 bits
                rt = (instruction >> 16) & (unsigned int) 31;
                rs = (instruction >> 21) & (unsigned int) 31;
            }

            break;

        case OP_DECODE_1:
            op_type = REGIMM;

            offset =  instruction & (unsigned int)65535; // Get only first 16 bits
            op_code = ((instruction >> 16) & (unsigned int)31);
            rs = (instruction >> 21) & (unsigned int)31;

            break;

        case OP_DECODE_2:
            op_type = SPECIAL2;

            op_code =  instruction & (unsigned int)63; // Get only first 6 bits
            rd = (instruction >> 11) & (unsigned int)31; // Get only first 5 bits
            rt = (instruction >> 16) & (unsigned int)31;
            rs = (instruction >> 21) & (unsigned int)31;

            break;

        default:
            op_type = NONE;

            op_code = instruction >> 26;

            if(op_code == J){
                offset = instruction & (unsigned int)67108863;
            }

            immediate = (uint16_t)(instruction & (unsigned int)65535);
            rt = (instruction >> 16) & (unsigned int)31;
            rs = (instruction >> 21) & (unsigned int)31;

            break;
    }

    inst_barrier.op_type = op_type;
    inst_barrier.op_code = op_code;
    inst_barrier.rd = rd;
    inst_barrier.rt = rt;
    inst_barrier.rs = rs;
    inst_barrier.immediate = immediate;
    inst_barrier.offset = offset;
    inst_barrier.code = code;

    updatePc(op_type, op_code, rs, rt, immediate, offset);

    if(pc == num_instructions) running = 0;
}

void execution(){
    if(has_error) return;
}

void memory(){
    if(has_error) return;
}

void align(){
    if(has_error) return;
}

void write(){
    if(has_error) return;
}

void updatePc(unsigned int op_type, unsigned int op_code,
              unsigned int rs, unsigned int rt, uint16_t immediate, unsigned int offset){

    if(!branchTest(op_type, op_code, rs, rt, immediate, offset))
        pc = pc + 1;
}

unsigned int branchTest(unsigned int op_type, unsigned int op_code,
                        unsigned int rs, unsigned int rt, uint16_t immediate, unsigned int offset){

    switch (op_type){
        case SPECIAL:
            if(op_code == JR) unconditionalBranch(op_code, rs, immediate, offset);
            else return 0;

            return 1;

        case REGIMM:
            branchPredictor();
            return 1;

        case SPECIAL2:
            return 0;

        case NONE:
            switch (op_code){
                case J:
                    unconditionalBranch(op_code, rs, immediate, offset);
                    return 1;

                case B:
                    if((rt == 0) && (rs == 0)) unconditionalBranch(op_code, rs, immediate, offset);
                    else branchPredictor();
                    return 1;

                case BEQL:
                case BGTZ:
                case BLEZ:
                case BNE:
                    branchPredictor();
                    return 1;

                default:
                    return 0;
            }

        default:
            printDebugError("Instruction/branch predictor", "Unkown op type");
            error();
            return 0;
    }
}

void branchPredictor(){
    int i;

    two_bit_t bht[BRENCH_PRED_SIZE];

    for(i = 0; i < BRENCH_PRED_SIZE; i++){
        bht[i].is_new = 1;
    }

    unsigned int twoBitPred(uint16_t index){
        if(index < pc){
            if(bht[index].is_new){
                bht[index].is_new = 0;
                bht[index].pred = 3;
            }
        }
        else{
            if(bht[index].is_new){
                bht[index].is_new = 0;
                bht[index].pred = 0;
            }
        }

        return bht[index].pred;
    }

    void updateBht(uint16_t index, unsigned int b){
        if(b){
            if(bht[index].pred != 3) bht[index].pred = bht[index].pred + 1;
        }
        else{
            if(bht[index].pred != 0) bht[index].pred = bht[index].pred - 1;
        }
    }

}

void unconditionalBranch(unsigned int op_c, unsigned int r, uint16_t imm, unsigned int off){
    if(has_error) return;

    printDebugMessageInt("inconditional branch from", pc);

    switch (op_c){
        case B:
            pc = pc + (int16_t)imm;
            break;
        case J:
            pc = off;
            break;
        case JR:
            pc = registers[r];
            break;
        default:
            printDebugError("Instruction/unconditional branch", "Unkown unconditional branch op code");
            error();
    }

    printDebugMessageInt("to", pc);
}

void error(){
    has_error = 1;
    running = 0;
    printDebugMessage("An error happend, stopping.");
}