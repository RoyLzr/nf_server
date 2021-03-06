#include "Buffer.h"

void Buffer :: init(int size)
{
    cache = Allocate :: allocate(size + 1);
    if(cache == NULL)
        throw "Init Buffer Error";
    allo_len = Allocate::ROUND_UP(size+1);
    str_idx = 0;
    end_idx = -1;
}

void Buffer :: clear()
{
    if(cache != NULL)
    {
        Allocate :: deallocate(cache, allo_len);
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
    if(cache == NULL)
        init(len);
    if(len > get_empty_size())
        fresh_cache(len);

    memcpy((char *)cache + end_idx + 1, (char *)tmp, len);
    end_idx += len;
    return len;
}

int Buffer :: check_empty_space()
{
    char * tt = (char *)cache;
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
    return str_idx;
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
        if(allo_len > FRESHLIMIT)
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
    int new_len = allo_len*2 > (end_idx + len + 1) ? 2*allo_len : (end_idx + len +1);
    char * tmp = (char *) Allocate :: allocate(new_len + 1);

    memcpy((char *)tmp, (char *)cache, end_idx + 1);

    Allocate :: deallocate(cache, allo_len);
    cache = tmp;
    allo_len = Allocate::ROUND_UP(new_len + 1);
    return 0;
}



