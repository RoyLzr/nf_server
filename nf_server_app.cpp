//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  work thread 使用的处理，应用函数，用户可设计自己的处理方法
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#include "sapool.h"
#include "rapool.h"
#include "lfpool.cpp"
#include "nf_server_app.h"

enum
{
    IDLE = 0
};

int 
LfReadLine :: work(void * data)
{
    Log :: NOTICE("LF READ LINE WORK");
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) data;

    char * req = (char *) pdata->read_buf;
    char * res = (char *) pdata->write_buf;
    int epfd = pdata->epfd;
    int fd = pdata->fd;
    int readsize = pdata->read_size;   
    //int writesize = pdata->write_size;   
    nf_server_t *sev = pdata->server; 
     
    pdata->rio.rio_fd = pdata->fd;
    pdata->rio.rio_cnt = 0;
    pdata->rio.rio_bufptr = pdata->rio.rio_ptr; 
     
    int ret;
    if((ret = net_ep_add_in(epfd, fd)) < 0)
    {
        Log :: WARN("add epoll error");
        return -1;
    }      

    int readto = nf_server_get_readto();
    int writeto = nf_server_get_writeto();
     
    struct epoll_event events[1];
    
    //event loop
    while(sev->run)
    {
        ret = epoll_wait(pdata->epfd, events, 1, readto * 20);
        if(ret == 0)
        {
            Log :: WARN("APP EOPLL WAIT read timeout error"); 
            return -1;
        }
        else if(ret < 0)
        {
            Log :: WARN("APP epoll wait error : %s", strerror(errno));
            return -1;
        } 
        //events analyse
        
        if(events[0].events & EPOLLRDHUP)
        { 
            Log :: WARN("APP event close fd error HUP");
            return -1;
        }
        else if(events[0].events & EPOLLERR )
        {
            Log :: WARN("APP event fd error ERR");
            return -1;
        }
        else if( events[0].events & EPOLLIN )
        {
            int n;
            while((n = rio_readline_to_ms(&pdata->rio, req, readsize, readto)) > 0)   
            {
                Log :: DEBUG("APP READ DATA IS LINE : %d", n);
                pdata->readed_size = n;
                sev->p_handle();
                if((n = sendn_to_ms(pdata->rio.rio_fd, res, pdata->writed_size, writeto))< 0)
                {
                    Log :: WARN ("write data error : %s",strerror(errno));
                        return -1;
                }
                pdata->read_start = 0;
                pdata->readed_size = 0;
                pdata->write_start = 0;
                pdata->writed_size = 0; 

                Log :: DEBUG("APP SEND DATA IS LINE SUCC: %d", n);
            }
        //离开 循环读 情况：
        //1. send error, 包括 TIMEOUT 敝苯� � return 
        //2. read error, 如果是 fin 或者 其余读到其他错误 return
        //3. read error, 如果是 TIMEOUT
        //   (1) 上次裷ead读到一行，处理后，但是下次数据一直没有来. 
        //        event loop 进行更长时间超时等待数据
        //   (2) 上次 read读到半行，等待读到回车符，等待过程中 发生超时，读不到完整一行。return.
            if(n == 0|| errno != ETIMEDOUT )
            {
                if( n == 0)
                    Log :: WARN("recv fin");
                else 
                    Log :: WARN("read error : %s",strerror(errno) );
                return -1;
            }
            if(errno == ETIMEDOUT && n != -1)
                return -1;
        }
    }
    return 0;
}

//SA model read line support:
//1. rio rio_ptr 读数据缓存区 = 
//2. rio cache   存储状态buf, 防止读事件发生阻塞，非阻塞操作
//               一旦读空，存储状态，释放thread，thread
//               获取queue新事件处理
//3. rio cache   获取采用内存池代替malloc

int 
SaReadLine :: work(void * data)
{
    Log :: NOTICE("APP SA READLINE WORK");
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) data;
    nf_server_t * sev = (nf_server_t *) pdata->server;
    sapool_t * pool = (sapool_t *) sev->pool;    

    char * req = (char *) pdata->read_buf;
    char * res = (char *) pdata->write_buf;
    int epfd = pdata->epfd;
    int fd = pdata->fd;
    int readsize = pdata->read_size;   
    //int writesize = pdata->write_size;   
    int idx = pdata->idx;    
    rio_t * rp = &(pool->sockets[idx].rp); 

    //清空 读数据的 缓存区
    rp->rio_fd = pdata->fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_ptr; 
     
    int ret;
    if((ret = net_ep_add_in(epfd, fd)) < 0)
    {
        Log :: WARN("add epoll error");
        return -1;
    }      

    //int readto = nf_server_get_readto();
    int writeto = nf_server_get_writeto();
     
    struct epoll_event events[1];
    
    //event loop
    while(sev->run)
    {
        ret = epoll_wait(pdata->epfd, events, 1, 1000);
        if(ret == 0)
        {
            Log :: WARN("read timeout error"); 
            return -1;
        }
        else if(ret < 0)
        {
            Log :: WARN("epoll wait error : %s",strerror(errno));   
            return -1;
        } 
        //events analyse
        
        if(events[0].events & EPOLLRDHUP)
        { 
            Log :: WARN("event close fd error"); 
            return -1;
        }
        else if(events[0].events & EPOLLERR )
        {
            Log :: WARN("event fd  error"); 
            return -1;
        }
        else if( events[0].events & EPOLLIN )
        {
            int n;
            int st;
            int clen = 0;
            //恢复上次 socket 状态
            if(rp->cache != NULL && rp->cache_len > 0)
            {
                memcpy(req, rp->cache, rp->cache_len);
                clen = rp->cache_len;

                Log :: NOTICE("Back status of cache: %d", clen);
 
                Allocate :: deallocate(rp->cache, rp->cache_len);
                rp->cache_len = 0; 
                rp->cache = NULL;
            }
            //无等待时间的 非阻塞读，读空协议栈缓存，但数据凑不满一行时，
            //保存状态，释放线程处理别的事件。恶性肥荩ü�
            //检测清空`

            while((n = rio_readline(rp, req + clen, readsize - clen, &st)) > 0)   
            {
                Log :: DEBUG("APP READ LINE DATA : %d", n);
                if(st < 0)
                {
                    int len = clen + n -1;
                    if(len > 0)
                    {   
                        Log :: NOTICE("APP DUMP CACHE : %d", len );
                        rp->cache = (char *) Allocate :: allocate(len);
                    
                        rp->cache_len = len;
                        memcpy(rp->cache, req, len);
                        //return 0, 关闭pdata->epfd, socket 作为ready加入epoll
                    }
                    return 0;
                }
                pdata->readed_size = n + clen;
                clen = 0;
                sev->p_handle();
                if((n = sendn_to_ms(rp->rio_fd, res, pdata->writed_size, writeto))< 0)
                {
                    Log :: WARN("APP WRITE DATA ERROR: %s", strerror(errno));
                        return -1;
                }
                Log :: DEBUG("APP WRITE LINE DATA : %d", n);
                pdata->read_start = 0;
                pdata->readed_size = 0;
                pdata->write_start = 0;
                pdata->writed_size = 0; 
            }
            if(n == 0)
            {
                Log :: WARN( "APP RECV Fin" );
                return -1; 
            }
            else if(n < 0)
            {
                Log :: WARN(" APP recv error : ", strerror(errno));
                return -1; 
            }
        }
    }
    return 0;
}


int
RaReadLine :: work(void * data)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) data;

    char * req = (char *) pdata->read_buf;
    char * res = (char *) pdata->write_buf;
    //int epfd = pdata->epfd;
    int readsize = pdata->read_size;   
    //int writesize = pdata->write_size;   
    nf_server_t *sev = pdata->server;
    rapool_t * pool = (rapool_t *) sev->pool; 
 
    //int ret;
    //int readto = nf_server_get_readto();
    //int writeto = nf_server_get_writeto();
    int ssiz = nf_server_get_socksize(sev);
    
    struct epoll_event events[ssiz], ev;
    memset(&ev, 0, sizeof(ev));
    memset(events, 0, sizeof(events));
   
    pdata->write_start = 0;

    int timeout = (pdata->timer).top_timer_ms();
     
    //event loop
    while(sev->run)
    {
        Log :: DEBUG("TRACE WORK REACTOR TIMEOUT %d ", timeout);
        
        if(timeout == 0 || timeout > pool->check_interval)
            timeout = pool->check_interval;
        else if(timeout < 0)
            timeout = 0;
        
        int num = epoll_wait(pdata->epfd, events, ssiz, timeout);
        
        //handle timeout event 
        (pdata->timer).expire_timer_ms();
 
        if(num < 0)
        {
            Log :: WARN("nf_server_app: 331 PDATA ID : %d EPOLL WAIT ERROR: %s", 
                        pdata->id, strerror(errno));
            continue;
        }

        //events analyse
        for(int i = 0; i < num; i++)
        {
            int idx = events[i].data.fd;
            int sock = pool->sockets[idx].sock;
            rio_t * rp = &(pool->sockets[idx].rp);
            int sock_timeout = pool->sockets[idx].sock_timeout;
            
            if(pool->sockets[idx].status == IDLE || pool->sockets[idx].sock < 0)
                continue;
            
            Log :: DEBUG("EVENT LOOP Handle pool pos %d", idx);


            //清空 读数据的 缓存区
            rp->rio_fd = sock;
            rp->rio_cnt = 0;
            rp->rio_bufptr = rp->rio_ptr; 

            if(events[i].events & EPOLLRDHUP)
            { 
                Log :: WARN("EPOLL HUP FD : %d POOL POS : %d",
                             pool->sockets[idx].sock, idx);
                rapool_del(sev, idx, 0, true);
                (pdata->timer).del_timer_ms(sock_timeout, &pool->sockets[idx]);
                continue;
            }
            else if(events[i].events & EPOLLERR)
            {
                Log :: WARN("APP : 356 EPOLL ERROR FD : %d POOL POS : %d : %s",
                             pool->sockets[idx].sock, idx);
                rapool_del(sev, idx, 0, true);
                (pdata->timer).del_timer_ms(sock_timeout, &pool->sockets[idx]);
                continue;
            }
            else if( events[i].events & EPOLLIN )
            {
                int n;
                int clen = 0;
                //恢复上次 socket 状态
                if(rp->cache != NULL && rp->cache_len > 0)
                {
                    memcpy(req, rp->cache, rp->cache_len);
                    clen = rp->cache_len;
                    
                    //Log :: DEBUG("BACK DUMP DATA IN READ %d bytes %s", clen, rp->cache);
                    Log :: DEBUG("BACK DUMP DATA IN READ %d ", clen);
                    Allocate :: deallocate(rp->cache, rp->cache_len);
                    rp->cache_len = 0; 
                    rp->cache = NULL;
                }
                if((n = readn(sock, req + clen, readsize - clen)) < 0) 
                {
                    Log :: WARN("READ ERROR THREAD ID %d, ERROR %s",
                                pdata->id, strerror(errno));
                    rapool_del(sev, idx, 0, true);
                    (pdata->timer).del_timer_ms(sock_timeout, &pool->sockets[idx]);
                    continue;
                }
                if( n == 0)
                {
                    Log :: WARN("READ FIN ID %d",
                                pdata->id);
                    rapool_del(sev, idx, 0, true);
                    (pdata->timer).del_timer_ms(sock_timeout, &pool->sockets[idx]);
                    continue;
                }

                req[n + clen] = '\0';
                //Log :: DEBUG("READ DATA %d byte VALUE : %s" ,n + clen, req);
                Log :: DEBUG("READ DATA %d byte " ,n + clen);
                
                //READ ANALYSIS 
                int start; 
                start = 0; 
                for(int i = 0; *(req + i) != '\0'; i++)
                {
                    if(*(req + i) == '\n')
                    {
                        //Log :: DEBUG("SEARCHING REQ LINE FD : %d POS : %d VAL : %s" , 
                        //             sock, i, req);
                        pdata->read_start = start;
                        pdata->readed_size = i - start + 1;
                        sev->p_handle();
                        start = i + 1; 
                    }
                }

                int len = n + clen - start;
                if(len > 0)
                {
                    rp->cache = (char *) Allocate :: allocate(len);
                    rp->cache_len = len;
                    memcpy(rp->cache, req + start, len);
                    Log :: DEBUG("READ DUMP CACHE %d bytes VAL : %s ", len, rp->cache);
                }

                //need to send data 
                if(pdata->writed_size > 0)
                {
                    //try to send data
                    if((n = sendn(sock, res, pdata->writed_size)) < 0)
                    {
                        Log :: WARN("WRITE ERROR THREAD ID %d, ERROR %s",
                                    pdata->id, strerror(errno));
                        rapool_del(sev, idx, 0, true);
                        (pdata->timer).del_timer_ms(sock_timeout, &pool->sockets[idx]);
                        continue;
                    }
                    Log :: DEBUG("WRITE DATA %d byte LEFT : %d" ,n, pdata->writed_size - n);
                    //send not enough, need to store write cache
                    if((size_t)n < pdata->writed_size) 
                    {
                        rp->w_cache = (char *) Allocate :: allocate(pdata->writed_size - n);
                        rp->w_allo_cache = rp->w_cache;    
                    
                        rp->w_cache_len = pdata->writed_size - n;
                        rp->w_allo_len = rp->w_cache_len;

                        memcpy(rp->w_cache, res + n, rp->w_cache_len);
                        Log :: DEBUG("WRITE DUMP CACHE %d bytes VAL : %s ", 
                                     rp->w_cache_len, rp->w_cache);
                        //若发送不全，更改下次监控为写事件，否则为读事件不变
                        
                        rapool_epoll_mod_write(sev, idx, pdata->id);
                    }
                }
                (pdata->timer).del_timer_ms(sock_timeout, &pool->sockets[idx]);
                pool->sockets[idx].sock_timeout = (pdata->timer).add_timer_ms(sev->write_to, 
                                                   call_back_timeout, 
                                                   &pool->sockets[idx]);
                //clear work space 
                pdata->read_start = 0;
                pdata->readed_size = 0;
                pdata->write_start = 0;
                pdata->writed_size = 0; 
            }
            else if( events[i].events & EPOLLOUT )
            {
                int n;
            
                if((n = sendn(sock, rp->w_cache, rp->w_cache_len)) < 0)
                {
                    Log :: WARN("WRITE ERROR THREAD ID %d, ERROR %s",
                                pdata->id, strerror(errno));

                    rapool_del(sev, idx, 0, true);
                    (pdata->timer).del_timer_ms(sock_timeout, &pool->sockets[idx]);
                    continue;
                }
                Log :: DEBUG("WRITE event data %d byte LEFT : %d" ,n, rp->w_cache_len - n);
            
                //send not enough, need to send again
                if(n < rp->w_cache_len) 
                {
                    move_forward(rp->w_cache, n, rp->w_cache_len);
                    rp->w_cache_len = rp->w_cache_len - n;
                    Log :: DEBUG("WRITE event LEFT %d bytes VAL : %s ", 
                                 rp->w_cache_len, rp->w_cache);
                    //下个事件仍监听写
                    goto WRITE_END;
                }
                //write succ, wait read event
                Log :: DEBUG("BACK DUMP DATA IN WRITE %d bytes %s", rp->w_allo_len, rp->w_cache);
                Allocate :: deallocate(rp->w_allo_cache, rp->w_allo_len);
                
                rp->w_cache_len = 0; 
                rp->w_cache = NULL;
                rp->w_allo_len = 0; 
                rp->w_allo_cache = NULL; 

                rapool_epoll_mod_read(sev, idx, pdata->id);
                WRITE_END:
                (pdata->timer).del_timer_ms(sock_timeout, &pool->sockets[idx]);
                pool->sockets[idx].sock_timeout = (pdata->timer).add_timer_ms(sev->write_to, 
                                                                              call_back_timeout, 
                                                                              &pool->sockets[idx]);
            }
        }
        //fresh timeout
        timeout = (pdata->timer).top_timer_ms();
        //printf("timeout = %d\n", timeout);
    }
    return 0;
}

void 
nf_default_handle()
{
    char * read_buf = (char *) nf_server_get_read_buf(); 
    char * write_buf = (char *) nf_server_get_write_buf(); 
    int readed_size = nf_server_get_readed_size();
    strncpy(write_buf, read_buf, readed_size);

    if (nf_server_set_writed_size(readed_size) < 0)
        std :: cout << "set writed size error " << std :: endl;
    nf_server_set_writed_start(readed_size);     
}

