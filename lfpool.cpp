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

void * lf_main(void * param)
{
    int num = 1;
    nf_server_pdata_t *pdata = (nf_server_pdata_t *)param;
    nf_server_t *sev = (nf_server_t *)pdata->server;
    
     
    return 0;
}

//启动线程池内线程
int lfpool_run(nf_server_t * sev)
{
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
            }
            
            ret = pthread_create(&sev->pdata[i].pid, &thread_attr, lf_main, &sev->pdata[i]);
            if(ret != 0)
            {
                std::cout << "create thread error" << std::endl;
                std::cout << strerror(errno) << std::endl;
                return -1;
            }
        }
    }

    return 0; 
}



int lfpool_join(nf_server_t * sev)
{
    return 0;
}

int lfpool_destroy(nf_server_t * sev)
{
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

