//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  work thread ʹ�õĴ���Ӧ�ú������û�������Լ��Ĵ�����
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
        //�뿪 ѭ���� �����
        //1. send error, ���� TIMEOUT �ֱ�� � return 
        //2. read error, ����� fin ���� ��������������� return
        //3. read error, ����� TIMEOUT
        //   (1) �ϴ��read����һ�У�����󣬵����´�����һֱû����. 
        //        event loop ���и���ʱ�䳬ʱ�ȴ�����
        //   (2) �ϴ� read�������У��ȴ������س������ȴ������� ������ʱ������������һ�С�return.
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
//1. rio rio_ptr �����ݻ����� = 
//2. rio cache   �洢״̬buf, ��ֹ���¼���������������������
//               һ�����գ��洢״̬���ͷ�thread��thread
//               ��ȡqueue���¼�����
//3. rio cache   ��ȡ�����ڴ�ش���malloc

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

    //��� �����ݵ� ������
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
            //�ָ��ϴ� socket ״̬
            if(rp->cache != NULL && rp->cache_len > 0)
            {
                memcpy(req, rp->cache, rp->cache_len);
                clen = rp->cache_len;

                Log :: NOTICE("Back status of cache: %d", clen);
 
                Allocate :: deallocate(rp->cache, rp->cache_len);
                rp->cache_len = 0; 
                rp->cache = NULL;
            }
            //�޵ȴ�ʱ��� ��������������Э��ջ���棬�����ݴղ���һ��ʱ��
            //����״̬���ͷ��̴߳������¼������Է���ݣ�ͨ����ʱ
            //������`

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
                        //return 0, �ر�pdata->epfd, socket ��Ϊready����epoll
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


            //��� �����ݵ� ������
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
                //�ָ��ϴ� socket ״̬
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
                        //�����Ͳ�ȫ�������´μ��Ϊд�¼�������Ϊ���¼�����
                        
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
                    //�¸��¼��Լ���д
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

