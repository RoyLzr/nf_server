#include "nf_server_core.h"
#include "net.h"
#include <unistd.h>
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
        std :: cout << "empty read buf" << std :: endl;
        return NULL;
    }
	else
		return ptr->write_buf;
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

    sev->sock_num = 500;  
    sev->queue_len = 200;
    sev->check_interval = 5;
    sev->timeout = 60;

    sev->cb_work = NULL;
    sev->pdata = NULL;
    sev->p_start = NULL;
    sev->p_end = NULL;
    sev->p_read = NULL;     
    sev->p_write = NULL;     
    
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

    pdata->rio.rio_ptr = (char *)malloc(sizeof(char) * pdata->read_size);
    rio_init(&pdata->rio, pdata->fd, pdata->read_size);

    if(pdata->rio.rio_ptr == NULL)
        return -1;
    memset(pdata->rio.rio_ptr, 0, sizeof(char) * pdata->usr_size);
    
    return 0;
}

int 
nf_server_init(nf_server_t * sev)
{
    int ret = 0;
    
    sev->run = 1;
    //conf 相关内容初始化
    if(sev == NULL)
        return -1;
    //监听端口
    sev->listen_port = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("server", "listenPort")).c_str());
    //超时设置
    sev->connect_to = (size_t)atoi((Singleton<ConfigParser>::
                                    instance()->get("server", "connectTo")).c_str());

    sev->read_to = (size_t)atoi((Singleton<ConfigParser>::
                                 instance()->get("server", "readTo")).c_str());

    sev->write_to = (size_t)atoi((Singleton<ConfigParser>::
                                  instance()->get("server", "writeTo")).c_str());

    //connect 方法
    sev->connect_type = (size_t)atoi((Singleton<ConfigParser>::
                                      instance()->get("server", "connectType")).c_str());
    
    //server_type
    sev->server_type = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("server", "type")).c_str());
    //init by pool.init()
    sev->pool = NULL;

    //线程数
    sev->pthread_num = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("server", "threadNum")).c_str());

    //线程栈
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
  
 
    //线程内容 初始化 
    sev->pdata = (nf_server_pdata_t *) malloc(sizeof(nf_server_pdata_t) * (sev->pthread_num));
    if (sev->pdata == NULL)
        return -1;
    
    if(sev->cb_work == NULL)
        sev->cb_work = nf_default_worker;
    if(sev->p_read == NULL)
        sev->p_read = nf_default_read_buf;
    if(sev->p_write == NULL)
        sev->p_write = nf_default_write_buf;

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
    
    //默认开启TCP_DEFER_ACCCEPT, drop no data connection
    int timeout = sev->connect_to;
    setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(timeout));
    
    //默认 设置 accept socket is noblock
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
    //复用
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
    //listen 函数为空
    if( g_pool[sev->server_type].listen == NULL)
        return 0;
    return g_pool[sev->server_type].listen(sev);
}

int nf_default_worker(void *req)
{
    nf_server_pdata_t * pdata = (nf_server_pdata_t *)req;
    int ret = net_ep_add_in(pdata->epfd, pdata->fd);
    if( ret < 0)
        return -1;

    int readto = (((nf_server_pdata_t *)req)->server)->read_to;
    nf_handle_t read_fun = (((nf_server_pdata_t *)req)->server)->p_read;
    nf_handle_t write_fun = (((nf_server_pdata_t *)req)->server)->p_write;
     
    const int size = pdata->ep_size;
    struct epoll_event events[size], ev;

    readto = -1;
    
    //event loop 
    while(1)
    {
        ret = epoll_wait(pdata->epfd, events, pdata->ep_size, readto);
        if(ret == 0)
            {std::cout << "read timeout error" << std::endl; return ret;}
        else if(ret < 0)
            {std::cout << "line:226 core.cpp :" << strerror(errno) <<std::endl;   return ret;} 
        //events analyse
        //std::cout << "get event" << std::endl;
        for(int i = 0; i < ret; i++)
        {
            if( events[i].events & EPOLLRDHUP)
            { 
                std::cout << "event close fd error" << std::endl; 
                return -1;
            }
            else if( events[i].events & EPOLLERR )
            {
                //std::cout << "event fd  error" << std::endl; 
                return -1;
            }
            else if( events[i].events & EPOLLIN )
            {   
                if( read_fun(pdata) < 0)
                    return -1;
                else
                {
                    ev.data.fd = pdata->fd;
                    ev.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
                    epoll_ctl(pdata->epfd, EPOLL_CTL_MOD, pdata->fd, &ev);
                }
            }
            else if( events[i].events & EPOLLOUT )
            {
                if( write_fun(pdata) < 0)
                    return -1;
                else
                {
                    ev.data.fd = pdata->fd;
                    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
                    epoll_ctl(pdata->epfd, EPOLL_CTL_MOD, pdata->fd, &ev);
                }
            }
        }
    }
    return 1;
}

int nf_default_read_buf(void *data)
{
    int ret;
    nf_server_pdata_t *pdata = (nf_server_pdata_t *)data;
    pdata->rio.rio_fd = pdata->fd;

    void * read_buf =  nf_server_get_read_buf();

    if ((ret = rio_readn_to_ms(&(pdata->rio), read_buf, 5, 1000)) <= 0)
    {
        return -1; 
    }
    std::cout << read_buf << " : read value" <<std::endl;
    return ret;
}

int nf_default_write_buf(void *data)
{
    nf_server_pdata_t *pdata = (nf_server_pdata_t *)data;

    char * write_buf = (char *) nf_server_get_write_buf();
    char * read_buf = (char *) nf_server_get_read_buf();

    strncpy(write_buf, read_buf, 5);
    char * temp = write_buf;
    temp[5] = '\0';
    int ret;
    if ( (ret = sendn_to_ms(pdata->fd, write_buf, 5, 1000)) <= 0 )
    {
        std::cout << strerror(errno) << std::endl;
        return -1;
    }
    return ret;
}
