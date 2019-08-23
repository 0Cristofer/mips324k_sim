/* Mips32 4K simulator linked list header file
   Authors: Cristofer Oswald
   Created: 22/05/2019
   Edited: 27/05/2019 */

#ifndef MIPS324K_SIM_LINKED_LIST_H
#define MIPS324K_SIM_LINKED_LIST_H

typedef struct linked_list linked_list_t;
typedef struct instruction_data instruction_data_t;

/**
 * Linked list main structure. Elements are always added to the head of the list
 */
struct linked_list {
    instruction_data_t *data;
    linked_list_t *next;
};

/**
 * Adds a element to a given list
 * @param list The list to be modified
 * @param data The data to be added
 * @return A pointer to the list
 */
linked_list_t *insertElement(linked_list_t *list, instruction_data_t *data);

/**
 * Removes the last element added to the list
 * @param list The list to be modified
 * @return A pointer to the list
 */
linked_list_t *removeElement(linked_list_t *list);

/**
 * Clears the list freeing all elements.
 * PS.: The data itself is not freed
 * @param list The list to be cleared
 */
void clearList(linked_list_t *list);

#endif //MIPS324K_SIM_LINKED_LIST_H
