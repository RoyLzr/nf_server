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
    pthread_t main;
};


/**
 * @brief   init connect pool 
 * @author  Liu ZhaoRui
**/
int 
rapool_init(nf_server_t *);

/**
 * @brief   init thread run
 * @author  Liu ZhaoRui
**/
int 
rapool_run(nf_server_t *);

/**
 * @brief   wait thread end
 * @author  Liu ZhaoRui
**/
int 
rapool_join(nf_server_t *);

/**
 * @brief   destroy connection pool
 * @author  Liu ZhaoRui
**/
int 
rapool_destroy(nf_server_t *);

/**
 * @brief   no use
 * @author  Liu ZhaoRui
**/
long long 
rapool_get_queuenum(nf_server_t *);

/**
 * @brief   listen/Load balance thread
 * @author  Liu ZhaoRui
**/
void * 
rapool_main(void *);


/**
 * @brief   reactor thread
 * @author  Liu ZhaoRui
**/
void * 
rapool_workers(void *);

/**
 * @brief   listen function/add accepted 
 *          fd to reactor
 * @author  Liu ZhaoRui
**/
int 
rapool_produce(nf_server_t *sev, 
               struct sockaddr *addr, 
               socklen_t *addrlen, 
               int work_reactor);

/**
 * @brief   reactor work fun 
 * @author  Liu ZhaoRui
**/
int 
rapool_reactor(rapool_t *pool, 
               nf_server_pdata_t *data);

/**
 * @brief   rapool use timer, no use
 * @author  Liu ZhaoRui
**/
int 
rapool_check_timeout(nf_server_t *sev);

/**
 * @brief   add sock to connection pool
 * @author  Liu ZhaoRui
**/
int 
rapool_add(nf_server_t *sev, 
           int sock, 
           struct sockaddr_in * addr);

/**
 * @brief   del sock from connection pool and reactor
 * @author  Liu ZhaoRui
**/
int 
rapool_del(nf_server_t *sev, 
           int idx, 
           int keep_alive, 
           bool remove=false);

/**
 * @brief   add sock to reactor
 * @author  Liu ZhaoRui
**/
int 
rapool_epoll_add_read(nf_server_t *sev, 
                      int idx, 
                      int work_reactor);

/**
 * @brief   mod sock reactor
 * @author  Liu ZhaoRui
**/
int 
rapool_epoll_mod_read(nf_server_t *sev, 
                      int idx, 
                      int work_reactor);

/**
 * @brief   del sock reacctor
 * @author  Liu ZhaoRui
**/
int 
rapool_epoll_del(nf_server_t *sev, 
                 int idx, 
                 int id);

/**
 * @brief   mod write sock reactor 
 * @author  Liu ZhaoRui
**/
int 
rapool_epoll_mod_write(nf_server_t *sev, 
                       int idx, 
                       int work_reactor);

/**
 * @brief   定时器使用的，超时后回调函数
 * @author  Liu ZhaoRui
**/
int 
call_back_timeout(void * param);


int
rapool_set_stratgy(nf_server_t *, BaseWork *);


#endif  

