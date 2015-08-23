//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  sync and reactor model
//  
//  listen thread 使用reactor model, 监控连接池内所有连接 +
//  listen fd, 将监控到的事件放入 事件队列 全程操作非阻塞
//
//  work thread 获取事件队列数据进行处理，read event 全程非阻塞
//  ，读到数据量不满足处理格式时， 利用内存池分配空间，保存状态，
//  等待下次读的到来，恢复上次状态。 写事件采用 阻塞 + 超时处理
//           
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************
#ifndef  __SAPOOL_H_
#define  __SAPOOL_H_

#include "nf_server_core.h"
#include "commonn/queue.h"
#include "nf_server_app.h"
#include "nf_server.h"

#define LISTENER_PRIORITY    10
#define WORKER_PRIORITY        5 

typedef struct _sapool_sock_item_t sapool_sock_item_t;
typedef struct _sapool_t sapool_t;

struct _sapool_sock_item_t
{
    int status;
    //int epoll_staus;
    int sock;
    time_t last_active;
    struct sockaddr_in addr;
    
    rio_t rp;
};

struct _sapool_t
{
    sapool_sock_item_t * sockets;
    struct epoll_event * ep_events;
    int size;
    long long using_size;

    int epfd;

    int timeout;        //epoll的超时时间
    int check_interval;    //微妙级别支持超时
    time_t next_check_time;

    int * run;
    int sev_sock_id;

    queue_t queue;
    pthread_mutex_t ready_mutex;
    pthread_cond_t  ready_cond;

    pthread_t main;
};


class SaServer : public NfServer
{
    public:
        SaServer(){};

        virtual ~SaServer(){};

        virtual int svr_init();

        virtual int svr_run();

        virtual int svr_join();

        virtual int svr_listen();

        virtual int svr_destroy();

        virtual int svr_pause();

        virtual int svr_resume();

        virtual int svr_set_stragy(BaseWork *);

        static long long sapool_get_queuenum(nf_server_t *);

        static void * sapool_main(void *);
        
        static void * sapool_workers(void *);       
        
        static int 
        sapool_produce(nf_server_t *sev, struct sockaddr *addr, 
                       socklen_t *addrlen);

        static int 
        sapool_consume(sapool_t *pool, nf_server_pdata_t *data);

        static int 
        sapool_check_timeout(nf_server_t *sev);

        static int 
        sapool_add(nf_server_t *sev, int sock, 
                   struct sockaddr_in * addr);

        static int 
        sapool_del(nf_server_t *sev, int idx, 
                   int keep_alive, bool remove=false);

        static int 
        sapool_epoll_add(nf_server_t *sev, int idx);

        static int 
        sapool_epoll_del(nf_server_t *sev, int idx);

        static int 
        sapool_put(sapool_t *pool, int idx);

        static int 
        sapool_get(nf_server_t *sev, int *idx);

        static int 
        sapool_pthread_cond_timewait(sapool_t *pool);

        static int 
        check_socket_queue(nf_server_t *sev);
        
        static int 
        add_listen_socket(nf_server_t *, int);
        
        static void 
        sapool_close_pool_sockets(nf_server_t *, bool );

    protected:
        static sapool_t * sa_pool;        
};

#endif  

