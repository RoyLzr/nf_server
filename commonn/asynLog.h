
//**********************************************************
//          Net Utility 1.0
//
//  Description:
//  异步LOG 写数据,支持 DEBUG NOTICE WARN ERROR
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef _ASYN_LOG_
#define _ASYN_LOG_

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <time.h>
#include <string>
#include <stdarg.h>
#include <pthread.h>

using std::string;
using std::queue;

enum
{
    LOG_DEBUG,
    LOG_NOTICE,
    LOG_WARN, 
    LOG_ERROR,
    LOG_MAXLINE = 6000
};

enum
{
    LOG_INIT,
    LOG_RUN,
    LOG_STOP
};

class Log
{
    private:

    static queue<string> log_buffer;   
    static FILE * fp; 
    static char buffer[LOG_MAXLINE];
    static int STATUS;   
    static int LEVEL;

    static void produce_log(int event, const char * fmt, va_list args); 
    public:

    static int DEBUG(const char * fmt, ...); 
    static int NOTICE(const char * fmt, ...);  
    static int WARN(const char * fmt, ...);  
    static int ERROR(const char * fmt, ...);
    
    static void * write_log(void *);   

    static void set_file(FILE * file)
    {
        fp = file;
    }
    
    static void set_status(int status)
    {
        STATUS = status;
    }
 
    static FILE * get_file()
    {
        return fp;
    }
 
    static void stop()
    {
        STATUS = LOG_STOP;    
    }
    
    static void set_level(int level)
    {
        LEVEL = level;    
    }
 
    static pthread_mutex_t log_mutex;
    static pthread_cond_t  log_cond;
    static int init(const char *);
    static string str_prefix[4];
};


#endif

