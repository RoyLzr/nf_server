//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  sync and reactor model
//  
//  listen thread ʹ��reactor model, ������ӳ����������� +
//  listen fd, ����ص����¼����� �¼����� ȫ�̲���������
//
//  work thread ��ȡ�¼��������ݽ��д���read event ȫ�̷�����
//  �����������������㴦���ʽʱ�� �����ڴ�ط���ռ䣬����״̬��
//  �ȴ��´ζ��ĵ������ָ��ϴ�״̬�� д�¼����� ���� + ��ʱ����
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

    int timeout;        //epoll�ĳ�ʱʱ��
    int check_interval;    //΢���֧�ֳ�ʱ
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

