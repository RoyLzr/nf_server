#ifndef  __RAPOOL_H_
#define  __RAPOOL_H_

#include "nf_server_core.h"

#define LISTENER_PRIORITY    10
#define WORKER_PRIORITY        5 

typedef struct _rapool_sock_item_t rapool_sock_item_t;
typedef struct _rapool_t rapool_t;

struct _rapool_sock_item_t
{
    int status;
    //int epoll_staus;
    int sock;
    time_t last_active;
    struct sockaddr_in addr;
    
    rio_t rp;
};

struct _rapool_t
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

    pthread_mutex_t ready_mutex;
    pthread_cond_t  ready_cond;

    pthread_t main;
};

/**
 * @brief 
 *
 * @return  int 
 * @retval   
 * @see 
 * @author Liu ZhaoRui
**/
int rapool_init(nf_server_t *);

int rapool_run(nf_server_t *);

int rapool_join(nf_server_t *);

int rapool_destroy(nf_server_t *);

long long rapool_get_queuenum(nf_server_t *);

void * rapool_main(void *);

void * rapool_workers(void *);

int rapool_produce(nf_server_t *sev, struct sockaddr *addr, 
                   socklen_t *addrlen, int work_reactor);

int rapool_reactor(rapool_t *pool, nf_server_pdata_t *data);

int rapool_check_timeout(nf_server_t *sev);

int rapool_add(nf_server_t *sev, int sock, 
               struct sockaddr_in * addr);

int rapool_del(nf_server_t *sev, int idx, 
               int keep_alive, bool remove=false);

int rapool_epoll_add_read(nf_server_t *sev, 
                          int idx, 
                          int work_reactor);

int rapool_epoll_del(nf_server_t *sev, int idx, int id);

int rapool_pthread_cond_timewait(rapool_t *pool);

#endif  

