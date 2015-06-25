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
//1. rio rio_ptr 读数据缓存区 = 
//2. rio cache   存储状态buf, 防止读事件发生阻塞，非阻塞操作
//               一旦读空，存储状态，释放thread，thread
//               获取queue新事件处理
//3. rio cache   获取采用内存池代替malloc

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

    //清空 读数据的 缓存区
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
            //恢复上次 socket 状态
            if(rp->cache != NULL && rp->cache_len > 0)
            {
                memcpy(req, rp->cache, rp->cache_len);
                clen = rp->cache_len;

                std :: cout << "back status of cache" << std::endl;
 
                Allocate :: deallocate(rp->cache, rp->cache_len);
                rp->cache_len = 0; 
                rp->cache = NULL;
            }
            //无等待时间的 非阻塞读，读空协议栈缓存，但数据凑不满一行时，
            //保存状态，释放线程处理别的事件。恶性肥荩ü�
            //检测清空`

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
                        //return 0, 关闭pdata->epfd, socket 作为ready加入epoll
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


//LF 的 readn 不使用缓存区
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
