#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct _queue_t
{
    int cap;
    int size;
    int rear;
    int front;
    int *array;
} queue_t;


int is_empty_q(queue_t *q)
{
    return (NULL == q->array || 0 == q->size) ? 1 : 0;
}

int is_full_q(queue_t *q)
{
    return (NULL == q->array || q->size >= q->cap) ? 1 : 0;
}

int empty_q(queue_t *q)
{
    q->size = 0;
    q->rear = 1;
    q->front = 0;
}

int create_q(queue_t *q, int qcap)
{
    ++ qcap;
    if (qcap < 2) 
        return -1;
    q->cap = qcap;
    empty_q(q);
    q->array = (int *)malloc(sizeof(int) * qcap);
    if (q->array == NULL) 
    {
        return -1;
    }
    return 0;
}

int push_q(queue_t *q, int val)
{
    if (is_full_q(q)) 
        return -1;
    q->array[q->rear] = val;
    ++ q->size;
    if (++ q->rear >= q->cap) 
        q->rear = 0;
    return 0;
}

int pop_q(queue_t *q, int *val)
{
    if (is_empty_q(q)) 
        return -1;
    ++ q->front;
    if (q->front >= q->cap) 
        q->front = 0;
    *val = q->array[q->front];
    -- q->size;
    return 0;
}

int destroy_q(queue_t *q)
{
    q->cap = 0;
    free(q->array);
    return 0;
}

#endif
