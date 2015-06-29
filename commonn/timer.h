#ifndef _TIMER_
#define _TIMER_

#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <sys/time.h>
#include <utility>

using std::multimap;
using std::pair;
using std::make_pair;

class Timer
{
    public:
        typedef int (* timer_callback_proc)(void * param);
        
        long long add_timer_ms(long long time, 
                          timer_callback_proc proc, 
                          void * param);

        void del_timer_ms(long long time, 
                          void *);

        void expire_timer_ms();

        long long top_timer_ms();
    
    private:
        multimap<long long, pair<timer_callback_proc, void *> > timer;
        long long get_time_usec(struct timeval * );
        long long get_time_msec(struct timeval * );

};

#endif
