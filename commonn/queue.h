#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct _queue_t
{
    int cap;
    int size;
    int rear;
    int front;
    int *array;
};
typedef struct _queue_t queue_t;

int is_empty_q(queue_t *q);

int is_full_q(queue_t *q);

int empty_q(queue_t *q);

int create_q(queue_t *q, int qcap);

int push_q(queue_t *q, int val);

int pop_q(queue_t *q, int *val);

int destroy_q(queue_t *q);

#endif
