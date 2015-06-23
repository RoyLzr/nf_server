#include "nf_server_core.h"
#include "sapool.h"
#include "pool_register.h"

static pthread_key_t pkey;
static pthread_once_t ponce = PTHREAD_ONCE_INIT;

static void 
create_key_once(void)
{
    pthread_key_create(&pkey, NULL);
}

int 
set_pthread_data(nf_server_pdata_t *data)
{
    void *ptr = NULL;
    pthread_once(&ponce, create_key_once);
    if ((ptr = pthread_getspecific(pkey)) == NULL) 
    {
        ptr = data;
        pthread_setspecific(pkey, ptr);
    }
    return 0;
}

nf_server_pdata_t * 
get_pdata()
{
    void * ptr = pthread_getspecific(pkey);
    return (nf_server_pdata_t *) ptr;
}

void * 
nf_server_get_read_buf()
{
    nf_server_pdata_t *ptr = get_pdata();
    if (ptr == NULL) 
    {
        std :: cout << "empty read buf" << std :: endl;
        return NULL;
    }
    else
        return ptr->read_buf;
}

void * 
nf_server_get_write_buf()
{
    nf_server_pdata_t *ptr = get_pdata();
    if (ptr == NULL) 
    {
        std :: cout << "empty wirte buf" << std :: endl;
        return NULL;
    }
    else
        return ptr->write_buf;
}

int
nf_server_get_readto()
{
    nf_server_pdata_t *ptr = get_pdata();
    if (ptr == NULL) 
    {
        std :: cout << "empty readto" << std :: endl;
        return -1;
    }
    else
        return (ptr->server)->read_to;
}

int
nf_server_get_writeto()
{
    nf_server_pdata_t *ptr = get_pdata();
    if (ptr == NULL) 
    {
        std :: cout << "empty readto" << std :: endl;
        return -1;
    }
    else
        return (ptr->server)->write_to;
}

int
nf_server_get_qsize(nf_server_t * sev)
{
    if (sev == NULL) 
    {
        std :: cout << "empty qsize" << std :: endl;
        return -1;
    }
    else
        return sev->qsize;
}

int
nf_server_get_socksize(nf_server_t * sev)
{
    if (sev == NULL) 
    {
        std :: cout << "empty socketsize" << std :: endl;
        return -1;
    }
    else
        return sev->socksize;
}

int
nf_server_get_readed_size()
{
    nf_server_pdata_t *ptr = get_pdata();
    if (ptr == NULL) 
    {
        std :: cout << "empty readed size" << std :: endl;
        return -1;
    }
    else
        return ptr->readed_size;
}

int
nf_server_set_writed_size(int n)
{
    nf_server_pdata_t *ptr = get_pdata();
    if (ptr == NULL) 
    {
        std :: cout << "empty writed size" << std :: endl;
        return -1;
    }
    ptr->writed_size = n;
    return 1;
}

int
nf_server_get_writed_size()
{
    nf_server_pdata_t *ptr = get_pdata();
    if (ptr == NULL) 
    {
        std :: cout << "empty writed size" << std :: endl;
        return -1;
    }
    else
        return ptr->writed_size;
}

nf_server_t * 
nf_server_create(const char * sev_name)
{
    nf_server_t *sev = (nf_server_t *)malloc(sizeof(nf_server_t));
    if(sev == NULL)
        return NULL;
   
    sev->server_type = NFSVR_LFPOOL;
    sev->connect_type = NFSVR_SHORT_CONNEC;
    sev->pthread_num = 1;
    sev->run_thread_num = 0;
    sev->sock_family = AF_INET;
    sev->backlog = 2048;
    sev->thread_write_buf = 0;
    sev->thread_read_buf = 0;
   
    sev->stack_size = 10485760;  //10M
    sev->sev_socket = -1;    
    
    sev->listen_prio = 10;
    sev->work_prio = 5;  

    sev->socksize = 500;  
    sev->qsize = 10000;
    sev->check_interval = 5;
    sev->timeout = 60;

    sev->cb_work = NULL;
    sev->pdata = NULL;
    sev->p_start = NULL;
    sev->p_end = NULL;
    sev->p_handle = NULL;     
    
    sev->status = INIT;

    if(sev_name == NULL)
    {
        strncpy(sev->name, "simple server", sizeof(sev->name));
        sev->name[sizeof(sev->name) - 1] = '\0';
    }
    else
    {
        strncpy(sev->name, sev_name, sizeof(sev->name));
        sev->name[sizeof(sev->name) - 1] = '\0';
    } 
    return sev;
}

int 
nf_pdata_init(nf_server_pdata_t * pdata, nf_server_t * sev)
{
    //thread contains readbuff writebuf usr_buf to use
    pdata->pid = 0;
    pdata->id = 0;
    pdata->server = sev;
    pdata->fd = -1;
     
    pdata->read_buf = NULL;
    pdata->write_buf = NULL;
    
    pdata->ep_size = 1;

    if(sev->thread_read_buf == 0)
        pdata->read_size = 1024;
    else
        pdata->read_size = sev->thread_read_buf;

    if(sev->thread_write_buf == 0)
        pdata->write_size = 1024;
    else
        pdata->write_size = sev->thread_write_buf;
    
    if(sev->thread_usr_buf == 0)
        pdata->usr_size = 1024;
    else
        pdata->usr_size = sev->thread_usr_buf;

    pdata->read_buf = malloc(sizeof(char) * pdata->read_size);
    if(pdata->read_buf == NULL)
        return -1;
    memset(pdata->read_buf, 0, sizeof(char) * pdata->read_size);
    
    pdata->write_buf = malloc(sizeof(char) * pdata->write_size);
    if(pdata->write_buf == NULL)
        return -1;
    memset(pdata->write_buf, 0, sizeof(char) * pdata->write_size);
    
    pdata->usr_buf = malloc(sizeof(char) * pdata->usr_size);
    if(pdata->usr_buf == NULL)
        return -1;
    memset(pdata->usr_buf, 0, sizeof(char) * pdata->usr_size);

    if(sev->server_type == NFSVR_LFPOOL)
    {
        pdata->rio.rio_ptr = (char *)malloc(sizeof(char) * pdata->usr_size);
        rio_init(&pdata->rio, pdata->fd, pdata->usr_size);
    
        if(pdata->rio.rio_ptr == NULL)
            return -1;
        memset(pdata->rio.rio_ptr, 0, sizeof(char) * pdata->usr_size);
    }
    return 0;
}

int 
nf_server_init(nf_server_t * sev)
{
    int ret = 0;
    
    sev->run = 1;
    //conf œ‡πÿƒ⁄»›≥ı ºªØ
    if(sev == NULL)
        return -1;
    //º‡Ã˝∂Àø⁄
    sev->listen_port = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("server", "listenPort")).c_str());
    //≥¨ ±…Ë÷√
    sev->connect_to = (size_t)atoi((Singleton<ConfigParser>::
                                    instance()->get("server", "connectTo")).c_str());

    sev->read_to = (size_t)atoi((Singleton<ConfigParser>::
                                 instance()->get("server", "readTo")).c_str());

    sev->write_to = (size_t)atoi((Singleton<ConfigParser>::
                                  instance()->get("server", "writeTo")).c_str());

    //connect ∑Ω∑®
    sev->connect_type = (size_t)atoi((Singleton<ConfigParser>::
                                      instance()->get("server", "connectType")).c_str());
    
    //server_type
    sev->server_type = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("server", "type")).c_str());
    //init by pool.init() pool ƒ⁄»›
    sev->pool = NULL;
    sev->check_interval = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("pool", "check_interval")).c_str());
    sev->timeout = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("pool", "timeout")).c_str());

    //œﬂ≥Ã ˝
    sev->pthread_num = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("server", "threadNum")).c_str());

    //œﬂ≥Ã’ª
    //sev->stack_size = 10485760;  //10M
    sev->stack_size = (size_t)atoi((Singleton<ConfigParser>:: 
                                    instance()->get("server", "stackSize")).c_str());

    //thread write/read/usr buf size
    sev->thread_read_buf = (size_t)atoi((Singleton<ConfigParser>:: 
                                    instance()->get("thread", "threadReadBuf")).c_str());
    sev->thread_write_buf = (size_t)atoi((Singleton<ConfigParser>:: 
                                    instance()->get("thread", "threadWriteBuf")).c_str());
    sev->thread_usr_buf = (size_t)atoi((Singleton<ConfigParser>:: 
                                    instance()->get("thread", "threadUsrBuf")).c_str());
  
    //œﬂ≥Ãƒ⁄»› ≥ı ºªØ 
    sev->pdata = (nf_server_pdata_t *) malloc(sizeof(nf_server_pdata_t) * (sev->pthread_num));
    if (sev->pdata == NULL)
        return -1;
    
    if(sev->cb_work == NULL)
        sev->cb_work = nf_LF_readline_worker;
    if(sev->p_handle == NULL)
        sev->p_handle = nf_default_handle;

    for(int i = 0; i < (sev->pthread_num); i++)
    { 
       if( (ret = nf_pdata_init(&sev->pdata[i], sev) ) < 0 )
            return -1;
    }
    return 0;
}

int set_sev_socketopt(nf_server_t *sev, int fd)
{
    const int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    
    if( sev->connect_type == NFSVR_SHORT_CONNEC)
    {
        struct linger li;
        memset(&li, 0, sizeof(li)); 
        li.l_onoff = 1;
        li.l_linger = 2;
        setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&li, sizeof(li) );     
    }
    
    //ƒ¨»œø™∆ÙTCP_DEFER_ACCCEPT, drop no data connection
    int timeout = sev->connect_to;
    setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(timeout));
    
    //ƒ¨»œ …Ë÷√ accept socket is noblock
    if ( set_fd_noblock(fd) < 0)
        return -1;

    return 0; 
}

int nf_server_bind(nf_server_t * sev)
{
    //set svr : listen socket 
    const int on = 1;
    struct sockaddr_in addr;
    int ret;
    
    if(sev->sev_socket < 0)
    {
        if( (sev->sev_socket = socket(PF_INET, SOCK_STREAM, 0) ) < 0 )
        {
            std::cout << "create sock: " << strerror(errno) << std::endl;
            return -1; 
        }
    }
    //∏¥”√
    setsockopt(sev->sev_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    set_sev_socketopt(sev, sev->sev_socket);
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(sev->listen_port);
    
    if (bind(sev->sev_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) 
    {
        std::cout << "bind sock: " << strerror(errno) << std::endl;
        close(sev->sev_socket);
        return -1;    
    }
    
    ret = g_pool[sev->server_type].init(sev); 
    return 0;
}


int nf_server_listen(nf_server_t * sev)
{
    if(sev->backlog <= 5)
        sev->backlog = 2048;
    int backlog = sev->backlog;
    if( listen(sev->sev_socket, backlog) < 0)
    {
        std::cout << "listen sock: " << strerror(errno) << std::endl;
        close(sev->sev_socket);
        return -1;    
    }
    std::cout << "listen ok" << std::endl;
    //listen ∫Ø ˝Œ™ø’
    if( g_pool[sev->server_type].listen == NULL)
        return 0;
    return g_pool[sev->server_type].listen(sev);
}

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
        //¿Îø™ —≠ª∑∂¡ «Èøˆ£∫
        //1. send error, ∞¸¿® TIMEOUT ±÷±Ω” ” return 
        //2. read error, »Áπ˚ « fin ªÚ’ﬂ ∆‰”‡∂¡µΩ∆‰À˚¥ÌŒÛ return
        //3. read error, »Áπ˚ « TIMEOUT
        //   (1) …œ¥Œ—read∂¡µΩ“ª––£¨¥¶¿Ì∫Û£¨µ´ «œ¬¥Œ ˝æ›“ª÷±√ª”–¿¥. 
        //        event loop Ω¯––∏¸≥§ ±º‰≥¨ ±µ»¥˝ ˝æ›
        //   (2) …œ¥Œ read∂¡µΩ∞Î––£¨µ»¥˝∂¡µΩªÿ≥µ∑˚£¨µ»¥˝π˝≥Ã÷– ∑¢…˙≥¨ ±£¨∂¡≤ªµΩÕÍ’˚“ª––°£return.
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
//1. rio rio_ptr ∂¡ ˝æ›ª∫¥Ê«¯ = 
//2. rio cache   ¥Ê¥¢◊¥Ã¨buf, ∑¿÷π∂¡ ¬º˛∑¢…˙◊Ë»˚£¨∑«◊Ë»˚≤Ÿ◊˜
//               “ªµ©∂¡ø’£¨¥Ê¥¢◊¥Ã¨£¨ Õ∑≈thread£¨thread
//               ªÒ»°queue–¬ ¬º˛¥¶¿Ì
//3. rio cache   ªÒ»°≤…”√ƒ⁄¥Ê≥ÿ¥˙ÃÊmalloc

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

    //«Âø’ ∂¡ ˝æ›µƒ ª∫¥Ê«¯
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
            //ª÷∏¥…œ¥Œ socket ◊¥Ã¨
            if(rp->cache != NULL && rp->cache_len > 0)
            {
                memcpy(req, rp->cache, rp->cache_len);
                clen = rp->cache_len;

                std :: cout << "back status of cache" << std::endl;
 
                Allocate :: deallocate(rp->cache, rp->cache_len);
                rp->cache_len = 0; 
                rp->cache = NULL;
            }
            //Œﬁµ»¥˝ ±º‰µƒ ∑«◊Ë»˚∂¡£¨∂¡ø’–≠“È’ªª∫¥Ê£¨µ´ ˝æ›¥’≤ª¬˙“ª–– ±£¨
            //±£¥Ê◊¥Ã¨£¨ Õ∑≈œﬂ≥Ã¥¶¿Ì±µƒ ¬º˛°£∂Ò–‘∑ ˝æ›£¨Õ®π˝≥¨ ±
            //ºÏ≤‚«Âø’`

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
                        //return 0, πÿ±’pdata->epfd, socket ◊˜Œ™readyº”»Îepoll
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


//LF µƒ readn ≤ª π”√ª∫¥Ê«¯
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
