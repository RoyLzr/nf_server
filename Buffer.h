#ifndef _BUFFER_
#define _BUFFER_

#include "commonn/memCache.h"
#include "net.h"


static const bool DELETED = true;
static const bool UNDELETED = false;
static const int  FRESHLIMIT = 1024;


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
        explicit Buffer(const Buffer &);

        void init(int size);

        void clear();
        
        Buffer & operator=(const Buffer & bf);

        bool isEmpty()
        {
            if(cache == NULL)
                return true;
            return false;
        }
        
        int add_data(void * tmp, int len);

        int add_handl_num(int handled);

        int check_empty_space();
        
        inline int get_allo_size() { return allo_len;}

        inline void add_unhandle_num(int size)
        {
            end_idx += size;
        }

        void * get_cache() const
        {
            return cache;
        }

        inline void * get_empty_cache() const
        {
            return (char *)cache + end_idx + 1;
        }

        inline int get_empty_size() const
        {
            return allo_len - end_idx -1;
        }

        inline int get_unhandle_num() const
        {
            return end_idx - str_idx + 1;
        }

        inline void * get_unhandle_cache() const
        {
            return (char *)cache + str_idx;
        }

        virtual ~Buffer()
        {
            clear();
        }
        
        void swap(Buffer &rhs);
        
        int fresh_cache(int len);
        
    protected:
        
        void * cache;
        int allo_len;
        int str_idx;
        int end_idx;
};

#endif
