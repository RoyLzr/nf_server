//**********************************************************
//          Net Utility 1.0
//
//  Description:
//  tiny 定时器，内部使用红黑树---map, 保存定时数据
//  支持定时格式
//  超时时间， 函数指针， 函数参数
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

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
