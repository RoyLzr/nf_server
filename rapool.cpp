#include "rapool.h"

enum 
{
    RA_CHECK_INTERVAL = 10,  ///Ĭ��check_interval, 10ms
    RA_TIMEOUT = 60,         ///�ȴ����������Ĭ�ϳ�ʱʱ��t, 60s
    RA_SOCK_NUM = 500,       ///ͬʱά�������sock�������500
};

enum 
{
    IDLE = 0,
    BUSY
};

#define MAX(a,b) (((a)>(b)) ? (a) : (b))

int  
RaServer :: add_listen_socket(nf_server_t *sev, int listenfd)
{
    rapool_t *pool = (rapool_t *)sev->pool;
    int idx = 0;
    int work = 0;
    //���������Զ����busy״̬�������ر��ر�
    pool->sockets[idx].status = BUSY;
    pool->sockets[idx].sock = listenfd;

    char tmp[] = "0.0.0.0";
    set_tcp_sockaddr(tmp, 0, &(pool->sockets[idx].addr));

    return rapool_epoll_add_read(sev, idx, work);
}

int 
RaServer :: svr_run()
{
    //int i = 0;
    int ret = 0;
    rapool_t *pool = (rapool_t *)sev_data->pool;

    pthread_attr_t thread_attr;
    struct sched_param param = { 0 };
    ret = pthread_attr_init(&thread_attr);
    if (ret) 
    {
        Log :: ERROR("error thread attr init");
        return -1;
    }

    ret = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) 
    {
        Log :: ERROR( "error thread set herit sched");
        return -1;
    }

    ret = pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    if (ret) 
    {
        Log :: ERROR("error thread set schedpolicy");
        return -1;
    }

    param.sched_priority = LISTENER_PRIORITY;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if (ret) 
    {
        Log :: ERROR("error thread set listen priority"); 
        return -1;
    }

    ret = pthread_create(&pool->main, &thread_attr, 
                         rapool_main, &sev_data->pdata[0]);
    sev_data->pdata[0].pid = pool->main;

    Log :: NOTICE("PDATA INDEX : %d IS LISTEN THREAD tid %d ", 0, pool->main);    

    if (ret) 
    {
        Log :: ERROR("THREAD SET LISTEN THREAD ERROR ");    
        return -1;
    }

    sev_data->run_thread_num = 0;
    param.sched_priority = WORKER_PRIORITY;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if (ret) 
    {
        Log :: ERROR("error thread set work priority"); 
        return -1;
    }

    //�����߼��������߳�
    for (size_t i = 1; i < sev_data->pthread_num; ++i) 
    {
        sev_data->pdata[i].id = i;
        ret = pthread_create(&sev_data->pdata[i].pid, &thread_attr, 
                             rapool_workers, &sev_data->pdata[i]);
        if (ret) 
        {
            Log:: ERROR("CREATE WORK THREAD ERROR ID : %d", i);
            exit(0); 
            return -1;
        }
        Log :: NOTICE("CREATE WORK THREAD SUCC ID : %d", i);
        ++ sev_data->run_thread_num;
    }

    Log :: NOTICE("CREATE REACTOR NUM : %d", sev_data->run_thread_num);
    sev_data->status = RUNNING;
    return 0;
}

int 
RaServer :: svr_listen()
{
    if(sev_data->backlog <= 5)
        sev_data->backlog = 2048;

    int backlog = sev_data->backlog;
    if( listen(sev_data->sev_socket, backlog) < 0)
    {
        std::cout << "listen sock: " << strerror(errno) << std::endl;
        close(sev_data->sev_socket);
        return -1;    
    }
    Log :: NOTICE("LISTEN SOCKET START OK");
    
    return add_listen_socket(sev_data, sev_data->sev_socket);
}

int 
RaServer :: svr_join()
{
    rapool_t *pool = (rapool_t *)sev_data->pool;
    if (sev_data->run_thread_num >= 0) 
    {
        pthread_join(pool->main, NULL);
        for (int i=0; i< sev_data->run_thread_num; ++i) 
        {
            pthread_join(sev_data->pdata[i].pid, NULL);
        }
    }
    return 0;
}

void 
RaServer :: rapool_close_pool_sockets(nf_server_t *sev, bool is_listenfd)
{
    rapool_t * pool = (rapool_t *) sev->pool;
    //is_listenfd��trueʱ�������̵߳���ʱ������is_listenfd=true���ȹرռ���sockfd
    //��� listen �� socket
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
    
    //��� pending socket pool�� �� ����, pending socket �
    //һ�� produce ������ ����Ҫ ��
    for (int i=0; i < pool->size; ++i) 
    {
        if (pool->sockets[i].status != IDLE) 
        {
            close(pool->sockets[i].sock);
            pool->sockets[i].status = IDLE;
            pool->sockets[i].sock = -1;
        if(pool->sockets[i].rp.w_allo_cache != NULL)
            Allocate :: deallocate(pool->sockets[i].rp.w_allo_cache, 
                                   pool->sockets[i].rp.w_allo_len);
        if(pool->sockets[i].rp.cache != NULL)
            Allocate :: deallocate(pool->sockets[i].rp.cache, 
                                   pool->sockets[i].rp.cache_len);
        }
    }
}

int 
RaServer :: svr_destroy()
{
    rapool_t *pool = (rapool_t *)sev_data->pool;
    if (pool == NULL) 
        return 0;

    //��ʱ�����̶߳����˳����ʹ˹ر�����������sockfd
    rapool_close_pool_sockets(sev_data, false);

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

    free (sev_data->pool);
    sev_data->pool = NULL;
    return 0;
}

void * 
RaServer :: rapool_main(void *param)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) param;
    nf_server_t * sev = (nf_server_t *) pdata->server;
    
    set_pthread_data(pdata);

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    set_fd_noblock(sev->sev_socket);
     
    size_t work_thread = 1;
    while (sev->run) 
    {
        //���ؾ���, simple ��ѯ
        int ret = rapool_produce(sev, (struct sockaddr *) &addr, &addr_len, work_thread);
        //ret < 0 , timeoutt or EINTER, �����´���reactor
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
RaServer :: rapool_produce(nf_server_t * sev, 
               struct sockaddr * addr, 
               socklen_t * addrlen, 
               int work_reacotr)
{
    int ret = 0;
    rapool_t *pool = (rapool_t *) sev->pool;
    int sev_sock = sev->sev_socket;
    char ip[40];
    int len = 40;
    int port;
    int idx;
    //long long key_timer;    

    int num = epoll_wait(pool->epfd, pool->ep_events, pool->size, pool->timeout);

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
            return -1;
        }
        //add timer
        rapool_t * pool = (rapool_t *) sev->pool;
        pool->sockets[idx].sock_timeout = (sev->pdata[work_reacotr].timer).add_timer_ms(
                                           1000, 
                                           call_back_timeout, 
                                           &(pool->sockets[idx]));

        //add cli_sock to reactor,order is important
        if (rapool_epoll_add_read(sev, idx, work_reacotr) < 0)
        {
            close(cli_sock);
            return -1;
        }
    } 
    return ret;
}

int 
RaServer :: rapool_add(nf_server_t * sev, int sock, struct sockaddr_in *addr)
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
    pool->sockets[idx].addr = *addr;
    pool->sockets[idx].sock = sock;
    
    Log :: NOTICE("ADD LISTEN FD : %d TO CONN POOL SUCC, ADD POS : %d",
                  sock, idx);
    
    return idx;
}

int 
RaServer :: rapool_epoll_add_read(nf_server_t *sev, int idx, int work_reactor)
{
    rapool_t *pool = (rapool_t *) sev->pool;

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

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
RaServer :: rapool_epoll_mod_read(nf_server_t *sev, int idx, int work_reactor)
{
    rapool_t *pool = (rapool_t *) sev->pool;
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    
    ev.data.fd = idx;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    int sock = pool->sockets[idx].sock;
    
    if (epoll_ctl(sev->pdata[work_reactor].epfd, EPOLL_CTL_MOD, sock, &ev) < 0) 
    {
        Log :: WARN("MOD EPOLL EVNENT READ TO  REACTOR ERROR: %d STR:%s", 
                    work_reactor, strerror(errno));
        return -1;
    }
    Log :: NOTICE("SUCC MOD EPOLL EVNENT READ TO REACTOR : %d, FD : %d", 
                work_reactor, sock);
    return 0;
}

int 
RaServer :: rapool_epoll_mod_write(nf_server_t *sev, int idx, int work_reactor)
{
    rapool_t *pool = (rapool_t *) sev->pool;
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    
    ev.data.fd = idx;
    ev.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    int sock = pool->sockets[idx].sock;
    
    if (epoll_ctl(sev->pdata[work_reactor].epfd, EPOLL_CTL_MOD, sock, &ev) < 0) 
    {
        Log :: WARN("MOD EPOLL EVNENT WRITE TO REACTOR ERROR: %d STR:%s", 
                    work_reactor, strerror(errno));
        return -1;
    }
    Log :: NOTICE("SUCC MOD EPOLL EVNENT WRITE TO REACTOR : %d, FD : %d", 
                work_reactor, sock);
    return 0;
}

int 
RaServer :: rapool_epoll_del(nf_server_t * sev, int idx, int work_reactor)
{
    rapool_t *pool = (rapool_t *) sev->pool;
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    
    ev.data.fd = idx;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    int sock = pool->sockets[idx].sock;
    int epfd = sev->pdata[work_reactor].epfd;
    
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, sock, &ev) < 0) 
    {
        Log :: WARN("del socket error: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int 
RaServer :: rapool_del(nf_server_t *sev, int idx, int alive, bool remove)
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

        if(pool->sockets[idx].rp.cache != NULL)
            Allocate :: deallocate(pool->sockets[idx].rp.cache, 
                                   pool->sockets[idx].rp.cache_len);
        pool->sockets[idx].rp.cache = NULL;
        pool->sockets[idx].rp.cache_len = 0; 
 
        if(pool->sockets[idx].rp.w_cache != NULL)
            Allocate :: deallocate(pool->sockets[idx].rp.w_allo_cache, 
                                   pool->sockets[idx].rp.w_allo_len);
        pool->sockets[idx].rp.w_cache = NULL;
        pool->sockets[idx].rp.w_allo_cache = NULL;
        pool->sockets[idx].rp.w_cache_len = 0; 
        pool->sockets[idx].rp.w_allo_len = 0; 
        
        pool->sockets[idx].sock = -1;
        pool->sockets[idx].status = IDLE;
    } 
    return 0;
}


void * 
RaServer :: rapool_workers(void * param)
{
    
    nf_server_pdata_t *pdata = (nf_server_pdata_t *) param;
    nf_server_t * sev = (nf_server_t *) pdata->server;

    set_pthread_data(pdata);

    while (sev->run) 
    {
        rapool_reactor((rapool_t *) sev->pool, pdata);
    }
    
    pthread_key_del();
    pthread_exit(NULL);
    return NULL;
}

int 
RaServer :: rapool_reactor(rapool_t * pool, nf_server_pdata_t * pdata)
{
    //int idx;
    nf_server_t * sev = (nf_server_t *)pdata->server;
   
    //sev->cb_work(pdata);
    sev->stratgy->work(pdata);    

    if(!sev->run) 
    {
        close(pdata->epfd);
    }
    return 0;
}

int 
RaServer :: svr_pause()
{
    if(sev_data->status != RUNNING) 
    {
        std :: cout << "Because server's status isn't running, rapool_pause Failed!" 
        << std::endl;
        return -1;
    }

    rapool_t * pool = (rapool_t *) sev_data->pool;
    int index = 0;
    if(rapool_epoll_del(sev_data, index, 0) != 0) 
    {
        return -1;
    }
    pool->sockets[index].status = IDLE;
    pool->sockets[index].sock = -1;
    sev_data->status = PAUSE;
    return 0;
}

int 
RaServer :: svr_resume()
{
    if(sev_data->status != PAUSE) 
    {
        std::cout << "Because server's status isn't PAUSE, rapool_resume Failed!" 
        <<std::endl;
        return -1;
    }

    if(add_listen_socket(sev_data, sev_data->sev_socket) != 0) 
    {
        return -1;
    }

    sev_data->status = RUNNING;
    return 0;
}

int 
RaServer :: call_back_timeout(void * param)
{
    rapool_sock_item_t * sock_item = (rapool_sock_item_t *) param;
    int id = nf_server_get_thread_id(); 
    Log :: WARN("====TIME OUT FD : %d REACOTR %d WILL CLEAN ===", 
                sock_item->sock, id);
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    
    ev.data.fd = -1;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLOUT;
    int epfd = nf_server_get_thread_epfd();
   
    if(sock_item->sock < 0)
        return 0;
 
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, sock_item->sock, &ev) < 0) 
    {
        Log :: WARN("TIMEOUT DEL EPOLL SOCK TIMEOUT : %s", 
                    strerror(errno));
    }
    
    //CLEAN CONNECTION POOL    
    if (sock_item->sock >= 0) 
        close(sock_item->sock);
       
    if(sock_item->rp.cache != NULL)
        Allocate :: deallocate(sock_item->rp.cache, 
                               sock_item->rp.cache_len);
    sock_item->rp.cache = NULL;
    sock_item->rp.cache_len = 0; 
 
    if(sock_item->rp.w_cache != NULL)
        Allocate :: deallocate(sock_item->rp.w_allo_cache, 
                               sock_item->rp.w_allo_len);
    sock_item->rp.w_cache = NULL;
    sock_item->rp.w_allo_cache = NULL; 
    sock_item->rp.w_cache_len = 0; 
    sock_item->rp.w_allo_len = 0; 
        
    sock_item->sock = -1;
    sock_item->status = IDLE;

    return 0;
}

int
RaServer :: svr_set_stragy(BaseWork * sta)
{
    if(sta == NULL)
    {
        sev_data->stratgy = new RaReadLine();
        return 0; 
    }
    RaBaseWork * test = dynamic_cast<RaBaseWork *>(sta);
    assert(test != NULL);
    sev_data->stratgy = test;
             
    return 0;
}

