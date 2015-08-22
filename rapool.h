//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  reactor + multi-thread model
//  reactor model 每个线程维护一个自己的reactor,实现模型内部
//                无锁化
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef  __RAPOOL_H_
#define  __RAPOOL_H_

#include "nf_server_core.h"
#include "nf_server_app.h"
#include "nf_server.h"

#define LISTENER_PRIORITY    10
#define WORKER_PRIORITY        5 

typedef struct _rapool_sock_item_t rapool_sock_item_t;
typedef struct _rapool_t rapool_t;

//连接池 单元
struct _rapool_sock_item_t
{
    int status;
    //int epoll_staus;
    int sock;
    struct sockaddr_in addr;
    
    long long sock_timeout;    
    rio_t rp;
};

//连接池 数据结构
typedef struct _rapool_t
{
    rapool_sock_item_t * sockets;
    struct epoll_event * ep_events;
    int size;
    long long using_size;

    int epfd;

    int timeout;        //epoll的超时时间
    int check_interval;    //微妙级别支持超时
    time_t next_check_time;

    int * run;
    int sev_sock_id;
    pthread_t main;
}rapool_t;


class RaServer : public NfServer
{
    public:
        RaServer(){};

        virtual ~RaServer(){};

        virtual int svr_init(nf_server_t *);

        virtual int svr_run(nf_server_t *);

        virtual int svr_join(nf_server_t *);

        virtual int svr_listen(nf_server_t *);

        virtual int svr_destroy(nf_server_t *);

        virtual int svr_pause(nf_server_t *);

        virtual int svr_resume(nf_server_t *);

        virtual int svr_set_stragy(nf_server_t *, BaseWork *);

        static int 
        rapool_init(nf_server_t *);

        static int 
        rapool_run(nf_server_t *);

        static int 
        rapool_join(nf_server_t *);

        static int 
        rapool_destroy(nf_server_t *);

        static void * 
        rapool_main(void *);

        static void * 
        rapool_workers(void *);

        static int 
        rapool_produce(nf_server_t *sev, 
                       struct sockaddr *addr, 
                       socklen_t *addrlen, 
                       int work_reactor);

        static int 
        rapool_reactor(rapool_t *pool, 
                       nf_server_pdata_t *data);

        static int 
        rapool_check_timeout(nf_server_t *sev);

        static int 
        add_listen_socket(nf_server_t *sev, 
                          int listenfd);

        static int 
        rapool_add(nf_server_t *sev, 
                   int sock, 
                   struct sockaddr_in * addr);

        static int 
        rapool_del(nf_server_t *sev, 
                   int idx, 
                   int keep_alive, 
                   bool remove=false);

        static int 
        rapool_epoll_add_read(nf_server_t *sev, 
                              int idx, 
                              int work_reactor);

        static int 
        rapool_epoll_mod_read(nf_server_t *sev, 
                              int idx, 
                              int work_reactor);

        static int 
        rapool_epoll_del(nf_server_t *sev, 
                         int idx, 
                         int id);

        static int 
        rapool_epoll_mod_write(nf_server_t *sev, 
                               int idx, 
                               int work_reactor);

        static int 
        call_back_timeout(void * param);

        static void 
        rapool_close_pool_sockets(nf_server_t *, bool);

    protected:
       rapool_t * ra_pool; 

};


#endif
