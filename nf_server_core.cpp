#include "nf_server_core.h"

//1. SO_LINGER 2. TCP_NODELAY 3. DEFER_ACCEPT
int set_sev_socketopt(nf_server_t *sev, int fd)
{
    const int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    
    struct linger li;
    memset(&li, 0, sizeof(li)); 
    li.l_onoff = 1;
    li.l_linger = 1;
    setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&li, sizeof(li) );     
    
    //Ä¬ÈÏ¿ªÆôTCP_DEFER_ACCCEPT, don't wait last ack , wait first data

    int timeout = sev->connect_to;
    setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(timeout));
    
    return 0; 
}

int nf_server_bind(nf_server_t * sev)
{
    //set svr : listen socket 
    const int on = 1;
    struct sockaddr_in addr;
    
    if(sev->sev_socket < 0)
    {
        if((sev->sev_socket = socket(PF_INET, SOCK_STREAM, 0) ) < 0 )
        {
            Log :: ERROR("NF_SVR_CORE : CREATE SOCKET ERROR : %s", strerror(errno));
            return -1; 
        }
    }
    //¸´ÓÃ
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
    
    return 0;
}

int nf_server_listen(nf_server_t * sev)
{
    int backlog = sev->backlog;

    if(backlog <= 0)
        backlog = 5;

    if(listen(sev->sev_socket, backlog) < 0)
    {
        Log :: ERROR("SET LISTEN SOCKET ERROR");
        close(sev->sev_socket);
        return -1;
    }
    Log :: NOTICE("SET LISTEN SOCKET SUCC");
    
    return 0;
}

