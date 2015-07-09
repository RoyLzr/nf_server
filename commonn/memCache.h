//**********************************************************
//          Net Utility 1.0
//
//  Description:
//  tiny memory cache. 支持 16个级别的 内存大小块分配
//  server 动态内存 通过内存池使用
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef _MEM_CACHE_
#define _MEM_CACHE_

#include <pthread.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "asynLog.h"

enum {ALIGN = 64};
enum {MAX_BYTES = 1024};
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

