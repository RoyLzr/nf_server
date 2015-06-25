#include "sapool.h"

enum 
{
    DEFAULT_CHECK_INTERVAL = 10,  ///Ä¬ÈÏcheck_interval, 10ms
    DEFAULT_TIMEOUT = 60,         ///µÈ´ý¶ÓÁÐÀïÃæµÄÄ¬ÈÏ³¬Ê±Ê±¼ät, 60s
    DEFAULT_QUEUE_LEN = 100,      ///Ä¬ÈÏµÄµÈ´ý¶ÓÁÐµÄ³¤¶È 100
    DEFAULT_SOCK_NUM = 500,       ///Í¬Ê±Î¬»¤µÄ×î´ósock¾ä±úÊý£¬500
};

enum 
{
    IDLE = 0,
    READY,
    BUSY
};

#define MAX(a,b) (((a)>(b)) ? (a) : (b))

static int 
add_listen_socket(nf_server_t *sev, int listenfd)
{
    sapool_t *pool = (sapool_t *)sev->pool;
    int idx = 0;
    //¼àÌý¾ä±úÓÀÔ¶´¦ÓÚbusy×´Ì¬ÕâÑù²»»Ø±»¹Ø±Õ
    pool->sockets[idx].status = BUSY;
    pool->sockets[idx].sock = listenfd;

    char tmp[] = "0.0.0.0";
    set_tcp_sockaddr(tmp, 0, &(pool->sockets[idx].addr));

    return sapool_epoll_add(sev, idx);
}

int 
sapool_init(nf_server_t *sev)
{
    int qsiz = nf_server_get_qsize(sev);
    int ssiz = nf_server_get_socksize(sev);
    sapool_t *pool = NULL;

    if (qsiz <=1 || ssiz<=0) 
    {
        qsiz = DEFAULT_QUEUE_LEN;
        ssiz = DEFAULT_SOCK_NUM;
        sev->qsize = qsiz;
        sev->socksize = ssiz;
    }

    if (sev->pool == NULL) 
    {
        sev->pool = malloc(sizeof(sapool_t));
        if (sev->pool == NULL) 
        {
            std :: cout << "malloc sapool error" << std::endl;
            
            return -1;
        }
        memset(sev->pool, 0, sizeof(sapool_t));
    }
    
    pool = (sapool_t *) sev->pool;
    pthread_mutex_init(&pool->ready_mutex, NULL);
    pthread_cond_init(&pool->ready_cond, NULL);

    //´´½¨¾ÍÐ÷¶ÓÁÐ
    if (create_q(&pool->queue, qsiz) != 0) 
    {
        std :: cout << "error of create queue" << std :: endl;
        return -1;
    }

    //´´½¨socket×ÊÔ´
    pool->size = ssiz;
    pool->sockets = (sapool_sock_item_t *) malloc (sizeof(sapool_sock_item_t) * ssiz);
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
        std :: cout << "error of create sockets" << std :: endl;
        return -1;
    }

    //´´½¨epoll×ÊÔ´
    pool->ep_events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * ssiz);
    if (pool->ep_events == NULL) 
    {
        std :: cout << "error of create epoll events" << std :: endl;
        return -1;
    }
    memset(pool->ep_events, 0, ssiz * sizeof(struct epoll_event));

    //´´½¨epoll¾ä±ú
    pool->epfd = epoll_create(ssiz);
    if (pool->epfd < 0) 
    {
        std :: cout << "error of create epoll handle" << std :: endl;
        return -1;
    }

    pool->timeout = -1;
    if (pool->timeout <= 0) 
    {
        //pool->timeout = DEFAULT_TIMEOUT;
        pool->timeout = sev->timeout;
    }
    //pool->check_interval = DEFAULT_CHECK_INTERVAL;
    pool->check_interval = sev->check_interval;
    pool->run = &sev->run;
    pool->using_size = 0;

    return 0;
}

long long 
sapool_get_queuenum(nf_server_t *sev)
{
    if (sev == NULL) 
        return -1;

    sapool_t * pool = (sapool_t *) sev->pool;
    if (pool == NULL) 
        return -1;

    return (long long)pool->queue.size;
}

int 
sapool_run(nf_server_t *sev)
{
    int i = 0;
    int ret = 0;
    sapool_t *pool = (sapool_t *)sev->pool;

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
                         sapool_main, &sev->pdata[0]);
    sev->pdata[0].pid = pool->main;

    if (ret) 
    {
        std :: cout << "error thread set create main thread" << std :: endl;
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
    for (i=1; i<sev->pthread_num; ++i) 
    {
        sev->pdata[i].id = i;
        ret = pthread_create(&sev->pdata[i].pid, &thread_attr, 
                             sapool_workers, &sev->pdata[i]);
        if (ret) 
        {
            std :: cout << "error thread set create work thread" << std :: endl;
            return -1;
        }
        ++ sev->run_thread_num;
    }

    sev->status = RUNNING;
    pthread_attr_destroy(&thread_attr);
    return 0;
}

int 
sapool_listen(nf_server_t *sev)
{
    return add_listen_socket(sev, sev->sev_socket);
}

int 
sapool_join(nf_server_t *sev)
{
    sapool_t *pool = (sapool_t *)sev->pool;
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
sapool_close_pool_sockets(nf_server_t *sev, bool is_listenfd)
{
    sapool_t * pool = (sapool_t *) sev->pool;
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
    //Çå¿Õ ÊÂ¼þ ¶ÓÁÐ, ¶à¸ö worker ÐèÒªlock
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

int sapool_destroy(nf_server_t *sev)
{
    sapool_t *pool = (sapool_t *)sev->pool;
    if (pool == NULL) 
        return 0;

    //´ËÊ±Ïû·ÑÏß³Ì¶¼ÒÑÍË³ö£¬¾Í´Ë¹Ø±ÕËùÓÐÒÑÁ¬½Ósockfd
    sapool_close_pool_sockets(sev, false);

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

    free (sev->pool);
    sev->pool = NULL;
    return 0;
}

int check_socket_queue(nf_server_t *sev)
{
    sapool_t *pool = (sapool_t *)sev->pool;
    for (u_int i=0; i< pool->size; ++i) 
    {
        if (pool->sockets[i].status != IDLE) 
        {
            switch(pool->sockets[i].status)
            {
                case (READY):
                Log :: NOTICE("CONNECTION POOL IS NOT EMPTY tid : %d idx : %d READY ", pthread_self(), i);
                    break;
                case (BUSY):
                Log :: NOTICE("CONNECTION POOL IS NOT EMPTY tid : %d idx : %d BUSY ", pthread_self(), i);
                    break;
            }
            return -1;
        }
    }
    Log :: NOTICE("CONNECTION POOL IS EMPTY pid : %d ", pthread_self());
    return 0;
}

void * 
sapool_main(void *param)
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
sapool_check_timeout(nf_server_t *sev)
{
    sapool_t * pool = (sapool_t *) sev->pool;
    time_t curtime = time(NULL);
    if (curtime < pool->next_check_time) 
    {
        return 0;
    }
    
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
                    std :: cout << "socket pool timeout : " << i << std :: endl;
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
sapool_produce(nf_server_t * sev, struct sockaddr * addr, 
                   socklen_t * addrlen)
{
    int ret = 0;
    sapool_t *pool = (sapool_t *) sev->pool;
    int sev_sock = sev->sev_socket;
    char ip[40];
    int len = 40;
    int port;
    
    sapool_check_timeout(sev);

    int num = epoll_wait(pool->epfd, pool->ep_events, pool->size, pool->check_interval);

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
            //¾ä±ú¶Ï¿ª
            if (pool->ep_events[i].events & EPOLLHUP) 
            {
                sapool_del(sev, idx, 0, true);
                pool->using_size --;
                std :: cout << "fin catch by produce loop" << std :: endl;
            } 
            else if (pool->ep_events[i].events & EPOLLERR) 
            {
                //¾ä±ú³ö´í
                sapool_del(sev, idx, 0, true);
                pool->using_size --;
                std :: cout << "fd EPOLLERR catch by produce loop" << std :: endl;
            } 
            else if (pool->ep_events[i].events & EPOLLIN) 
            {
                //¾ä±ú¿É¶Á
                sapool_epoll_del(sev, idx);    //½«¾ä±ú´ÓepollÖÐÒÆ³ý
                //½«¾ä±ú¼ÓÈëµ½¾ÍÐ÷¶ÓÁÐ
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
sapool_add(nf_server_t * sev, int sock, struct sockaddr_in *addr)
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
    pool->sockets[idx].last_active = time(NULL);
    pool->sockets[idx].addr = *addr;
    pool->sockets[idx].sock = sock;
    pool->using_size++;

    if (sapool_epoll_add(sev, idx) != 0) 
    {
        return -1;
    }
    return idx;
}

int 
sapool_epoll_add(nf_server_t *sev, int idx)
{
    sapool_t *pool = (sapool_t *) sev->pool;
    struct epoll_event ev;
    ev.data.fd = idx;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    int sock = pool->sockets[idx].sock;
    
    if (epoll_ctl(pool->epfd, EPOLL_CTL_ADD, sock, &ev) < 0) 
    {
        std :: cout << "add socket error: " << strerror(errno) << std :: endl;
        return -1;
    }
    return 0;
}

int 
sapool_epoll_del(nf_server_t *sev, int idx)
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
sapool_del(nf_server_t *sev, int idx, int alive, bool remove)
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
        pool->sockets[idx].last_active = time(NULL);
        pool->sockets[idx].status = READY;
        sapool_epoll_add(sev, idx);
    }
    return 0;
}

int 
sapool_put(sapool_t *pool, int idx)
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
sapool_workers(void * param)
{
    nf_server_pdata_t *pdata = (nf_server_pdata_t *) param;
    nf_server_t * sev = (nf_server_t *) pdata->server;

    set_pthread_data(pdata);

    while (sev->run) 
    {
        sapool_consume((sapool_t *) sev->pool, pdata);
        if(!sev->run) 
        {
            //ÔËÐÐ Íê queue Êý¾Ý
            while(check_socket_queue(sev) != 0)
                sapool_consume((sapool_t *) sev->pool, pdata);
        }
    }

    pthread_exit(NULL);
    return NULL;
}

int 
sapool_pthread_cond_timewait(sapool_t *pool)
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
sapool_get(nf_server_t *sev, int *idx)
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
sapool_consume(sapool_t * pool, nf_server_pdata_t * pdata)
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

    pdata->epfd = net_ep_create(1);
    if(pdata->epfd < 0)
    {
        std::cout << strerror(errno) << std::endl;
    }      

    int ret = sev->cb_work(pdata);
    
    if (sev->run && ret == 0) 
    {
        close(pdata->epfd);
        sapool_del(sev, idx, 1);
        return 0;
    } 
    else if (!sev->run) 
    {
        //server stop; handle left dta in socket
        shutdown(pdata->fd, SHUT_WR);
        for(;sev->cb_work(pdata) == 0;);
            close(pdata->epfd);
    }
    
    //ret < 0 Ê±,close fd, ÒòÎªÊÂ¼þÒÑ¾­´Ó epollÖÐÒÆ³ý
    //´Ë´¦ ²»ÓÃÔÙ´Î ÒÆ³ý
    EXIT:
    if(pdata->epfd > 0)
        close(pdata->epfd);
    sapool_del(sev, idx, 0);
    pdata->fd = -1;
    return 0;
}

int 
sapool_pause(nf_server_t *sev)
{
    if(sev->status != RUNNING) 
    {
        std :: cout << "Because server's status isn't running, sapool_pause Failed!" 
        << std::endl;
        return -1;
    }

    sapool_t * pool = (sapool_t *) sev->pool;
    int index = 0;
    if(sapool_epoll_del(sev, index) != 0) 
    {
        return -1;
    }
    pool->sockets[index].status = IDLE;
    pool->sockets[index].sock = -1;
    sev->status = PAUSE;
    return 0;
}

int sapool_resume(nf_server_t *sev)
{
    if(sev->status != PAUSE) 
    {
        std::cout << "Because server's status isn't PAUSE, sapool_resume Failed!" 
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

