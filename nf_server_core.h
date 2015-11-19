
//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  server/thread 核心数据结构
//
//  核心server调用函数， server 类对该核心库的封装
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef _NFSERVER_CORE_H
#define _NFSERVER_CORE_H


#include "net.h"
#include "commonn/singleton.h"
#include "commonn/configParser.h"
#include "commonn/memCache.h"
#include "reactor.h"


typedef enum
{

    INIT    = 0,
    RUNNING = 1,
    PAUSE   = 2,
    STOP    = 3
}SERVER_STATUS_T;

enum 
{
    NFSVR_SAPOOL,        //建议用于多线程长连接     
    NFSVR_RAPOOL,        //reactor + pthread IO密集，thread 数与核有关
    NFSVR_POOL_NUM,     //当前有多少个pool    
};   

enum
{
    NFSVR_SHORT_CONNEC = 0,
    NFSVR_LONG_CONNEC
};

class NfReactor;

typedef struct _nf_server_t nf_server_t;
typedef struct sockaddr_in nf_server_cli_t;

struct _nf_server_t
{
    size_t server_type;
    size_t connect_type; 
    size_t reactor_num;   //reactor num
    size_t thread_num;    //每个reactor的thread num

    size_t backlog;
    size_t listen_port;
    size_t need_join;
    
    size_t connect_to;
    size_t read_to;
    size_t write_to;
    
    size_t thread_usr_buf;
    
    int run;     
    size_t stack_size; //线程栈大小

    size_t listen_prio; 
    size_t work_prio;

    size_t socksize;
 
    char name[256];
    
    int sock_family;
    int sev_socket; 
   
    ev_handle read_handle;
    ParseFun * read_parse_handle;
    ev_handle write_handle;
    ParseFun * write_parse_handle;
       
    NfReactor * svr_reactor;

    SERVER_STATUS_T status;
};

class NfReactor : public Reactor
{
    public:
        NfReactor() : Reactor()
        {} 
        int init(int, nf_server_t *);
        nf_server_t * get_server()
        {
            return svr;
        }
        
        nf_server_cli_t * get_cli_data()
        {
            return cli_data;
        }
        int set_cli_data(int, nf_server_cli_t *);

    protected:
        nf_server_t * svr;
        nf_server_cli_t * cli_data;
}; 

int nf_server_bind(nf_server_t * sev);

int nf_server_listen(nf_server_t * sev);

int set_sev_socketopt(nf_server_t *sev, int fd);

#endif
