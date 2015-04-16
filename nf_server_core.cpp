#include "nf_server_core.h"
#include "net.h"
#include <unistd.h>
#include "pool_register.h"

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
    sev->p_read = NULL;     
    sev->p_wirte = NULL;     
    
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
    pdata->fd = -1;
     
    pdata->read_buf = NULL;
    pdata->write_buf = NULL;

    pdata->ep_size = 1;

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
    
    sev->run = 1;
    //conf 相关内容初始化
    if(sev == NULL)
        return -1;
    //监听端口
    sev->listen_port = 1025;
    //超时设置
    sev->connect_to = 1000;
    sev->read_to = 100;
    sev->write_to = 100;
    
    if (sev->connect_type == NFSVR_SHORT_CONNEC)
        sev->server_type = NFSVR_LFPOOL;

    sev->pool = NULL;
    //线程数
    sev->pthread_num = 150;
    //线程栈
    //sev->stack_size = 10485760;  //10M
    sev->stack_size = 1; //默认值

    //线程内容 初始化 
    sev->pdata = (nf_server_pdata_t *) malloc(sizeof(nf_server_pdata_t) * (sev->pthread_num + 2));
    if (sev->pdata == NULL)
        return -1;
    
    if(sev->cb_work == NULL)
        sev->cb_work = nf_default_worker;
    if(sev->p_read == NULL)
        sev->p_read = nf_default_read_buf;
    if(sev->p_write == NULL)
        sev->p_write = nf_default_write_buf;

    for(int i = 0; i < (sev->pthread_num + 2); i++)
    { 
       if( (ret = nf_pdata_init(&sev->pdata[i], sev) ) < 0 )
            return -1;
    }

    return 0;
}

int set_sev_socketopt(nf_server_t *sev, int fd)
{
    //目前设计 为 write-write-read 模式，默认关 TCPDELAY, do not acculate syn
    const int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    
    //短链接 默认开启 linger设置, time_wait control
    if( sev->connect_type == NFSVR_SHORT_CONNEC)
    {
        struct linger li;
        memset(&li, 0, sizeof(li)); 
        li.l_onoff = 1;
        li.l_linger = 1;
        setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&li, sizeof(li) );     
    }
    
    //默认开启TCP_DEFER_ACCCEPT, drop no data connection
    int timeout = sev->connect_to;
    setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(timeout));
    
    //默认 设置 accept socket is noblock
    if ( set_fd_noblock(fd) < 0)
        return -1;

    return 0; 
}

int nf_server_bind(nf_server_t * sev)
{
    const int on = 1;
    struct sockaddr_in addr;
    int ret;
    
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
    set_sev_socketopt(sev, sev->sev_socket);
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
    
    ret = g_pool[sev->server_type].init(sev); 
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
    std::cout << "listen ok" << std::endl;
    //listen 函数为空
    if( g_pool[sev->server_type].listen == NULL)
        return 0;

    return g_pool[sev->server_type].listen(sev);
}


int nf_default_worker(void *req)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *)req;
    int ret = nepoll_add(pdata->epfd, pdata->fd);
    if( ret < 0)
        return -1;
    //读等待超时

    int readto = ((nf_server_t *)req->server)->read_to;
    nf_handle_t read_fun = ((nf_server_t *)req->server)->nf_default_read_buf;
    nf_handle_t write_fun = ((nf_server_t *)req->server)->nf_default_write_buf;
    
    int readto = ((nf_server_t *)req->server)->read_to;
    const int size = pdata->ep_size;
    struct epoll_event events[size], ev;
    
    while(1)
    {
        ret = epoll_wait(pdata->epfd, events, size, read_to);
        if(ret == 0)
            {std::cout << "read timeout error" << std::endl; return ret;}
        else if(ret < 0)
            return ret; 
        //events analyse
        for(int i = 0; i < ret; i++)
        {
            if( events[i].events & EPOLLRDHUP)
            { 
                std::cout << "event close fd error" << std::endl; 
                return -1;
            }
            else if( events[i].events & EPOLLIN )
            {
                if( read_fun(pdata) < 0)
                    return -1;
                else
                {
                    ev.data.fd = pdata->fd;
                    ev.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
                    epoll_ctl(pdata->epfd, EPOLL_CTL_ADD, pdata->fd, &ev);
                }
            }
            else if( events[i].events & EPOLLOUT )
            {
                if( write_fun(pdata) < 0)
                    return -1;
                else
                {
                    ev.data.fd = pdata->fd;
                    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
                    epoll_ctl(pdata->epfd, EPOLL_CTL_ADD, pdata->fd, &ev);
                }
            }
        }
    }
    return 1;
}

int nf_default_read_buf()
{

}





