/* Mips32 4K simulator branch component implementation
   Authors: Cristofer Oswald
   Created: 17/04/2019
   Edited: 17/04/2019 */


#include "src/simulator/include/branchComponent.h"
#include "include/simulator.h"
#include "include/util.h"

/* Adicionar prints de debug em PDs */
int branchComponent(int pc) {
    unsigned int instruction, op_code, offset, rt, rs;
    int next_pc = 1;
    uint16_t immediate;

    instruction = instruction_queue.tail->data.instruction;

    switch (instruction >> 26){
        case OP_DECODE_0:
        case OP_DECODE_2:
            break;

        case OP_DECODE_1:
            offset =  instruction & (unsigned int)65535;

            next_pc = branchPredictor((uint16_t) offset);

            break;

        default:
            op_code = instruction >> 26;

            switch (op_code){
                case ADDI:
                case ANDI:
                case LUI:
                case ORI:
                case XORI:
                    break;

                case J:
                    printDebugMessageInt("inconditional jump from", pc);

                    offset = instruction & (unsigned int)67108863;

                    popQueue(&instruction_queue);
                    next_pc = offset - pc;

                    printDebugMessageInt("to", offset);

                    break;

                case B:

                    immediate = (uint16_t)(instruction & (unsigned int)65535);

                    rt = (instruction >> 16) & (unsigned int)31;
                    rs = (instruction >> 21) & (unsigned int)31;

                    if((rt == 0) && (rs == 0)){
                        printDebugMessageInt("inconditional branch from", pc);

                        next_pc = (int16_t)immediate;
                        popQueue(&instruction_queue);

                        printDebugMessageInt("to", pc+(int16_t)immediate);
                    }
                    else{
                        next_pc = branchPredictor(immediate);
                    }

                    break;

                default:
                    immediate = (uint16_t)(instruction & (unsigned int)65535);

                    next_pc = branchPredictor(immediate);

                    break;
            }

            break;
    }

    return next_pc;
}

int branchPredictor(uint16_t offset) {
    offset = offset & ((uint16_t)1023);

    /*two_bit_t bht[BRENCH_PRED_SIZE];

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
    }*/

    return 1;
}