#include "rapool.h"

enum 
{
    RA_CHECK_INTERVAL = 10,  ///Ä¬ÈÏcheck_interval, 10ms
    RA_TIMEOUT = 60,         ///µÈ´ý¶ÓÁÐÀïÃæµÄÄ¬ÈÏ³¬Ê±Ê±¼ät, 60s
    RA_SOCK_NUM = 500,       ///Í¬Ê±Î¬»¤µÄ×î´ósock¾ä±úÊý£¬500
};

enum 
{
    IDLE = 0,
    BUSY
};

#define MAX(a,b) (((a)>(b)) ? (a) : (b))

static int 
add_listen_socket(nf_server_t *sev, int listenfd)
{
    rapool_t *pool = (rapool_t *)sev->pool;
    int idx = 0;
    //¼àÌý¾ä±úÓÀÔ¶´¦ÓÚbusy×´Ì¬ÕâÑù²»»Ø±»¹Ø±Õ
    pool->sockets[idx].status = BUSY;
    pool->sockets[idx].sock = listenfd;

    char tmp[] = "0.0.0.0";
    set_tcp_sockaddr(tmp, 0, &(pool->sockets[idx].addr));

    return rapool_epoll_add_read(sev, idx, 0);
}

int 
rapool_init(nf_server_t *sev)
{
    int ssiz = nf_server_get_socksize(sev);
    rapool_t *pool = NULL;

    if (ssiz <= 0) 
    {
        ssiz = RA_SOCK_NUM;
        sev->socksize = ssiz;
    }

    if (sev->pool == NULL) 
    {
        sev->pool = malloc(sizeof(rapool_t));
        if (sev->pool == NULL) 
        {
            Log :: ERROR("RAPOOL.cpp : 50 MALLOC RA-POOL ERROR");
            
            return -1;
        }
        memset(sev->pool, 0, sizeof(rapool_t));
    }

    Log :: NOTICE("RAPOOL.cpp MALLOC RA-POOL SUCC");
     
    pool = (rapool_t *) sev->pool;
    pthread_mutex_init(&pool->ready_mutex, NULL);
    pthread_cond_init(&pool->ready_cond, NULL);

    //´´½¨socket×ÊÔ´
    pool->size = ssiz;
    pool->sockets = (rapool_sock_item_t *) malloc (sizeof(rapool_sock_item_t) * ssiz);
    
    for(int i = 0; i < ssiz; i++)
    {
        pool->sockets[i].rp.rio_ptr = ((char *) malloc(sizeof(char) * 
                                    sev->thread_usr_buf)); 
        if(pool->sockets[i].rp.rio_ptr == NULL)
            return -1;
        memset(pool->sockets[i].rp.rio_ptr, 0, sev->thread_usr_buf); 
        rio_init(&pool->sockets[i].rp, -1, sev->thread_usr_buf); 
    }

    if (pool->sockets == NULL) 
    {
        Log :: ERROR("INIT RA-SOCKET POOL ERROR");
        return -1;
    }
    Log :: NOTICE("INIT RA-SOCKET POOL SUCC");

    //´´½¨epoll×ÊÔ´
    pool->ep_events = (struct epoll_event *) malloc (sizeof(struct epoll_event) * ssiz);

    if (pool->ep_events == NULL) 
    {
        Log :: ERROR("INIT EPOLL EVENTS IN LISTEN THREAD ERROR");
        return -1;
    }
    Log :: NOTICE("INIT EPOLL EVENTS IN LISTEN THREAD SUCC");
    memset(pool->ep_events, 0, ssiz * sizeof(struct epoll_event));

    //´´½¨epoll¾ä±ú
    //LISTEN MAIN REACTOR
    pool->epfd = epoll_create(ssiz);
    if (pool->epfd < 0) 
    {
        Log :: ERROR("INIT EPOLL HANDLE IN LISTEN THREAD ERROR");
        return -1;
    }
    sev->pdata[0].epfd = pool->epfd;
    Log :: NOTICE("INIT EPOLL HANDLE IN LISTEN THREAD SUCC");
    
    //´´½¨epoll¾ä±ú
    //SUB REACTOR
    for (int i=1; i< sev->pthread_num; ++i) 
    {
        sev->pdata[i].epfd = epoll_create(ssiz);
        if (pool->epfd < 0) 
        {
            Log :: ERROR("INIT EPOLL HANDLE IN WORK REACTOR THREAD ERROR,\
                         THREAD INDEX %d", i);
            return -1;
        }
        Log :: NOTICE("INIT EPOLL HANDLE IN WORK REACTOR THREAD SUCC, THREAD INDEX %d", i);
    }

    pool->timeout = -1;
    if (pool->timeout <= 0) 
    {
        pool->timeout = sev->timeout;
    }
    Log :: NOTICE("DEFAULT EPOLL TIMEOUT %d ", sev->timeout);

    //pool->check_interval = DEFAULT_CHECK_INTERVAL;
    pool->check_interval = sev->check_interval;
    Log :: NOTICE("DEFAULT EPOLL CHECK INTERVAL %d ", sev->check_interval);

    pool->run = &sev->run;
    pool->using_size = 0;

    return 0;
}

long long 
rapool_get_queuenum(nf_server_t *sev)
{
    return 0;
}

int 
rapool_run(nf_server_t *sev)
{
    int i = 0;
    int ret = 0;
    rapool_t *pool = (rapool_t *)sev->pool;

    pthread_attr_t thread_attr;
    struct sched_param param = { 0 };
    ret = pthread_attr_init(&thread_attr);
    if (ret) 
    {
        std :: cout << "error thread attr init" << std :: endl;
        return -1;
    }

    ret = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) 
    {
        std :: cout << "error thread set herit sched" << std :: endl;
        return -1;
    }

    ret = pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    if (ret) 
    {
        std :: cout << "error thread set schedpolicy" << std :: endl;
        return -1;
    }

    param.sched_priority = LISTENER_PRIORITY;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if (ret) 
    {
        std :: cout << "error thread set listen priority" << std :: endl;
        return -1;
    }

    ret = pthread_create(&pool->main, &thread_attr, 
                         rapool_main, &sev->pdata[0]);
    sev->pdata[0].pid = pool->main;

    Log :: NOTICE("PDATA INDEX : %d IS LISTEN THREAD tid %d ", 0, pool->main);    

    if (ret) 
    {
        Log :: ERROR("THREAD SET LISTEN THREAD ERROR ");    
        return -1;
    }

    sev->run_thread_num = 0;
    param.sched_priority = WORKER_PRIORITY;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if (ret) 
    {
        std :: cout << "error thread set work priority" << std :: endl;
        return -1;
    }

    //´´½¨Âß¼­´¦Àí×ÓÏß³Ì
    for (i = 1; i < sev->pthread_num; ++i) 
    {
        sev->pdata[i].id = i;
        ret = pthread_create(&sev->pdata[i].pid, &thread_attr, 
                             rapool_workers, &sev->pdata[i]);
        if (ret) 
        {
            Log:: ERROR("CREATE WORK THREAD ERROR ID : %d", i);
            return -1;
        }
        Log :: NOTICE("CREATE WORK THREAD SUCC ID : %d", i);
        ++ sev->run_thread_num;
    }

    Log :: NOTICE("CREATE REACTOR NUM : %d", sev->run_thread_num);
    sev->status = RUNNING;
    pthread_attr_destroy(&thread_attr);
    return 0;
}

int 
rapool_listen(nf_server_t *sev)
{
    return add_listen_socket(sev, sev->sev_socket);
}

int 
rapool_join(nf_server_t *sev)
{
    rapool_t *pool = (rapool_t *)sev->pool;
    if (sev->run_thread_num >= 0) 
    {
        pthread_join(pool->main, NULL);
        for (int i=0; i< sev->run_thread_num; ++i) 
        {
            pthread_join(sev->pdata[i].pid, NULL);
        }
    }
    return 0;
}

static void 
rapool_close_pool_sockets(nf_server_t *sev, bool is_listenfd)
{
    rapool_t * pool = (rapool_t *) sev->pool;
    //is_listenfdÊÇtrueÊ±£¬Éú²úÏß³Ìµ÷ÓÃÊ±£¬ÉèÖÃis_listenfd=true£¬ÏÈ¹Ø±Õ¼àÌýsockfd
    //Çå¿Õ listen µÄ socket
    if (is_listenfd) 
    {
        if (pool->sockets[0].status != IDLE) 
        {
            pool->sockets[0].status = IDLE;
            close(pool->sockets[0].sock);
            pool->sockets[0].sock = -1;
        }
        return;
    }
    
    //Çå¿Õ pending socket poolÖÐ µÄ ÄÚÈÝ, pending socket Ò
    //Ò»¸ö produce ²Ù×÷£¬ ²»ÐèÒª Ëø
    for (int i=0; i < pool->size; ++i) 
    {
        if (pool->sockets[i].status != IDLE) 
        {
            close(pool->sockets[i].sock);
            pool->sockets[i].status = IDLE;
            pool->sockets[i].sock = -1;
        }
    }
}

int rapool_destroy(nf_server_t *sev)
{
    rapool_t *pool = (rapool_t *)sev->pool;
    if (pool == NULL) 
        return 0;

    //´ËÊ±Ïû·ÑÏß³Ì¶¼ÒÑÍË³ö£¬¾Í´Ë¹Ø±ÕËùÓÐÒÑÁ¬½Ósockfd
    rapool_close_pool_sockets(sev, false);

    pthread_mutex_destroy(&pool->ready_mutex);
    pthread_cond_destroy(&pool->ready_cond);

    if (pool->sockets != NULL) 
    {
        free (pool->sockets);
        pool->sockets = NULL;
    }
    if (pool->ep_events != NULL) 
    {
        free (pool->ep_events);
        pool->ep_events = NULL;
    }
    if (pool->epfd > 0) 
    {
        close(pool->epfd);
    }

    free (sev->pool);
    sev->pool = NULL;
    return 0;
}

void * 
rapool_main(void *param)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) param;
    nf_server_t * sev = (nf_server_t *) pdata->server;
    
    set_pthread_data(pdata);

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    set_fd_noblock(sev->sev_socket);
     
    int work_thread = 1;
    while (sev->run) 
    {
        //¸ºÔØ¾ùºâ, simple ÂÖÑ¯
        int ret = rapool_produce(sev, (struct sockaddr *) &addr, &addr_len, work_thread);
        //ret < 0 , timeoutt or EINTER, ²»¸üÐÂ´¦Àíreactor
        if(ret >= 0)
        {
            work_thread += 1;
            if(work_thread == sev->pthread_num)
                work_thread = 1;
        }
    }

    rapool_close_pool_sockets(sev, true);

    pthread_exit(NULL);
    return NULL;
}

int 
rapool_produce(nf_server_t * sev, struct sockaddr * addr, 
                   socklen_t * addrlen, int work_reacotr)
{
    int ret = 0;
    rapool_t *pool = (rapool_t *) sev->pool;
    int sev_sock = sev->sev_socket;
    char ip[40];
    int len = 40;
    int port;
    int idx;

    int num = epoll_wait(pool->epfd, pool->ep_events, pool->size, pool->timeout * 1000);

    if (num <= 0) 
    {
        Log :: WARN("LISTEN WAIT IS INTERUPT/TIMEOUT, WILL RESTART");
        return -1;
    }
        //listen port
    if (pool->ep_events[0].data.fd == 0) 
    {
        int cli_sock = net_accept(sev_sock, addr, addrlen);
        if (cli_sock < 0) 
        {
            Log :: WARN("ACCEPT %d ERROR %s", sev_sock, strerror(errno));
        }
            
        get_tcp_sockaddr(ip, &port, (sockaddr_in *)addr, len);
        Log :: NOTICE("ACCEPT SUCC FROM CLIENT: %s:%d  new fd : %d ", 
                           ip, port, cli_sock);

        set_sev_socketopt(sev, cli_sock);

        //add cli_sock to connection pool
        if ((idx = rapool_add(sev, cli_sock, (struct sockaddr_in *) addr)) < 0) 
        {
            close(cli_sock);
            ret = -1;
        }
        //add cli_sock to reactor
        if (rapool_epoll_add_read(sev, idx, work_reacotr) < 0)
        {
            close(cli_sock);
            ret = -1;
        }
    } 
    return ret;
}

int 
rapool_add(nf_server_t * sev, int sock, struct sockaddr_in *addr)
{
    rapool_t *pool = (rapool_t *) sev->pool;
    int idx = -1;
    for (int i = 1; i < (int)pool->size; ++i) 
    {
        if (pool->sockets[i].status == IDLE) 
        {
            idx = i;
            break;
        }
    }

    if (idx < 0) 
    {
        Log :: WARN("Because the array(size=%d) is full, so close the new connection",(int) pool->size);
        return -1;
    }

    pool->sockets[idx].status = BUSY;
    pool->sockets[idx].last_active = time(NULL);
    pool->sockets[idx].addr = *addr;
    pool->sockets[idx].sock = sock;
    
    Log :: NOTICE("ADD LISTEN FD : %d TO CONN POOL SUCC, ADD POS : %d",
                  sock, idx);
    
    return idx;
}

int 
rapool_epoll_add_read(nf_server_t *sev, int idx, int work_reactor)
{
    rapool_t *pool = (rapool_t *) sev->pool;
    struct epoll_event ev;
    ev.data.fd = idx;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    int sock = pool->sockets[idx].sock;
    
    if (epoll_ctl(sev->pdata[work_reactor].epfd, EPOLL_CTL_ADD, sock, &ev) < 0) 
    {
        Log :: WARN("ADD EPOLL EVNENT READ TO  REACTOR ERROR: %d STR:%s", 
                    work_reactor, strerror(errno));
        return -1;
    }
    Log :: NOTICE("SUCC ADD EPOLL EVNENT READ TO REACTOR : %d, FD : %d", 
                work_reactor, sock);
    return 0;
}

int 
rapool_epoll_del(nf_server_t * sev, int idx, int work_reactor)
{
    rapool_t *pool = (rapool_t *) sev->pool;
    struct epoll_event ev;
    ev.data.fd = idx;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    int sock = pool->sockets[idx].sock;

    if (epoll_ctl(pool->epfd, EPOLL_CTL_DEL, sock, &ev) < 0) 
    {
        std :: cout << "del socket error: " << strerror(errno) << std :: endl;
        return -1;
    }
    return 0;
}

int 
rapool_del(nf_server_t *sev, int idx, int alive, bool remove)
{
    rapool_t * pool = (rapool_t *) sev->pool;
    int id = nf_server_get_thread_id(); 
    if (alive == 0) 
    {
        if(remove)
            rapool_epoll_del(sev, idx, id);  // remove it from epoll event sets
        if (pool->sockets[idx].sock >= 0) 
            close(pool->sockets[idx].sock);
        
        Log :: NOTICE("RELEASE CONN POOL %d FD : ", 
                      idx, pool->sockets[idx].sock);

        pool->sockets[idx].sock = -1;
        pool->sockets[idx].status = IDLE;

        if(pool->sockets[idx].rp.cache != NULL)
            Allocate :: deallocate(pool->sockets[idx].rp.cache, 
                                   pool->sockets[idx].rp.cache_len);
        pool->sockets[idx].rp.cache = NULL;
        pool->sockets[idx].rp.cache_len = 0; 
    } 
    return 0;
}


void * 
rapool_workers(void * param)
{
    
    nf_server_pdata_t *pdata = (nf_server_pdata_t *) param;
    nf_server_t * sev = (nf_server_t *) pdata->server;

    set_pthread_data(pdata);

    while (sev->run) 
    {
        rapool_reactor((rapool_t *) sev->pool, pdata);
    }

    pthread_exit(NULL);
    return NULL;
}

int 
rapool_reactor(rapool_t * pool, nf_server_pdata_t * pdata)
{
    int idx;
    nf_server_t * sev = (nf_server_t *)pdata->server;
   
    int ret = sev->cb_work(pdata);
    if(!sev->run) 
    {
        close(pdata->epfd);
    }
    return 0;
}

int 
rapool_pause(nf_server_t *sev)
{
    if(sev->status != RUNNING) 
    {
        std :: cout << "Because server's status isn't running, rapool_pause Failed!" 
        << std::endl;
        return -1;
    }

    rapool_t * pool = (rapool_t *) sev->pool;
    int index = 0;
    if(rapool_epoll_del(sev, index, 0) != 0) 
    {
        return -1;
    }
    pool->sockets[index].status = IDLE;
    pool->sockets[index].sock = -1;
    sev->status = PAUSE;
    return 0;
}

int rapool_resume(nf_server_t *sev)
{
    if(sev->status != PAUSE) 
    {
        std::cout << "Because server's status isn't PAUSE, rapool_resume Failed!" 
        <<std::endl;
        return -1;
    }

    if(add_listen_socket(sev, sev->sev_socket) != 0) 
    {
        return -1;
    }

    sev->status = RUNNING;
    return 0;
}

