//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  leader-follower model
//  线程模型，每个线程维护一个fd, 适合短连接，快速处理任务
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#include <sys/epoll.h>
#include <pthread.h>
#include "nf_server_core.h"
#include "nf_server_app.h"
#include "pool_register.h"

typedef struct _lfpool_t 
{   
    pthread_mutex_t lock;
} lfpool_t;

int lfpool_init(nf_server_t * sev)
{
    if(sev->pool == NULL )
    {
        sev->pool = malloc(sizeof(lfpool_t));
        if(sev->pool == NULL)
        {   std::cout << "malloc pool error" << std::endl; return -1;}
    }
    pthread_mutex_init(&((lfpool_t *)sev->pool)->lock, NULL);      
    return 0;
}

static int lfpool_once_op(int epfd, int fd, int timeout)
{
    int stat;
    //清空表 omit no such file error
    if (net_ep_del(epfd, fd) < 0)
        if(errno != 2)
            return -1;

    if (net_ep_add_in1(epfd, fd) < 0)
        return -1;
    
    struct epoll_event events[1];
    stat = epoll_wait(epfd, events, 1, timeout);

    if ( stat < 0)
        return -1;
    else if (stat == 0)
        return stat;

    return 1;
}


void * lf_main(void * param)
{
    
    nf_server_pdata_t *pdata = (nf_server_pdata_t *)param;
    nf_server_t *sev = (nf_server_t *)pdata->server;
    lfpool_t *pool = (lfpool_t *)sev->pool;
    int ret; 
    set_pthread_data(pdata);
    
    lfpool_t temp_pool;
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);             
    
    std::cout << "thread start: " << pdata->id << std::endl;
    pthread_mutex_init( &(temp_pool.lock), NULL);
    pdata->epfd = net_ep_create(1);
    if( pdata->epfd < 0)
    {
        std::cout << strerror(errno) << std::endl;
        goto EXIT;
    }

    while(sev->run)
    {  //race leader
       pthread_mutex_lock(&(pool->lock));
    
       if( !sev->run ) 
       {
           pthread_mutex_unlock(&(pool->lock));
           goto EXIT;                
       }
       
       //std:: cout << "listen fd : " << sev->sev_socket << std :: endl; 
       if((ret = lfpool_once_op(pdata->epfd, sev->sev_socket, 5000)) <= 0) 
       {
           if(ret < 0)
           {
               std::cout << strerror(errno) << std::endl;
               std::cout << "once op error : " << strerror(errno) << std::endl;
           }
           pthread_mutex_unlock(&(pool->lock));
           continue;
       }

       if(!sev->run)
       {
            pthread_mutex_unlock(&(pool->lock));
            goto EXIT;
       }
        
       pdata->fd = net_accept(sev->sev_socket, (sockaddr *)&caddr, (socklen_t *)&clen);
       //release become worker.
       pthread_mutex_unlock( &(pool->lock)); 
       
       pdata->client_addr = caddr; 
       //accept noblocking

       set_fd_noblock(pdata->fd);

       if(pdata->fd < 0)
       {
            pdata->fd = -1;
            std::cout << strerror(errno) << std::endl;
            std::cout << "accept fd error, maybe EINTER" << std::endl;
            continue;
       }
       //set worker socket
       set_sev_socketopt(sev, pdata->fd);

       //work
       sev->stratgy->work(pdata);
 
       net_ep_del(pdata->epfd, pdata->fd); 
       close(pdata->fd);
       pdata->fd = -1;
       //work end, become fllower 
    }

    EXIT:
    if(pdata->epfd > 0)
        close(pdata->epfd);
    return NULL;
}

//启动线程池内线程
int lfpool_run(nf_server_t * sev)
{
    sev->run_thread_num = 0;
    for(size_t i = 0; i < sev->pthread_num; ++i)
    {
        sev->pdata[i].id = i;
        int ret = 0;
         
        if(sev->stack_size > 0)
        {
            pthread_attr_t thread_attr;
            if (pthread_attr_init(&thread_attr) != 0)
            {
                std::cout << "init thread attr error" << std::endl; 
                return -1;
            }
    
            if( sev->stack_size > 1024)
            {
                if (pthread_attr_setstacksize(&thread_attr, sev->stack_size) != 0)
                {
                    std::cout << "set stack size error" << std::endl; 
                    return -1;
                }
                ret = pthread_create(&sev->pdata[i].pid, &thread_attr, lf_main, &sev->pdata[i]);
                pthread_attr_destroy(&thread_attr);
            }
            else
            {
                ret = pthread_create(&sev->pdata[i].pid, NULL, lf_main, &sev->pdata[i]);
            }
            if(ret != 0)
            {
                std::cout << "create thread error" << std::endl;
                std::cout << strerror(errno) << std::endl;
                return -1;
            }
            sev->run_thread_num++;
        }
        else
        {   std::cout << "stacksize must > 0" << std::endl; return -1; }
    }
    return 0; 
}



int lfpool_join(nf_server_t * sev)
{

    for(int i = 0; i < sev->run_thread_num; i++)
    {
        std::cout << "join: "<<i << std::endl;
        pthread_join(sev->pdata[i].pid, NULL);
        std::cout << "thread : " << sev->pdata[i].id << "return succ" << std::endl;  
    }
    return 0;
}

int lfpool_destroy(nf_server_t * sev)
{
    lfpool_t * pool = (lfpool_t *)sev->pool;
    if( pool == NULL)
        return 0;
    pthread_mutex_destroy(&pool->lock);    
    printf("destroy mutex ok");
    if ( sev->pool != NULL)
        free(sev->pool);
    sev->pool = NULL;    
    return 0;
}

long long lfpool_get_socknum(nf_server_t *)
{
    return 0;
}

long long lfpool_get_queuenum(nf_server_t *)
{
    return 0;
}

int lfpool_pause(nf_server_t *)
{
    return 0;
}

int lfpool_resume(nf_server_t *)
{
    return 0;
}

int lfpool_set_stratgy(nf_server_t * sev, BaseWork * sta)
{
    if(sta == NULL)
    {
        sev->stratgy = new LfReadLine();
        return 0; 
    }
    LfBaseWork * test = dynamic_cast<LfBaseWork *>(sta);
    assert(test != NULL);
    sev->stratgy = test;
             
    return 0;
}

