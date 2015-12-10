
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
#include "interface/ireactor.h"

typedef struct _nf_server_t nf_server_t;

struct _nf_server_t
{
    size_t server_type;
    size_t connect_type; 

    size_t backlog;
    size_t listen_port;
    
    size_t connect_to;
    size_t read_to;
    size_t write_to;
    
    int run;     

    size_t listen_prio; 
    size_t work_prio;

    size_t socksize;
 
    char name[256];
    
    int sock_family;
    int sev_socket; 
   
    IReactor * svr_reactor;

    int status;
};

int nf_server_bind(nf_server_t * sev);

int nf_server_listen(nf_server_t * sev);

int set_sev_socketopt(nf_server_t *sev, int fd);

#endif
