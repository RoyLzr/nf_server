#include "memCache.h"

Allocate :: obj * Allocate :: free_list[LISTS] = 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

pthread_mutex_t Allocate :: mem_mutex[LISTS];

void 
Allocate :: init()
{
    int index;
    obj * tmp;
    obj ** my_free_list;

    for(int i = 0; i < LISTS; i++)
        pthread_mutex_init(&mem_mutex[i], NULL);
    for(int i = ALIGN; i <= MAX_BYTES; i += ALIGN)
    {
        tmp = (obj *) refill(i);
        index = FREELIST_INDEX(i);
        my_free_list = free_list + index;
        tmp->free_list_link = *my_free_list;
        *my_free_list = tmp;
    }
}

void * 
Allocate :: allocate(size_t n)
{
    if(n <= 0)
        return NULL;
    obj ** my_free_list;
    obj * result;
    if(n > MAX_BYTES)
        std:: cout << n << std::endl;
    n = ROUND_UP(n);
    int index = FREELIST_INDEX(n);
    if(n > MAX_BYTES)
    {
        std:: cout << n <<" : malloc space "<<std::endl;
        return malloc(n);
    }

    pthread_mutex_lock(&(mem_mutex[index]));
    my_free_list = free_list + index;
    result = *my_free_list;
    if(result == 0)
    {
        std:: cout << n << " : expand list space "<<std::endl;
        void * r = refill(n);
        
        pthread_mutex_unlock(&mem_mutex[index]);
        
        return r;
    }
    *my_free_list = result->free_list_link;
    pthread_mutex_unlock(&mem_mutex[index]);

    return result;
}

void * 
Allocate :: refill(size_t n)
{
    if(n <= 0)
        return NULL;

    char * chunk = (char *) malloc(CHUNK_NODE * n);
    if(chunk == NULL)
        return NULL;

    obj ** my_free_list;
    obj * result;
    obj * curr_obj, * next_obj;
    
    my_free_list = free_list + FREELIST_INDEX(n); 
    
    result = (obj *) chunk;
    next_obj = (obj *) (chunk + n);
    *my_free_list = next_obj;
    
    for(int i = 1; ; i++)
    {
        curr_obj = next_obj;
        next_obj = (obj *)((char *)next_obj + n);
        if(CHUNK_NODE - 1 == i)
        {
            curr_obj->free_list_link = 0;
            break;
        }
        else
        {
            curr_obj->free_list_link = next_obj;
        }
    } 
    return result;
}


void  
Allocate :: deallocate(void * p, size_t n)
{
    if(n <= 0 || p == NULL)
        return;

    obj * q = (obj *) p;
    obj ** my_free_list;
    
    n = ROUND_UP(n);
    if(n > MAX_BYTES)
    {
        Log :: DEBUG("FREE MALLOC SPACE %d tid : %lu ", n, pthread_self()); 
        free(p);
        p = NULL;
        return;
    }
    int index = FREELIST_INDEX(n);

    pthread_mutex_lock(&mem_mutex[index]);
    my_free_list = free_list + index;
    q->free_list_link = *my_free_list;
    *my_free_list = q;
    pthread_mutex_unlock(&mem_mutex[index]);
   
    Log :: DEBUG("FREE LIST SPACE  size = %d index = %d tid = %lu",n, index, pthread_self()); 
    return;
}


