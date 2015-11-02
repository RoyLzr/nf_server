#include "sapool.h"

enum 
{
    DEFAULT_CHECK_INTERVAL = 10,  ///默认check_interval, 10ms
    DEFAULT_TIMEOUT = 60000,         ///等待队列里面的默认超时时间t, 60s
    DEFAULT_QUEUE_LEN = 100,      ///默认的等待队列的长度 100
    DEFAULT_SOCK_NUM = 500,       ///同时维护的最大sock句柄数，500
};

enum 
{
    IDLE = 0,
    READY,
    BUSY
};

#define MAX(a,b) (((a)>(b)) ? (a) : (b))

int 
SaServer :: add_listen_socket(nf_server_t *sev, int listenfd)
{
    sapool_t *pool = (sapool_t *)sev->pool;
    int idx = 0;
    //监听句柄永远处于busy状态这样不回被关闭
    pool->sockets[idx].status = BUSY;
    pool->sockets[idx].sock = listenfd;

    char tmp[] = "0.0.0.0";
    set_tcp_sockaddr(tmp, 0, &(pool->sockets[idx].addr));

    return sapool_epoll_add(sev, idx);
}

int 
SaServer :: svr_init()
{
    ReadEvent * r_ev = new ReadEvent();
    WriteEvent * w_ev = new WriteEvent();
    
    r_ev->init(1, sev->read_handle, sev->read_parse_handle);
    w_ev->init(1, sev->write_handle, sev->write_parse_handle);
   
    sev->svr_reactor->add_event(r_ev);
    sev->svr_reactor->add_event(w_ev);

}

long long 
SaServer :: sapool_get_queuenum(nf_server_t *sev)
{
    if (sev == NULL) 
        return -1;

    sapool_t * pool = (sapool_t *) sev->pool;
    if (pool == NULL) 
        return -1;

    return (long long)pool->queue.size;
}

int 
SaServer :: svr_run()
{
    //int i = 0;
    int ret = 0;
    sapool_t *pool = (sapool_t *)sev_data->pool;

    pthread_attr_t thread_attr;
    struct sched_param param = { 0 };
    ret = pthread_attr_init(&thread_attr);
    if (ret) 
    {
        std :: cout << "error thread attr init" << std :: endl;
        return -1;
    }
    
    /*
    if (sev->stack_size > 0) 
    {
        ret = pthread_attr_setstacksize(&thread_attr, sev->stack_size);
        if (ret) 
        {
            std :: cout << "error thread set stacksize" << std :: endl;
            return -1;
        }
    }
    */

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
                         sapool_main, &sev_data->pdata[0]);
    sev_data->pdata[0].pid = pool->main;

    if (ret) 
    {
        std :: cout << "error thread set create main thread" << std :: endl;
        return -1;
    }

    sev_data->run_thread_num = 0;
    param.sched_priority = WORKER_PRIORITY;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if (ret) 
    {
        std :: cout << "error thread set work priority" << std :: endl;
        return -1;
    }

    //创建逻辑处理子线程
    for (size_t i=1; i < sev_data->pthread_num; ++i) 
    {
        sev_data->pdata[i].id = i;
        ret = pthread_create(&sev_data->pdata[i].pid, &thread_attr, 
                             sapool_workers, &sev_data->pdata[i]);
        if (ret) 
        {
            std :: cout << "error thread set create work thread" << std :: endl;
            return -1;
        }
        ++ sev_data->run_thread_num;
    }

    sev_data->status = RUNNING;
    pthread_attr_destroy(&thread_attr);
    return 0;
}

int 
SaServer :: svr_listen()
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
    //listen 函数为空
    //return svr_listen(sev);

    return add_listen_socket(sev_data, sev_data->sev_socket);
}

int 
SaServer :: svr_join()
{
    sapool_t *pool = (sapool_t *)sev_data->pool;
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
SaServer :: sapool_close_pool_sockets(nf_server_t *sev, bool is_listenfd)
{
    sapool_t * pool = (sapool_t *) sev->pool;
    //is_listenfd是true时，生产线程调用时，设置is_listenfd=true，先关闭监听sockfd
    //清空 listen 的 socket
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
    //清空 事件 队列, 多个 worker 需要lock
    pthread_mutex_lock(&pool->ready_mutex);
    while (!is_empty_q(&pool->queue)) 
    {
        int hd = 0;
        pop_q(&pool->queue, &hd);
        close(pool->sockets[hd].sock);
        pool->sockets[hd].sock = -1;
        pool->sockets[hd].status = IDLE;
    }
    pthread_mutex_unlock(&pool->ready_mutex);
    
    //清空 pending socket pool中 的 内容, pending socket �
    //一个 produce 操作， 不需要 锁
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

int 
SaServer :: svr_destroy()
{
    sapool_t *pool = (sapool_t *)sev_data->pool;
    if (pool == NULL) 
        return 0;

    //此时消费线程都已退出，就此关闭所有已连接sockfd
    sapool_close_pool_sockets(sev_data, false);

    pthread_mutex_destroy(&pool->ready_mutex);
    pthread_cond_destroy(&pool->ready_cond);
    destroy_q(&pool->queue);

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

int 
SaServer :: check_socket_queue(nf_server_t *sev)
{
    sapool_t *pool = (sapool_t *)sev->pool;
    if(pool == NULL)
        return 0;
    for (int i=0; i< pool->size; ++i) 
    {
        if (pool->sockets[i].status != IDLE) 
        {
            switch(pool->sockets[i].status)
            {
                case (READY):
                Log :: NOTICE("CONNECTION POOL IS NOT EMPTY tid : %lu idx : %d READY ", pthread_self(), i);
                    break;
                case (BUSY):
                Log :: NOTICE("CONNECTION POOL IS NOT EMPTY tid : %lu idx : %d BUSY ", pthread_self(), i);
                    break;
            }
            return -1;
        }
    }
    Log :: NOTICE("CONNECTION POOL IS EMPTY tid : %lu ", pthread_self());
    return 0;
}

void * 
SaServer :: sapool_main(void *param)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) param;
    nf_server_t * sev = (nf_server_t *) pdata->server;
    set_pthread_data(pdata);

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    set_fd_noblock(sev->sev_socket);
    
    while (sev->run) 
    {
        sapool_produce(sev, (struct sockaddr *) &addr, &addr_len);
    }

    sapool_close_pool_sockets(sev, true);

    while(check_socket_queue(sev) != 0) 
    {
        Log :: WARN("SVR STOPPED CONNECTION POOL IS NOT EMPTY ");
        sapool_produce(sev, (struct sockaddr *)&addr, &addr_len);
    }

    pthread_exit(NULL);
    return NULL;
}

int 
SaServer :: sapool_check_timeout(nf_server_t *sev)
{
    sapool_t * pool = (sapool_t *) sev->pool;
    time_t curtime = time(NULL);
    curtime *= 1000;

    if (curtime < pool->next_check_time) 
    {
        return 0;
    }
    
    Log :: NOTICE("SA SVR WILL CHECK TIMEOUT SOCKETS");
    pool->next_check_time = curtime + pool->timeout;
    time_t tmp = 0;
    
    for (int i=0; i< pool->size; ++i) 
    {
        switch (pool->sockets[i].status) 
        {
            case READY:
                if (!sev->run) 
                {
                    sapool_del(sev, i, 0, true);
                    pool->using_size --;
                    break;
                }
                tmp = pool->sockets[i].last_active + pool->timeout;
                if (curtime >= tmp) 
                {
                    sapool_del(sev, i, 0, true);
                    pool->using_size --;
                    Log :: WARN("=====SOCKET POOL TIMEOUT idx : %d fd : %d: ", 
                                i, pool->sockets[i].sock);
                } 
                else if (pool->next_check_time > tmp) 
                {
                    pool->next_check_time = tmp;
                }
                break;
            case BUSY:
                pool->sockets[i].last_active = curtime;
                break;
        }
    }
    return 0;
}

int 
SaServer :: sapool_produce(nf_server_t * sev, struct sockaddr * addr, 
                   socklen_t * addrlen)
{
    int ret = 0;
    sapool_t *pool = (sapool_t *) sev->pool;
    int sev_sock = sev->sev_socket;
    char ip[40];
    int len = 40;
    int port;
    
    sapool_check_timeout(sev);

    int num = epoll_wait(pool->epfd, pool->ep_events, pool->size, pool->timeout);

    if (num <= 0) 
    {
        return ret;
    }
    for (int i=0; i<num; ++i) 
    {
        //listen port
        if (pool->ep_events[i].data.fd == 0) 
        {
            int cli_sock = net_accept(sev_sock, addr, addrlen);
            if (cli_sock < 0) 
            {
                Log :: WARN("ACCEPT %d ERROR ", sev_sock);
                continue;
            }
            
            get_tcp_sockaddr(ip, &port, (sockaddr_in *)addr, len);
            Log :: NOTICE("ACCEPT SUCC FROM CLIENT: %s:%d  new fd : %d ", 
                           ip, port, cli_sock);

            set_sev_socketopt(sev, cli_sock);

            if (sapool_add(sev, cli_sock, (struct sockaddr_in *) addr) < 0) 
            {
                close(cli_sock);
                ret = -1;
            }
        } 
        else if (pool->ep_events[i].data.fd > 0) 
        {
            int idx = pool->ep_events[i].data.fd;
            //句柄断开
            if (pool->ep_events[i].events & EPOLLHUP) 
            {
                sapool_del(sev, idx, 0, true);
                pool->using_size --;
                std :: cout << "fin catch by produce loop" << std :: endl;
            } 
            else if (pool->ep_events[i].events & EPOLLERR) 
            {
                //句柄出错
                sapool_del(sev, idx, 0, true);
                pool->using_size --;
                std :: cout << "fd EPOLLERR catch by produce loop" << std :: endl;
            } 
            else if (pool->ep_events[i].events & EPOLLIN) 
            {
                //句柄可读
                sapool_epoll_del(sev, idx);    //将句柄从epoll中移除
                //将句柄加入到就绪队列
                if (sapool_put(pool, idx) != 0) 
                {
                    sapool_del(sev, idx, 0, false);
                    pool->using_size --;
                    std :: cout << "Because the queue(size=queuesize) is full, then close the connection" 
                    << std::endl;
                }
            } 
            else 
            {
                sapool_del(sev, idx, 0, true);
                pool->using_size --;
            }
        }
    }
    return ret;
}

int 
SaServer :: sapool_add(nf_server_t * sev, int sock, struct sockaddr_in *addr)
{
    sapool_t *pool = (sapool_t *) sev->pool;
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
        std :: cout << "Because the array(size=socksize) is full, so close the new connection"
         << std :: endl;
        return -1;
    }

    pool->sockets[idx].status = READY;
    pool->sockets[idx].last_active = time(NULL) * 1000;
    pool->sockets[idx].addr = *addr;
    pool->sockets[idx].sock = sock;
    pool->using_size++;
    
    Log :: NOTICE("ADD FD TO POOL fd : %d, pos : %d", sock, idx);
    if (sapool_epoll_add(sev, idx) != 0) 
    {
        return -1;
    }
    return idx;
}

int 
SaServer :: sapool_epoll_add(nf_server_t *sev, int idx)
{
    sapool_t *pool = (sapool_t *) sev->pool;
    struct epoll_event ev;
    ev.data.fd = idx;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    int sock = pool->sockets[idx].sock;
    
    if (epoll_ctl(pool->epfd, EPOLL_CTL_ADD, sock, &ev) < 0) 
    {
        Log :: WARN("ADD SOCKET TO REACTOR ERROR: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int 
SaServer :: sapool_epoll_del(nf_server_t *sev, int idx)
{
    sapool_t *pool = (sapool_t *) sev->pool;
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
SaServer :: sapool_del(nf_server_t *sev, int idx, int alive, bool remove)
{
    sapool_t * pool = (sapool_t *) sev->pool;
    if (alive == 0) 
    {
        if(remove)
            sapool_epoll_del(sev, idx);  // remove it from epoll event sets
        if (pool->sockets[idx].sock >= 0) 
            close(pool->sockets[idx].sock);

        pool->sockets[idx].sock = -1;
        pool->sockets[idx].status = IDLE;

        if(pool->sockets[idx].rp.cache != NULL)
            Allocate :: deallocate(pool->sockets[idx].rp.cache, 
                                   pool->sockets[idx].rp.cache_len);
        pool->sockets[idx].rp.cache = NULL;
        pool->sockets[idx].rp.cache_len = 0; 
    } 
    else 
    {
        pool->sockets[idx].last_active = time(NULL) * 1000;
        pool->sockets[idx].status = READY;
        sapool_epoll_add(sev, idx);
    }
    return 0;
}

int 
SaServer :: sapool_put(sapool_t *pool, int idx)
{
    pthread_mutex_lock(&pool->ready_mutex);
    if (is_full_q(&pool->queue)) 
    {
        pthread_mutex_unlock(&pool->ready_mutex);
        return -1;
    }
    push_q(&pool->queue, idx);

    pool->sockets[idx].status = BUSY;
    pthread_cond_signal(&pool->ready_cond);
    pthread_mutex_unlock(&pool->ready_mutex);
    return 0;
}

void * 
SaServer :: sapool_workers(void * param)
{
    nf_server_pdata_t *pdata = (nf_server_pdata_t *) param;
    nf_server_t * sev = (nf_server_t *) pdata->server;

    set_pthread_data(pdata);

    while (sev->run) 
    {
        sapool_consume((sapool_t *) sev->pool, pdata);
        if(!sev->run) 
        {
            //运行 完 queue 数据
            while(check_socket_queue(sev) != 0)
                sapool_consume((sapool_t *) sev->pool, pdata);
        }
    }

    pthread_exit(NULL);
    return NULL;
}

int 
SaServer :: sapool_pthread_cond_timewait(sapool_t *pool)
{
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, 0);
    timeout.tv_sec = now.tv_sec + 5;
    timeout.tv_nsec = now.tv_usec * 1000;
    pthread_cond_timedwait(&pool->ready_cond, &pool->ready_mutex, &timeout);
    return 0;
}

int 
SaServer :: sapool_get(nf_server_t *sev, int *idx)
{
    sapool_t * pool = (sapool_t *) sev->pool;
    pthread_mutex_lock(&pool->ready_mutex);

    while (is_empty_q(&pool->queue) && sev->run) 
    {
        sapool_pthread_cond_timewait(pool);
    }

    if (is_empty_q(&pool->queue)) 
    {
        pthread_mutex_unlock(&pool->ready_mutex);
        return -1;
    }

    pop_q(&pool->queue, idx);
    pthread_mutex_unlock(&pool->ready_mutex);

    return 0;
}

int 
SaServer :: sapool_consume(sapool_t * pool, nf_server_pdata_t * pdata)
{
    int idx;
    nf_server_t * sev = (nf_server_t *)pdata->server;
    if (sapool_get(sev, &idx) != 0) 
    {
        return 0;
    }

    pdata->fd = pool->sockets[idx].sock;
    pdata->client_addr = pool->sockets[idx].addr;
    pdata->idx = idx;

    int ret = sev->stratgy->work(pdata);
    
    if (sev->run && ret == 0) 
    {
        sapool_del(sev, idx, 1);
        return 0;
    } 
    else if (!sev->run) 
    {
        //server stop; handle left dta in socket
        shutdown(pdata->fd, SHUT_WR);
        for(;sev->stratgy->work(pdata) == 0;);
    }
    
    //ret < 0 时,close fd, 因为事件已经从 epoll中移除
    //此处 不用再次 移除
    sapool_del(sev, idx, 0);
    pdata->fd = -1;
    return 0;
}

int 
SaServer :: svr_pause()
{
    if(sev_data->status != RUNNING) 
    {
        std :: cout << "Because server's status isn't running, sapool_pause Failed!" 
        << std::endl;
        return -1;
    }

    sapool_t * pool = (sapool_t *) sev_data->pool;
    int index = 0;
    if(sapool_epoll_del(sev_data, index) != 0) 
    {
        return -1;
    }
    pool->sockets[index].status = IDLE;
    pool->sockets[index].sock = -1;
    sev_data->status = PAUSE;
    return 0;
}

int 
SaServer :: svr_resume()
{
    if(sev_data->status != PAUSE) 
    {
        std::cout << "Because server's status isn't PAUSE, sapool_resume Failed!" 
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
SaServer :: svr_set_stragy( BaseWork * sta)
{
    if(sta == NULL)
    {
        sev_data->stratgy = new SaReadLine();
        return 0; 
    }
    SaBaseWork * test = dynamic_cast<SaBaseWork *>(sta);
    assert(test != NULL);
    sev_data->stratgy = test;
             
    return 0;
}

