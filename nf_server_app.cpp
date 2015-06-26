#include "nf_server_core.h"
#include "sapool.h"
#include "rapool.h"


int 
nf_LF_readline_worker(void * data)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) data;

    char * req = (char *) pdata->read_buf;
    char * res = (char *) pdata->write_buf;
    int epfd = pdata->epfd;
    int fd = pdata->fd;
    int readsize = pdata->read_size;   
    int writesize = pdata->write_size;   
    nf_server_t *sev = pdata->server; 
     
    pdata->rio.rio_fd = pdata->fd;
    pdata->rio.rio_cnt = 0;
    pdata->rio.rio_bufptr = pdata->rio.rio_ptr; 
     
    int ret;
    if((ret = net_ep_add_in(epfd, fd)) < 0)
    {
        std :: cout << "add epoll error" << std :: endl;
        return -1;
    }      

    int readto = nf_server_get_readto();
    int writeto = nf_server_get_writeto();
     
    struct epoll_event events[1], ev;
    
    //event loop
    while(sev->run)
    {
        ret = epoll_wait(pdata->epfd, events, 1, readto * 20);
        if(ret == 0)
        {
            std::cout << "read timeout error" << std::endl; 
            return -1;
        }
        else if(ret < 0)
        {
            std::cout << "epoll wait error :" << strerror(errno) <<std::endl;   
            return -1;
        } 
        //events analyse
        
        if(events[0].events & EPOLLRDHUP)
        { 
            std::cout << "event close fd error" << std::endl; 
            return -1;
        }
        else if(events[0].events & EPOLLERR )
        {
            std::cout << "event fd  error" << std::endl; 
            return -1;
        }
        else if( events[0].events & EPOLLIN )
        {
            int n;
            while((n = rio_readline_to_ms(&pdata->rio, req, readsize, readto)) > 0)   
            {
                pdata->readed_size = n;
                sev->p_handle();
                if((n = sendn_to_ms(pdata->rio.rio_fd, res, pdata->writed_size, writeto))< 0)
                {
                    std :: cout << "write error" << strerror(errno) << std :: endl;
                        return -1;
                }
            }
        //Àë¿ª Ñ­»·¶Á Çé¿ö£º
        //1. send error, °üÀ¨ TIMEOUT ±Ö±½Ó Ó return 
        //2. read error, Èç¹ûÊÇ fin »òÕß ÆäÓà¶Áµ½ÆäËû´íÎó return
        //3. read error, Èç¹ûÊÇ TIMEOUT
        //   (1) ÉÏ´ÎÑread¶Áµ½Ò»ÐÐ£¬´¦Àíºó£¬µ«ÊÇÏÂ´ÎÊý¾ÝÒ»Ö±Ã»ÓÐÀ´. 
        //        event loop ½øÐÐ¸ü³¤Ê±¼ä³¬Ê±µÈ´ýÊý¾Ý
        //   (2) ÉÏ´Î read¶Áµ½°ëÐÐ£¬µÈ´ý¶Áµ½»Ø³µ·û£¬µÈ´ý¹ý³ÌÖÐ ·¢Éú³¬Ê±£¬¶Á²»µ½ÍêÕûÒ»ÐÐ¡£return.
            if(n == 0|| errno != ETIMEDOUT )
            {
                if( n == 0)
                    std :: cout << "recv fin" << std :: endl;
                else if( errno != ETIMEDOUT)
                    std :: cout << "read error : " << strerror(errno) << std :: endl;
                return -1;
            }
            if(errno == ETIMEDOUT && n != -1)
                return -1;
        }
    }
    return 0;
}

//SA model read line support:
//1. rio rio_ptr ¶ÁÊý¾Ý»º´æÇø = 
//2. rio cache   ´æ´¢×´Ì¬buf, ·ÀÖ¹¶ÁÊÂ¼þ·¢Éú×èÈû£¬·Ç×èÈû²Ù×÷
//               Ò»µ©¶Á¿Õ£¬´æ´¢×´Ì¬£¬ÊÍ·Åthread£¬thread
//               »ñÈ¡queueÐÂÊÂ¼þ´¦Àí
//3. rio cache   »ñÈ¡²ÉÓÃÄÚ´æ³Ø´úÌæmalloc

int 
nf_SA_readline_worker(void * data)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) data;
    nf_server_t * sev = (nf_server_t *) pdata->server;
    sapool_t * pool = (sapool_t *) sev->pool;    

    char * req = (char *) pdata->read_buf;
    char * res = (char *) pdata->write_buf;
    int epfd = pdata->epfd;
    int fd = pdata->fd;
    int readsize = pdata->read_size;   
    int writesize = pdata->write_size;   
    int idx = pdata->idx;    
    rio_t * rp = &(pool->sockets[idx].rp); 

    //Çå¿Õ ¶ÁÊý¾ÝµÄ »º´æÇø
    rp->rio_fd = pdata->fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_ptr; 
     
    int ret;
    if((ret = net_ep_add_in(epfd, fd)) < 0)
    {
        std :: cout << "add epoll error" << std :: endl;
        return -1;
    }      

    int readto = nf_server_get_readto();
    int writeto = nf_server_get_writeto();
     
    struct epoll_event events[1], ev;
    
    //event loop
    while(sev->run)
    {
        ret = epoll_wait(pdata->epfd, events, 1, 1000);
        if(ret == 0)
        {
            std::cout << "read timeout error" << std::endl; 
            return -1;
        }
        else if(ret < 0)
        {
            std::cout << "epoll wait error :" << strerror(errno) <<std::endl;   
            return -1;
        } 
        //events analyse
        
        if(events[0].events & EPOLLRDHUP)
        { 
            std::cout << "event close fd error" << std::endl; 
            return -1;
        }
        else if(events[0].events & EPOLLERR )
        {
            std::cout << "event fd  error" << std::endl; 
            return -1;
        }
        else if( events[0].events & EPOLLIN )
        {
            int n;
            int st;
            int clen = 0;
            //»Ö¸´ÉÏ´Î socket ×´Ì¬
            if(rp->cache != NULL && rp->cache_len > 0)
            {
                memcpy(req, rp->cache, rp->cache_len);
                clen = rp->cache_len;

                std :: cout << "back status of cache" << std::endl;
 
                Allocate :: deallocate(rp->cache, rp->cache_len);
                rp->cache_len = 0; 
                rp->cache = NULL;
            }
            //ÎÞµÈ´ýÊ±¼äµÄ ·Ç×èÈû¶Á£¬¶Á¿ÕÐ­ÒéÕ»»º´æ£¬µ«Êý¾Ý´Õ²»ÂúÒ»ÐÐÊ±£¬
            //±£´æ×´Ì¬£¬ÊÍ·ÅÏß³Ì´¦Àí±ðµÄÊÂ¼þ¡£¶ñÐÔ·Êý¾Ý£¬Í¨¹ý³¬Ê±
            //¼ì²âÇå¿Õ`

            while((n = rio_readline(rp, req + clen, readsize - clen, &st)) > 0)   
            {
                if(n == 0)
                {
                    std :: cout << "recv fin" << std :: endl;
                    return -1; 
                }
                else if(n < 0)
                {
                    std :: cout << "recv error : " << strerror(errno) << std :: endl;
                    return -1; 
                }
                else if(st < 0)
                {
                    int len = clen + n -1;
                    if(len > 0)
                    {   
                        std :: cout << "dump cache : " << len << std::endl;
                        rp->cache = (char *) Allocate :: allocate(len);
                    
                        rp->cache_len = len;
                        memcpy(rp->cache, req, len);
                        //return 0, ¹Ø±Õpdata->epfd, socket ×÷Îªready¼ÓÈëepoll
                    }
                    return 0;
                }
                pdata->readed_size = n + clen;
                clen = 0;
                sev->p_handle();
                if((n = sendn_to_ms(rp->rio_fd, res, pdata->writed_size, writeto))< 0)
                {
                    std :: cout << "write error" << strerror(errno) << std :: endl;
                        return -1;
                }
            }
        }
    }
    return 0;
}


//LF µÄ readn ²»Ê¹ÓÃ»º´æÇø
int 
nf_LF_readnf_worker(void * data)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) data;

    char * req = (char *) pdata->read_buf;
    char * res = (char *) pdata->write_buf;
    int epfd = pdata->epfd;
    int fd = pdata->fd;
    int readsize = pdata->read_size;   
    int writesize = pdata->write_size;   
    nf_server_t *sev = pdata->server; 
     
    int ret;
    if((ret = net_ep_add_in(epfd, fd)) < 0)
    {
        std :: cout << "add epoll error : " << 
        strerror(errno)<< std :: endl;
        return -1;
    }      

    int readto = nf_server_get_readto();
    int writeto = nf_server_get_writeto();
     
    struct epoll_event events[1], ev;
    
    //event loop
    while(sev->run)
    {
        ret = epoll_wait(pdata->epfd, events, 1, readto * 20);
        if(ret == 0)
        {
            std::cout << "read timeout error" << std::endl; 
            return -1;
        }
        else if(ret < 0)
        {
            std::cout << "epoll wait error :" << strerror(errno) <<std::endl;   
            return -1;
        } 
        //events analyse
        
        if(events[0].events & EPOLLRDHUP)
        { 
            std::cout << "event close fd error" << std::endl; 
            return -1;
        }
        else if(events[0].events & EPOLLERR )
        {
            std::cout << "event fd  error" << std::endl; 
            return -1;
        }
        else if( events[0].events & EPOLLIN )
        {
            int n;
            if ((n = readn_to_ms(pdata->fd, req, 6, readto)) < 0)
            {
                std :: cout << "read error : " << strerror(errno) << std::endl;
                return -1;
            }
            pdata->readed_size = n;
            sev->p_handle();
    
            if((n = sendn_to_ms(pdata->fd, res, pdata->writed_size, writeto)) < 0)
            {
                std :: cout << "write error : " << strerror(errno) << std :: endl;
                return -1;
            }
        }
    }
    return 0;
}

int 
nf_RA_readline_worker(void * data)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) data;

    char * req = (char *) pdata->read_buf;
    char * res = (char *) pdata->write_buf;
    int epfd = pdata->epfd;
    int readsize = pdata->read_size;   
    int writesize = pdata->write_size;   
    nf_server_t *sev = pdata->server;
    rapool_t * pool = (rapool_t *) sev->pool; 
 
    int ret;
    int readto = nf_server_get_readto();
    int writeto = nf_server_get_writeto();
    int ssiz = nf_server_get_socksize(sev);
    
    struct epoll_event events[ssiz], ev;
   
    pdata->write_start = 0; 
    //event loop
    while(sev->run)
    {
        int num = epoll_wait(pdata->epfd, events, 1, pool->timeout * 1000);
        if(num == 0)
        {   
            Log :: DEBUG("nf_server_app : 325 BEGIN TIMEOUT TEST");
            continue;
        }
        
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
            
            //Çå¿Õ ¶ÁÊý¾ÝµÄ »º´æÇø
            rp->rio_fd = sock;
            rp->rio_cnt = 0;
            rp->rio_bufptr = rp->rio_ptr; 

            if(events[i].events & EPOLLRDHUP)
            { 
                Log :: WARN("EPOLL HUP FD : %d POOL POS : %d",
                             pool->sockets[idx].sock, idx);
                rapool_del(sev, idx, 0, true);
                continue;
            }
            else if(events[i].events & EPOLLERR)
            {
                Log :: WARN("APP : 356 EPOLL ERROR FD : %d POOL POS : %d",
                             pool->sockets[idx].sock, idx);
                rapool_del(sev, idx, 0, true);
                continue;
            }
            else if( events[i].events & EPOLLIN )
            {
                int n;
                int st;
                int clen = 0;
                //»Ö¸´ÉÏ´Î socket ×´Ì¬
                if(rp->cache != NULL && rp->cache_len > 0)
                {
                    memcpy(req, rp->cache, rp->cache_len);
                    clen = rp->cache_len;
                    
                    Log :: DEBUG("BACK DUMP DATA IN READ %d bytes %s", clen, rp->cache);
                    Allocate :: deallocate(rp->cache, rp->cache_len);
                    rp->cache_len = 0; 
                    rp->cache = NULL;
                }
                if((n = readn(sock, req + clen, readsize - clen)) < 0) 
                {
                    Log :: WARN("READ ERROR THREAD ID %d, ERROR %s",
                                pdata->id, strerror(errno));
                    rapool_del(sev, idx, 0, true);
                    continue;
                }
                if( n == 0)
                {
                    Log :: WARN("READ FIN ID %d",
                                pdata->id);
                    rapool_del(sev, idx, 0, true);
                    continue;
                }

                req[n + clen] = '\0';
                Log :: DEBUG("READ DATA %d byte VALUE : %s" ,n + clen, req);
                
                //READ ANALYSIS 
                int start, end; 
                start = 0; 
                for(int i = 0; *(req + i) != '\0'; i++)
                {
                    if(*(req + i) == '\n')
                    {
                        Log :: DEBUG("SEARCHING REQ LINE FD : %d POS : %d VAL : %s" , 
                                     sock, i, req);
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
                        continue;
                    }
                    Log :: DEBUG("WRITE DATA %d byte LEFT : %d" ,n, pdata->writed_size - n);
                    //send not enough, need to store write cache
                    if(n < pdata->writed_size) 
                    {
                        rp->w_cache = (char *) Allocate :: allocate(pdata->write_size - n);
                        rp->w_cache_len = pdata->write_size - n;
                        rp->w_allo_len = rp->w_cache_len;

                        memcpy(rp->w_cache, res + n, rp->w_cache_len);
                        Log :: DEBUG("WRITE DUMP CACHE %d bytes VAL : %s ", 
                                     rp->w_cache_len, rp->w_cache);
                        //Èô·¢ËÍ²»È«£¬¸ü¸ÄÏÂ´Î¼à¿ØÎªÐ´ÊÂ¼þ£¬·ñÔòÎª¶ÁÊÂ¼þ²»±ä
                        rapool_epoll_mod_write(sev, idx, pdata->id);
                    }
                } 
                //clear work space 
                pdata->read_start = 0;
                pdata->readed_size = 0;
                pdata->write_start = 0;
                pdata->writed_size = 0; 
            }
            else if( events[i].events & EPOLLOUT )
            {
                int n;
                int st;
                int clen = 0;
            
                if((n = sendn(sock, rp->w_cache, rp->w_cache_len)) < 0)
                {
                    Log :: WARN("WRITE ERROR THREAD ID %d, ERROR %s",
                                pdata->id, strerror(errno));

                    rapool_del(sev, idx, 0, true);
                    continue;
                }
                Log :: DEBUG("WRITE event data %d byte LEFT : %d" ,n, rp->w_cache_len - n);
            
                //send not enough, need to store write cache
                if(n < rp->w_cache_len) 
                {
                    move_forward(rp->w_cache, n, rp->w_cache_len);
                    rp->w_cache_len = rp->w_cache_len - n;
                    Log :: DEBUG("WRITE event LEFT %d bytes VAL : %s ", 
                                 rp->w_cache_len, rp->w_cache);
                    //ÏÂ¸öÊÂ¼þÈÔ¼àÌýÐ´
                    continue;
                }
                //write succ, wait read event
                Log :: DEBUG("BACK DUMP DATA IN WRITE %d bytes %s", rp->w_allo_len, rp->cache);
                Allocate :: deallocate(rp->cache, rp->w_allo_len);
                
                rp->w_cache_len = 0; 
                rp->w_cache = NULL;
                rp->w_allo_len = 0; 

                rapool_epoll_mod_read(sev, idx, pdata->id);
            }
        } 
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
    std :: cout << "server read data : " << read_buf << std :: endl;

    if (nf_server_set_writed_size(readed_size) < 0)
        std :: cout << "set writed size error " << std :: endl;
    nf_server_set_writed_start(readed_size);     
}

