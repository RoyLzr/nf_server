#ifndef _BUFFER_
#define _BUFFER_

#include "commonn/memCache.h"

class Buffer
{
    public:
        explicit Buffer() : cache(NULL),
                            allo_len(0),
                            str_idx(0),
                            end_idx(-1)
        {}
        explicit Buffer(int size)
        {
            init(size);
        }
        void init(int size)
        {
            cache = Allocate :: allocate(size);
            allo_len = size;
            str_idx = 0;
            end_idx = -1;
        }

        void clear()
        {
            Allocate :: deallocate(cache, allo_len);
            cache = NULL;
            allo_len = 0;
            str_idx = 0;
            end_idx = -1;
        }
        
        Buffer & operator=(Buffer & bf)
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

        bool isEmpty()
        {
            if(cache == NULL)
                return true;
            return false;
        }
        
        int add_data(void * tmp,
                     int len)
        {
            if(len > get_rmind_cache())
            {
                assert(false);
            }
            memcpy((char *)cache + end_idx + 1, (char *)tmp, len);
            end_idx += len;
            return len;
        }

        void * get_cache()
        {
            return cache;
        }

        int get_rmind_cache()
        {
            return allo_len - end_idx - 1;
        }

        int get_handle_num()
        {
            return end_idx - str_idx + 1;
        }
        void * get_handle_cache()
        {
            return (char *)cache + str_idx;
        }

        virtual ~Buffer()
        {
            clear();
        }

    private:
        Buffer(Buffer & bf)
        {
            assert(false);
        }
        void * cache;
        int allo_len;
        int str_idx;
        int end_idx;
};

#endif
