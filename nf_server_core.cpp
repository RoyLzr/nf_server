#include "nf_server_core.h"


int NfReactor :: init(int ssize, nf_server_t * svr)
{
    this->svr = svr;
    nf_server_pdata_t * pdata = (nf_server_pdata_t *) malloc (sizeof(nf_server_pdata_t) * svr->thread_num); 
    memset(pdata, 0, sizeof(nf_server_pdata_t)*svr->thread_num);
    
    if(pdata == NULL)
    {
        Log :: WARN("INIT svr pdata Error");
        return -1;
    }
    
    for(int i = 0; i < svr->thread_num; i++)
    {
        pdata[i].reactor = this;
    }
    struct threadParas paras;
    paras.num = svr->thread_num;
    paras.buff_size = svr->thread_usr_buf; 

    return Reactor :: init(ssize, paras);
}

/*
int 
nf_server_get_thread_epfd()
{
    nf_server_pdata_t * ptr = get_pdata();
    if (ptr == NULL) 
    {
        Log :: ERROR("NF_SVR_CORE : GET EPFD EMPTY");
        return -1;
    }
    else
        return ptr->epfd;
}

int
nf_server_get_readto()
{
    nf_server_pdata_t *ptr = get_pdata();
    if (ptr == NULL) 
    {
        Log :: ERROR("NF_SVR_CORE : GET READ TIMEOUT EMPTY");
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
        Log :: ERROR("NF_SVR_CORE : GET WRITE TIMEOUT EMPTY");
        return -1;
    }
    else
        return (ptr->server)->write_to;
}

int
nf_server_get_socksize(nf_server_t * sev)
{
    if (sev == NULL) 
    {
        Log :: ERROR("NF_SVR_CORE : GET SOCK SIZE EMPTY");
        return -1;
    }
    else
        return sev->socksize;
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
    
    if(sev->sev_socket < 0)
    {
        if( (sev->sev_socket = socket(PF_INET, SOCK_STREAM, 0) ) < 0 )
        {
            Log :: ERROR("NF_SVR_CORE : CREATE SOCKET ERROR : %s", strerror(errno));
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
    
    //ret = svr_init(sev); 
    //Log :: NOTICE("NF_SVR_CORE : INIT POOL STATUS : %d", ret);
    return 0;
}

*/
