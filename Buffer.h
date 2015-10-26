#include "commonn/memCache.h"

class Buffer
{
    public:
        explicit Buffer() : cache(NULL),
                            allo_len(0),
                            used_len(0)
        {}
        explicit Buffer(int size) : allo_len(size)
        {
            cache = Allocate :: allocate(allo_len);
            used_len = 0;
        }

        bool isEmpty()
        {
            if(cache == NULL)
                return true;
            return true;
        }

        virtual ~Buffer()
        {
            Allocate :: deallocate(cache, allo_len);
            cache = NULL;
            allo_len = 0;
            used_len = 0;
        }

        int copyIn()

    private:

        void * cache;
        int allo_len;
        int used_len;
};
