#include <stdlib.h>
#include <stdio.h>

#define TRUE  1
#define FALSE 0

typedef struct {
    int TaskID;
    int GiftID;
    char TaskType;
    char Responsible;
    int completed;
    int requestTime;
    int taskArrival;
    int TT;
} Task;

typedef struct Gift {
    int GiftID;
    int GiftType;
    Task *paintingTask;
    Task *packagingTask;
    Task *assemblyTask;
    Task *qaTask;
    Task *deliveryTask;
} Gift;


/* a link in the queue, holds the data and point to the next Node */
typedef struct Node_t {
    Task data;
    struct Node_t *prev;
} NODE;

/* the HEAD of the Queue, hold the amount of node's that are in the queue */
typedef struct Queue {
    NODE *head;
    NODE *tail;
    int size;
    int limit;
} Queue;

Queue *ConstructQueue(int limit);
void DestructQueue(Queue *queue);
int Enqueue(Queue *pQueue, Task t);
Task Dequeue(Queue *pQueue);
int isEmpty(Queue* pQueue);

Queue *ConstructQueue(int limit) {
    Queue *queue = (Queue*) malloc(sizeof (Queue));
    if (queue == NULL) {
        return NULL;
    }
    if (limit <= 0) {
        limit = 65535;
    }
    queue->limit = limit;
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void DestructQueue(Queue *queue) {
    NODE *pN;
    while (!isEmpty(queue)) {
        Dequeue(queue);
    }
    free(queue);
}

int Enqueue(Queue *pQueue, Task t) {
    /* Bad parameter */
    NODE* item = (NODE*) malloc(sizeof (NODE));
    item->data = t;

    if ((pQueue == NULL) || (item == NULL)) {
        return FALSE;
    }
    // if(pQueue->limit != 0)
    if (pQueue->size >= pQueue->limit) {
        return FALSE;
    }
    /*the queue is empty*/
    item->prev = NULL;
    if (pQueue->size == 0) {
        pQueue->head = item;
        pQueue->tail = item;

    } else {
        /*adding item to the end of the queue*/
        pQueue->tail->prev = item;
        pQueue->tail = item;
    }
    pQueue->size++;
    return TRUE;
}

Task Dequeue(Queue *pQueue) {
    /*the queue is empty or bad param*/
    NODE *item;
    Task ret;
    if (isEmpty(pQueue))
        return ret;
    item = pQueue->head;
    pQueue->head = (pQueue->head)->prev;
    pQueue->size--;
    ret = item->data;
    free(item);
    return ret;
}

int isEmpty(Queue* pQueue) {
    if (pQueue == NULL) {
        return FALSE;
    }
    if (pQueue->size == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}
