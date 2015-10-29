#ifndef _BUFFER_
#define _BUFFER_

#include "commonn/memCache.h"
#include "net.h"

static const bool DELETED = true;
static const bool UNDELETED = false;

#ifdef WORK
static const bool FRESHLIMIT = 1024;
#else
static const bool FRESHLIMIT = 8;
#endif


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

        void init(int size);

        void clear();
        
        Buffer & operator=(Buffer & bf);

        bool isEmpty()
        {
            if(cache == NULL)
                return true;
            return false;
        }
        
        int add_data(void * tmp, int len);

        int add_handl_num(int handled);

        void * get_cache()
        {
            return cache;
        }

        int get_rmind_cache()
        {
            return allo_len - end_idx - 1;
        }

        int get_unhandle_num()
        {
            return end_idx - str_idx + 1;
        }

        void * get_unhandle_cache()
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

        int fresh_cache(int len);

        void * cache;
        int allo_len;
        int str_idx;
        int end_idx;
};

#endif
