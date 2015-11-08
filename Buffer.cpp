#include "Buffer.h"

void Buffer :: init(int size)
{
    cache = Allocate :: allocate(size + 1);
    allo_len = size;
    str_idx = 0;
    end_idx = -1;
}

void Buffer :: clear()
{
    if(cache != NULL)
    {
        Allocate :: deallocate(cache, allo_len + 1);
        cache = NULL;
        allo_len = 0;
        str_idx = 0;
        end_idx = -1;
    }
}

Buffer & Buffer :: operator=(Buffer & bf)
{
    cache = bf.cache;
    str_idx = bf.str_idx;
    end_idx = bf.end_idx;
    allo_len = bf.allo_len;
    
    bf.cache = NULL;
    bf.str_idx = 0;
    bf.end_idx = -1;
    bf.allo_len = 0; 
}


int Buffer :: add_data(void * tmp,
                       int len)
{
    if(cache == NULL || len > get_rmind_cache())
    {
        if(cache == NULL)
            init(len);
        else
            fresh_cache(len);
    }

    char * tt = (char *) cache;

    memcpy((char *)cache + end_idx + 1, (char *)tmp, len);
    end_idx += len;
    tt[end_idx + 1] = '\0';
    return len;
}

int Buffer :: add_handl_num(int handled)
{
    str_idx += handled;
    if(str_idx >= end_idx)
    {
        if(allo_len > 2 * FRESHLIMIT)
            clear();
        else
        {
            str_idx = 0;
            end_idx = -1;
        }
    }
    return str_idx;
}


int Buffer :: fresh_cache(int len)
{
    int new_len = allo_len*2 > len ? 2*allo_len : len;
    char * tmp = (char *) Allocate :: allocate(new_len + 1);
    char * tt = (char *) cache;
    
    if(allo_len > FRESHLIMIT && str_idx > FRESHLIMIT/2)
    {
    #ifndef WORK
        tt[end_idx + 1] = '\0';
        printf("Before move %s\n", tt);
    #endif

        move_forward((char *)cache, str_idx, end_idx);
        str_idx = 0;
        end_idx -= str_idx;
        tt[end_idx + 1] = '\0'; 

    #ifndef WORK
        tt[end_idx + 1] = '\0';
        printf("After move %s\n", tt);
    #endif
    } 

    memcpy((char *)tmp, (char *)cache, end_idx + 1);

#ifndef WORK
    char * origin = (char *) cache;
    origin[end_idx+1] = '\0';
    tmp[end_idx+1] = '\0';
    printf("Fresh cache orig len : %d end_idx : %d data : %s\n", \
            allo_len, end_idx, origin);
#endif
    
    Allocate :: deallocate(cache, allo_len + 1);
    cache = tmp;
    allo_len = new_len;
    return 0;
}

int Buffer :: get_unhandle_data(void * tmp)
{

    memcpy((char *)tmp, (char *)cache + str_idx, end_idx - str_idx + 1);
    
    add_handl_num(end_idx - str_idx + 1);    
    return end_idx - str_idx + 1;
}


