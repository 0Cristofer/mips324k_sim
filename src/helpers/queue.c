/* Mips32 4K simulator queue implementation
   Authors: Cristofer Oswald
   Created: 17/04/2019
   Edited: 30/05/2019 */

#include <stdlib.h>

#include "include/queue.h"

void initQueue(queue_t *queue) {
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
}

void pushQueue(queue_t *queue, queue_data_t qd) {
    queue_element_t *new_element;

    new_element = malloc(sizeof(queue_element_t));

    new_element->data = qd;
    new_element->next = NULL;
    new_element->prev = NULL;

    if (queue->tail != NULL) {
        queue->tail->prev = new_element;
        new_element->next = queue->tail;
    }

    if (queue->head == NULL) queue->head = new_element;
    queue->tail = new_element;
    queue->size = queue->size + 1;
}

queue_data_t popQueue(queue_t *queue) {
    queue_data_t data;
    queue_element_t *next_head;

    data.instruction = 0;

    if (queue->head != NULL) {
        data = queue->head->data;

        next_head = queue->head->prev;

        if (next_head == NULL) queue->tail = NULL;
        else next_head->next = NULL;

        free(queue->head);
        queue->head = next_head;

        queue->size = queue->size - 1;
    }

    return data;
}

queue_data_t popLastQueue(queue_t *queue) {
    queue_data_t data;
    queue_element_t *next_tail;

    data.instruction = 0;

    if (queue->tail != NULL) {
        data = queue->tail->data;

        next_tail = queue->tail->next;

        if (next_tail == NULL) queue->head = NULL;
        else next_tail->prev = NULL;

        free(queue->tail);
        queue->tail = next_tail;

        queue->size = queue->size - 1;
    }

    return data;
}

void clearQueue(queue_t *queue) {
    queue_element_t *current;

    current = queue->tail;

    while (current) {
        queue->tail = current->next;

        free(current);

        current = queue->tail;
    }
}