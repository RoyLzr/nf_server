#ifndef _Lf_SERVER_H
#define _Lf_SERVER_H

#include <sys/epoll.h>
#include <pthread.h>
#include <string>
#include "nf_server_core.h"
#include "nf_server_app.h"
#include "nf_server.h"
#include <iostream>
#include <string.h>

typedef struct _lfpool_t 
{   
    pthread_mutex_t lock;
} lfpool_t;

class LfServer : public NfServer
{
    public:
        LfServer(){};

        virtual ~LfServer(){};

        virtual int svr_init();

        virtual int svr_run();

        virtual int svr_join();

        virtual int svr_listen();

        virtual int svr_destroy();

        virtual int svr_pause();

        virtual int svr_resume();

        virtual int svr_set_stragy( BaseWork *);

        static void * lf_main(void *);
         
        static int lfpool_once_op(int, int, int);

    protected:
        static lfpool_t * lf_pool;        
};

#endif
