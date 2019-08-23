/* Mips32 4K simulator branch component implementation
   Authors: Cristofer Oswald
   Created: 17/04/2019
   Edited: 20/08/2019 */

#include <stdlib.h>

#include "include/branchComponent.h"
#include "include/simulator.h"
#include "include/util.h"
#include "../helpers/include/linked_list.h"

two_bit_t bht[BRENCH_PRED_SIZE];
instruction_data_t *current_branch_inst = NULL;
int total_jumps = 0;

void initBranchPredictor() {
    int i;

    for (i = 0; i < BRENCH_PRED_SIZE; i++) {
        bht[i].is_new = 1;
    }
}

/**
 * Two bit branch predictor. Uses a branch history table indexed by the 10 first bits of the branch target. Each
 * entry is a satured counter, from 0 (strongly not taken) to 3 (strongly taken)
 * @param offset The index of the BHT to be read
 * @return The prediction
 */
int twoBitPred(uint16_t offset) {
    uint16_t index = offset & ((uint16_t) 1023);

    if (((int16_t) offset) < 0) {
        if (bht[index].is_new) {
            bht[index].is_new = 0;
            bht[index].pred = 3;
        }
    } else {
        if (bht[index].is_new) {
            bht[index].is_new = 0;
            bht[index].pred = 0;
        }
    }

    return bht[index].pred;
}

/**
 * Reads the first 10 bits of the offset and uses it as an index for the branch predictor
 */
int branchPredictor(uint16_t offset, instruction_data_t *inst) {
    current_branch_inst = inst;

    total_jumps++;

    printDebugMessageInt("Predicting branch", inst->pc);

    if (twoBitPred(offset) > 1) {
        printDebugMessageInt("Branch taken to ", inst->pc + offset);
        inst->taken = 1;
        return (int16_t) offset;
    }
    printDebugMessage("Branch not taken");
    inst->taken = 0;

    return 1;
}

void updateBht(uint16_t index, unsigned int b) {
    index = index & ((uint16_t) 1023);

    if (b) {
        if (bht[index].pred != 3) bht[index].pred = bht[index].pred + 1;
    } else {
        if (bht[index].pred != 0) bht[index].pred = bht[index].pred - 1;
    }
}

int branchComponent(int current_pc) {
    unsigned int instruction, op_code, offset, rt, rs;
    int next_pc = 1;
    uint16_t immediate;
    instruction_data_t *inst_data;

    inst_data = instruction_queue.tail->data.instruction;
    instruction = inst_data->instruction;

    switch (instruction >> (unsigned int) 26) {
        case OP_DECODE_0:
            op_code = instruction & (unsigned int) 63;

            if (op_code == SYSCALL) { // If the instruction is a SYSCALL set PC to the end of program to stop fetching
                popLastQueue(&instruction_queue);
                return num_instructions - current_pc;
            }

        case OP_DECODE_2:
            addSpeculative(inst_data);

            break;

        case OP_DECODE_1:
            addSpeculative(inst_data);

            offset = instruction & (unsigned int) 65535;

            next_pc = branchPredictor((uint16_t) offset, inst_data);

            break;

        default:
            op_code = instruction >> (unsigned int) 26;

            switch (op_code) {
                case ADDI:
                case ANDI:
                case LUI:
                case ORI:
                case XORI:
                    addSpeculative(inst_data);

                    break;

                case J:
                    printDebugMessageInt("inconditional jump from", current_pc);

                    offset = instruction & (unsigned int) 67108863;

                    free(instruction_queue.tail->data.instruction);
                    popLastQueue(&instruction_queue);
                    next_pc = offset - current_pc;

                    printDebugMessageInt("to", offset);

                    break;

                case B:

                    immediate = (uint16_t) (instruction & (unsigned int) 65535);

                    rt = (instruction >> (unsigned int) 16) & (unsigned int) 31;
                    rs = (instruction >> (unsigned int) 21) & (unsigned int) 31;

                    if ((rt == 0) && (rs == 0)) {
                        printDebugMessageInt("inconditional branch from", current_pc);

                        free(instruction_queue.tail->data.instruction);
                        popLastQueue(&instruction_queue);
                        next_pc = (int16_t) immediate;

                        printDebugMessageInt("to", current_pc + (int16_t) immediate);
                    } else {
                        addSpeculative(inst_data);
                        next_pc = branchPredictor(immediate, inst_data);
                    }

                    break;

                default:
                    addSpeculative(inst_data);

                    immediate = (uint16_t) (instruction & (unsigned int) 65535);

                    next_pc = branchPredictor(immediate, inst_data);

                    break;
            }

            break;
    }

    return next_pc;
}

void addSpeculative(instruction_data_t *inst_data) {
    if (current_branch_inst) {
        printDebugMessageInt("Adding speculative inst:", inst_data->pc);
        inst_data->is_speculate = 1;

        if (current_branch_inst->speculative_insts) {
            current_branch_inst->speculative_insts = insertElement(current_branch_inst->speculative_insts, inst_data);
        } else {
            current_branch_inst->speculative_insts = malloc(sizeof(linked_list_t));
            current_branch_inst->speculative_insts->data = inst_data;
            current_branch_inst->speculative_insts->next = NULL;
        }
    }
}
