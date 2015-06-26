#ifndef _MEM_CACHE_
#define _MEM_CACHE_

#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "asynLog.h"

enum {ALIGN = 8};
enum {MAX_BYTES = 128};
enum {LISTS = MAX_BYTES/ALIGN};
enum {CHUNK_NODE = 20};

class Allocate
{
    private:
    static size_t ROUND_UP(size_t n)
    {
        return ((n + ALIGN -1)/(ALIGN))*ALIGN ;
    }
 
    union obj
    {
        union obj * free_list_link;
        char client_data[1];
    };

    static obj * free_list[LISTS];

    static size_t FREELIST_INDEX(size_t n)
    {
        return (n + ALIGN-1)/ALIGN - 1;
    }
    
    static void * refill(size_t n);
    
    public:

    static void * allocate(size_t n);
    static void  deallocate(void * p, size_t n);   
    static pthread_mutex_t mem_mutex[LISTS];
    static void init();
};


#endif

