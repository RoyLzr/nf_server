/*
 * brief nf_server data-structure
 * author liuzhaorui
 * email  liuzhaorui1@163.com
 */
#ifndef _NFSERVER_CORE_H
#define _NFSERVER_CORE_H

#include "net.h"
#include "string.h"

typedef enum{

    INIT    = 0,
    RUNNING = 1,
    PAUSE   = 2,
    STOP    = 3
}SERVER_STATUS_T;

typedef int (* nf_callback_proc)(void *req);
typedef int (* nf_handle_t)(void * req);
typedef struct _nf_server_pdata_t nf_server_pdata_t;
typedef struct _nf_server_t nf_server_t;

struct _nf_server_pdata_t
{
    pthread_t pid;
    size_t id;
    struct sockaddr_in client_addr;

    void *read_buf;
    size_t read_size;
    void *write_buf;
    size_t write_size;

    nf_server_t *server;
    int epfd;
    int fd;
    size_t ep_size;
};

struct _nf_server_t
{
    size_t server_type;
    size_t connect_type; 
    size_t pthread_num;        //线程池开启线程总数
    int run_thread_num;     //工作线程数
    size_t backlog;
    size_t listen_port;
    size_t need_join;
    
    size_t connect_to;
    size_t read_to;
    size_t write_to;
    
    size_t thread_read_buf;
    size_t thread_write_buf;
    
    size_t run;     
    size_t stack_size; //线程栈大小
    char name[256];
    
    int sock_family;
    int sev_socket; 
    int epfd;
    
    nf_callback_proc cb_work;
    nf_server_pdata_t *pdata;

    nf_handle_t p_start;     
    nf_handle_t p_end;    
     
    nf_handle_t p_read;     
    nf_handle_t p_write;     
    
    void * pool;
    SERVER_STATUS_T status;

};

enum {
    NFSVR_LFPOOL = 0,    //建议用于多线程短连接                                                                   
    NFSVR_PCPOOL,        //建议用于多线程长连接      
    NFSVR_POOL_NUM,     //当前有多少个pool    
};   

enum{
    NFSVR_SHORT_CONNEC = 0,
    NFSVR_LONG_CONNEC
};

extern nf_server_t * nf_server_create(const char  *);

extern int nf_server_bind(nf_server_t *);

extern int nf_server_init(nf_server_t *);

extern int nf_server_listen(nf_server_t *);

extern int set_sev_socketopt(nf_server_t *, int);

extern int nf_default_worker(void *);

extern int nf_default_write_buf(void *);

extern int nf_default_read_buf(void *);

#endif
