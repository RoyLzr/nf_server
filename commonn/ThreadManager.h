
#ifndef  _THREADMG_H_
#define  _THREADMG_H_

#include <vector>
#include <pthread.h>
#include "../util.h"

class ThreadManager : private Uncopyable
{
    typedef void * (* cb_t)(void *);
    public:
        
        int run(int threadnum, cb_t callback, void *param);
        
        int join();
        
        inline size_t size() { return _tids.size(); }
        
        ThreadManager() {}
        
        ~ThreadManager() { this->join(); }

    private:

        std::vector<pthread_t> _tids;
};

#endif

