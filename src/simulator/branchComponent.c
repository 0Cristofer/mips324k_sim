/* Mips32 4K simulator branch component implementation
   Authors: Cristofer Oswald
   Created: 17/04/2019
   Edited: 17/04/2019 */


#include "src/simulator/include/branchComponent.h"
#include "include/simulator.h"
#include "include/util.h"

int branchComponent(){
    unsigned int instruction, op_type, op_code, next_pc = 0;

    instruction = instruction_queue.head->data.instruction;

    switch (instruction >> 26){
        case OP_DECODE_0:
            op_type = SPECIAL;
            op_code =  instruction & (unsigned int)63;

            if(op_code == JR) op_code;
            else return 0;

            break;

        case OP_DECODE_1:
            op_type = REGIMM;

            op_code = ((instruction >> 16) & (unsigned int)31);

            break;

        case OP_DECODE_2:
            return 0;

        default:
            op_type = NONE;

            op_code = instruction >> (unsigned int)26;

            break;
    }
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