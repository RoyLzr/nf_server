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

        virtual int svr_init(nf_server_t *);

        virtual int svr_run(nf_server_t *);

        virtual int svr_join(nf_server_t *);

        virtual int svr_listen(nf_server_t *);

        virtual int svr_destroy(nf_server_t *);

        virtual int svr_pause(nf_server_t *);

        virtual int svr_resume(nf_server_t *);

        virtual int svr_set_stragy(nf_server_t *, BaseWork *);

        static void * lf_main(void *);
         
        static int lfpool_once_op(int, int, int);

    protected:
        lfpool_t * lf_pool;        
};

#endif
