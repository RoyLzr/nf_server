#include "Buffer.h"

void Buffer :: init(int size)
{
    cache = Allocate :: allocate(size + 1);
    
    if(cache == NULL)
        throw "Init Buffer Error";

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

Buffer :: Buffer(const Buffer & rhs)
{
    init(rhs.allo_len);
    allo_len = rhs.allo_len;
    str_idx = rhs.str_idx;
    end_idx = rhs.end_idx;   
    void * rhs_cache = rhs.get_cache(); 
    memcpy(cache, rhs_cache, end_idx); 
}

void Buffer :: swap(Buffer &rhs)
{
    std::swap(str_idx, rhs.str_idx);
    std::swap(end_idx, rhs.end_idx);
    std::swap(allo_len, rhs.allo_len);
    std::swap(cache, rhs.cache);
}

Buffer & Buffer :: operator=(const Buffer & rhs)
{
    Buffer tmp(rhs);
    swap(tmp);
    return *this;
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
    char * tt = (char *)cache;

#ifndef WORK
    Log :: DEBUG("[Buffer] after add handle num str : %d, end : %d",
                  str_idx, end_idx);
#endif

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
    else
    {
        if(allo_len > FRESHLIMIT && str_idx > allo_len/2)
        {
        #ifndef WORK
            tt[end_idx + 1] = '\0';
            Log :: DEBUG("Before move str:%d end:%d", str_idx, end_idx);
        #endif

            move_forward((char *)cache, str_idx, end_idx);
            str_idx = 0;
            end_idx -= str_idx;
            tt[end_idx + 1] = '\0'; 

        #ifndef WORK
            tt[end_idx + 1] = '\0';
            Log :: DEBUG("After move str:%d end:%d", str_idx, end_idx);
        #endif
        } 
    }
    return str_idx;
}


int Buffer :: fresh_cache(int len)
{
    int new_len = allo_len*2 > (end_idx + len) ? 2*allo_len : (end_idx + len);
    char * tmp = (char *) Allocate :: allocate(new_len + 1);
    char * tt = (char *) cache;

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


