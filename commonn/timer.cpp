#include "timer.h"

long long 
Timer :: add_timer_ms(long long time, 
                      timer_callback_proc proc, 
                      void * param)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int msec = get_time_msec(&tv);
    msec += time;
    timer.insert(make_pair(msec, make_pair(proc, param)));
    return msec;
}

void
Timer :: del_timer_ms(long long time, const void * const param)
{
    
    multimap<long long, pair<timer_callback_proc, void *> > :: iterator iter = timer.find(time);
    int num = timer.count(time);
    //printf("%lld   %lld\n", time, timer.begin()->first);
    for(int i = 0; i != num; i++, iter++)
    {
        if((iter->second).second == param)
        {
            timer.erase(iter);
            break;
        }
    }
    //printf("timersize = %d\n", timer.size());
    return;
}

long long
Timer :: get_time_msec(const struct timeval * tv)
{
    long long time = 0;
    time += tv->tv_sec * 1000;
    time += tv->tv_usec /1000;
    return time;
}

long long
Timer :: top_timer_ms()
{
    if(timer.empty())
        return 0;

    long long time = (timer.begin())->first;
    //printf("begin = %lld\n", time);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int msec = get_time_msec(&tv);
    return time - msec; 
}

void 
Timer :: expire_timer_ms()
{
    multimap<long long, pair<timer_callback_proc, void *> > :: iterator iter = timer.begin();
    
    multimap<long long, pair<timer_callback_proc, void *> > :: iterator del_iter = timer.begin();

    struct timeval tv;
    gettimeofday(&tv, NULL);
    int msec = get_time_msec(&tv);
    if(timer.size() <= 0)
        return;
 
    for(; iter != timer.end(); iter ++)
    {
        if(msec >= iter->first)
        {
            (iter->second).first((iter->second).second);
        }
        else
            break;
    }
    for(; msec >= del_iter->first; )
    {
        timer.erase(del_iter);  
        del_iter = timer.begin();      
    }
    return;
}

