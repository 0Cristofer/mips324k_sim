/* Mips32 4K simulator queue header file
   Authors: Cristofer Oswald
   Created: 17/04/2019
   Edited: 17/04/2019 */

#ifndef MIPS324K_SIM_QUEUE_H
#define MIPS324K_SIM_QUEUE_H

union queue_data;
struct queue;
struct queue_element;

typedef union queue_data queue_data_t;
typedef struct queue queue_t;
typedef struct queue_element queue_element_t;

/**
 * Possible data types to be used in the queue.
 */
union queue_data{
    unsigned int instruction;
};

/**
 * Queue head, holds basic information about the hole queue.
 */
struct queue{
    int size;
    queue_element_t *head;
    queue_element_t *tail;
};

/**
 * Queue main structure, holds information of a single queue element.
 */
struct queue_element{
    queue_data_t data;
    queue_element_t* next;
    queue_element_t* prev;
};

/**
 * Initializates a new queue
 * @param queue The queue to be initializated
 */
void initQueue(queue_t *queue);

/**
 * Pushs an element to the queue
 * @param queue The queue to be updated
 * @param qd The data to be held by the queue
 */
void pushQueue(queue_t *queue, queue_data_t qd);

/**
 * Pops a element from the queue, removing it from the queue
 * @param queue The queue to be updated
 * @return The data held by the queue
 */
queue_data_t popQueue(queue_t *queue);

/**
 * Clears a queue, freeing all elements
 * @param qh The queue to be freed
 */
void clearQueue(queue_t *queue);


#endif //MIPS324K_SIM_QUEUE_H