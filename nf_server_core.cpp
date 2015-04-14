#include "nf_server_core.h"
#include "net.h"
#include <unistd.h>


nf_server_t * nf_server_create(const char * sev_name)
{
    nf_server_t *sev = (nf_server_t *)malloc(sizeof(nf_server_t));
    if(sev == NULL)
        return NULL;
   
    sev->server_type = NFSVR_LFPOOL;
    sev->connect_type = NFSVR_SHORT_CONNEC;
    sev->pthread_num = 1;
    sev->run_thread_num = 0;
    sev->sock_family = AF_INET;
    sev->backlog = 2048;
    sev->thread_write_buf = 0;
    sev->thread_read_buf = 0;
   
    sev->stack_size = 10485760;  //10M
    sev->sev_socket = -1;    

    sev->cb_work = NULL;
    sev->pdata = NULL;
    sev->p_start = NULL;
    sev->p_end = NULL;

    sev->status = INIT;

    if(sev_name == NULL)
    {
        strncpy(sev->name, "simple server", sizeof(sev->name));
        sev->name[sizeof(sev->name) - 1] = '\0';
    }
    else
    {
        strncpy(sev->name, sev_name, sizeof(sev->name));
        sev->name[sizeof(sev->name) - 1] = '\0';
    } 
    return sev;
}

int nf_pdata_init(nf_server_pdata_t * pdata, nf_server_t * sev)
{
    pdata->pid = 0;
    pdata->id = 0;
    pdata->server = sev;
        
    pdata->read_buf = NULL;
    pdata->write_buf = NULL;

    if(sev->thread_read_buf == 0)
        pdata->read_size = 1024;
    if(sev->thread_write_buf == 0)
        pdata->write_size = 1024;

    pdata->read_buf = malloc(sizeof(char) * pdata->read_size);
    if(pdata->read_buf == NULL)
        return -1;
    memset(pdata->read_buf, 0, sizeof(char) * pdata->read_size);
    
    pdata->write_buf = malloc(sizeof(char) * pdata->write_size);
    if(pdata->write_buf == NULL)
        return -1;
    memset(pdata->write_buf, 0, sizeof(char) * pdata->write_size);

    return 0;
}

int nf_server_init(nf_server_t * sev)
{
    int ret = 0;
    //conf 相关内容初始化
    if(sev == NULL)
        return -1;
    //监听端口
    sev->listen_port = 1028;
    //超时设置
    sev->connect_to = 1000;
    sev->read_to = 1000;
    sev->write_to = 1000;
    
    if (sev->connect_type == NFSVR_SHORT_CONNEC)
        sev->server_type = NFSVR_LFPOOL;

    //线程数
    sev->pthread_num = 1;
    //线程栈
    sev->stack_size = 10485760;  //10M

    //线程内容 初始化 
    sev->pdata = (nf_server_pdata_t *) malloc(sizeof(nf_server_pdata_t) * (sev->pthread_num + 2));
    if (sev->pdata == NULL)
        return -1;
    
    for(int i = 0; i < (sev->pthread_num + 2); i++)
    { 
       if( (ret = nf_pdata_init(&sev->pdata[i], sev) ) < 0 )
            return -1;
    }

    return 0;
}

int set_sev_socketopt(nf_server_t *sev)
{
    //目前设计 为 write-write-read 模式，默认关 TCPDELAY
    const int on = 1;
    setsockopt(sev->sev_socket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    
    //短链接 默认开启 linger设置
    if( sev->connect_type == NFSVR_SHORT_CONNEC)
    {
        struct linger li;
        memset(&li, 0, sizeof(li)); 
        li.l_onoff = 1;
        li.l_linger = 1;
        setsockopt(sev->sev_socket, SOL_SOCKET, SO_LINGER, (const char*)&li, sizeof(li) );     
    }
    
    //默认开启TCP_DEFER_ACCCEPT
    int timeout = sev->connect_to;
    setsockopt(sev->sev_socket, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(timeout));
    
    return 0; 
}

int nf_server_bind(nf_server_t * sev)
{
    const int on = 1;
    struct sockaddr_in addr;

    if(sev->sev_socket < 0)
    {
        if( (sev->sev_socket = socket(PF_INET, SOCK_STREAM, 0) ) < 0 )
        {
            std::cout << "create sock: " << strerror(errno) << std::endl;
            return -1; 
        }
    }
    //复用
    setsockopt(sev->sev_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(sev->listen_port);
    
    if (bind(sev->sev_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) 
    {
        std::cout << "bind sock: " << strerror(errno) << std::endl;
        close(sev->sev_socket);
        return -1;    
    }
     
    return 0;
}


int nf_server_listen(nf_server_t * sev)
{
    if(sev->backlog <= 5)
        sev->backlog = 2048;
    int backlog = sev->backlog;
    if( listen(sev->sev_socket, backlog) < 0)
    {
        std::cout << "listen sock: " << strerror(errno) << std::endl;
        close(sev->sev_socket);
        return -1;    
    }
    return 0;
}





