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

typedef struct _nf_server_pdata_t
{
    pthread_t pid;
    size_t id;
    struct sockaddr_in client_addr;

    void *read_buf;
    size_t read_siz;
    void *write_buf;
    size_t write_siz;
    
}nf_server_pdata_t;

typedef struct _nf_server_t
{
    size_t server_type;
    size_t connect_type; 
    size_t pthread_num;
    size_t run_thread_num;
    size_t backlog;
    std::string name;
    
    int sock_family;
    
    
    nf_callback_proc cb_work;
    nf_server_pdata_t *pdata;

    nf_handle_t p_start;     
    nf_handle_t p_end;     

    SERVER_STATUS_T status;

}nf_server_t;

enum {
    NFSVR_LFPOOL = 0,    //建议用于多线程短连接                                                                   
    NFSVR_PCPOOL,        //建议用于多线程长连接      
    NFSVR_POOL_NUM,     //当前有多少个pool    
};   

enum{
    NFSVR_SHORT_CONNEC = 0,
    NFSVR_LONG_CONNEC
};

extern nf_server_t * nf_server_create(const std::string * sev_name);








#endif
