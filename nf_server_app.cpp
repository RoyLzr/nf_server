#include "nf_server_core.h"
#include "sapool.h"


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
        //   (1) ÉÏ´ÎÑread¶Áµ½Ò»ĞĞ£¬´¦Àíºó£¬µ«ÊÇÏÂ´ÎÊı¾İÒ»Ö±Ã»ÓĞÀ´. 
        //        event loop ½øĞĞ¸ü³¤Ê±¼ä³¬Ê±µÈ´ıÊı¾İ
        //   (2) ÉÏ´Î read¶Áµ½°ëĞĞ£¬µÈ´ı¶Áµ½»Ø³µ·û£¬µÈ´ı¹ı³ÌÖĞ ·¢Éú³¬Ê±£¬¶Á²»µ½ÍêÕûÒ»ĞĞ¡£return.
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
//1. rio rio_ptr ¶ÁÊı¾İ»º´æÇø = 
//2. rio cache   ´æ´¢×´Ì¬buf, ·ÀÖ¹¶ÁÊÂ¼ş·¢Éú×èÈû£¬·Ç×èÈû²Ù×÷
//               Ò»µ©¶Á¿Õ£¬´æ´¢×´Ì¬£¬ÊÍ·Åthread£¬thread
//               »ñÈ¡queueĞÂÊÂ¼ş´¦Àí
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

    //Çå¿Õ ¶ÁÊı¾İµÄ »º´æÇø
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
            //ÎŞµÈ´ıÊ±¼äµÄ ·Ç×èÈû¶Á£¬¶Á¿ÕĞ­ÒéÕ»»º´æ£¬µ«Êı¾İ´Õ²»ÂúÒ»ĞĞÊ±£¬
            //±£´æ×´Ì¬£¬ÊÍ·ÅÏß³Ì´¦Àí±ğµÄÊÂ¼ş¡£¶ñĞÔ·Êı¾İ£¬Í¨¹ı³¬Ê±
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
}
