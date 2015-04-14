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

typedef int (* nf_callback_proc)();
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
    
};

struct _nf_server_t
{
    size_t server_type;
    size_t connect_type; 
    size_t pthread_num;        //线程池开启线程总数
    size_t run_thread_num;     //已经开启线程数
    size_t backlog;
    size_t listen_port;
    size_t need_join;
    
    size_t connect_to;
    size_t read_to;
    size_t write_to;
    
    size_t thread_read_buf;
    size_t thread_write_buf;
     
    size_t stack_size; //线程栈大小
    char name[256];
    
    int sock_family;
    int sev_socket; 
    
    nf_callback_proc cb_work;
    nf_server_pdata_t *pdata;

    nf_handle_t p_start;     
    nf_handle_t p_end;     

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








#endif
