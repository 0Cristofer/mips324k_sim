/* Mips32 4K simulator linked list implementation file
   Authors: Cristofer Oswald
   Created: 22/05/2019
   Edited: 27/05/2019 */

#include <stdlib.h>

#include "include/linked_list.h"

linked_list_t *insertElement(linked_list_t *list, instruction_data_t *data) {
    linked_list_t *l;

    l = malloc(sizeof(linked_list_t));
    l->data = data;
    l->next = list;

    return l;
}

linked_list_t *removeElement(linked_list_t *list) {
    linked_list_t *l;

    l = list->next;

    free(list);

    return l;
}

void clearList(linked_list_t *list) {
    linked_list_t *l;

    while (list) {
        l = list;
        list = list->next;
        removeElement(l);
    }
}