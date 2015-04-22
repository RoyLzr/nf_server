#include <sys/epoll.h>
#include "pool_register.h"
#include <pthread.h>


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
    //��ձ� omit no such file error
    if (nepoll_del(epfd, fd, 1) < 0)
        if(errno != 2)
            return -1;

    if (nepoll_add_one(epfd, fd) < 0)
        return -1;
    
    struct epoll_event events[1];
    if (epoll_wait(epfd, events, 1, timeout) < 0)
        return -1;
    return 0;
}


void * lf_main(void * param)
{
    
    nf_server_pdata_t *pdata = (nf_server_pdata_t *)param;
    nf_server_t *sev = (nf_server_t *)pdata->server;
    lfpool_t *pool = (lfpool_t *)sev->pool;
    lfpool_t temp_pool;
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);             
    
    std::cout << "thread start: " << pdata->id << std::endl;
    pthread_mutex_init( &(temp_pool.lock), NULL);
    pdata->epfd = nepoll_create(1);
    if( pdata->epfd < 0)
    {
        std::cout << strerror(errno) << std::endl;
        goto EXIT;
    }
 
    while(sev->run)
    {  //race leader
       pthread_mutex_lock(&(pool->lock));
    
       if( !sev->run || lfpool_once_op(pdata->epfd, sev->sev_socket, -1) < 0) 
       {
           std::cout << strerror(errno) << std::endl;
           std::cout << "once op error : " << strerror(errno) << std::endl;
           pthread_mutex_unlock(&(pool->lock));
           goto EXIT;                
       }
       
       if( !sev->run )
       {
            pthread_mutex_unlock(&(pool->lock));
            goto EXIT;
       }

       pdata->fd = naccept(sev->sev_socket, (sockaddr *)&caddr, (socklen_t *)&clen);
       //release become worker.
       pthread_mutex_unlock( &(pool->lock)); 
      
       pdata->client_addr = caddr; 
       //accept noblocking

       set_fd_noblock(pdata->fd);

       if(pdata->fd < 0)
       {
            pdata->fd = -1;
            std::cout << strerror(errno) << std::endl;
            std::cout << "accept fd error, maybe TIMEWAIT" << std::endl;
            continue;
       }
       //set worker socket
       set_sev_socketopt(sev, pdata->fd);
       //Ŀǰ���� ���� ����ݽ����    

       //work
       if(sev->cb_work(pdata) < 0)
       {
            if(errno != 2)
                std::cout << "work end error: " << strerror(errno) << std::endl;
       }
 
       nepoll_del(pdata->epfd, pdata->fd); 
       close(pdata->fd);
       pdata->fd = -1;
       //work end, become fllower 
    }

    EXIT:
    if(pdata->epfd > 0)
        close(pdata->epfd);
}

//�����̳߳����߳�
int lfpool_run(nf_server_t * sev)
{
    sev->run_thread_num = 0;
    for(int i = 0; i < sev->pthread_num; ++i)
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
        std::cout << i << std::endl;
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

