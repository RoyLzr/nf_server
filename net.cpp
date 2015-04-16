//**********************************************************
//          Net Utility 1.0
//
//  Description:
//  ÍøÂç »ù±¾ ²Ù×÷
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#include "net.h"


int connect_retry(int family, int type, int protcol, 
                  const struct sockaddr *addr, 
                  size_t len, size_t maxsleep)
{
    int sec, fd;
    //BSD µÚÒ»´ÎÁ¬½ÓÊ§°Ü£¬ÔÍ¬Ò»ÃèÊö·û Ö® ºóµÄ³¢ÊÔ¶¼»áÊ§°Ü
    //Ã¿´ÎÁ¬½Ó¶¼Éú²úĞÂµÄÃèÊö·û
    for(sec = 1; sec <= maxsleep; sec <<=1)
    {
        if((fd = socket(family, type, protcol)) < 0)
            return -1;
        if(connect(fd, addr, len) == 0)
        {
            return fd;
        }
        //Ê§°Ü¹Ø±Õ fd
        close(fd);
        std::cout << "retry" << std::endl;
        if (sec <= maxsleep)
            sleep(sec);
    }
    return -1; 
}

ssize_t sendn(int fd, const void *ptr, size_t n, size_t maxtime)
{
    size_t nleft;
    ssize_t nwrite;

    nleft = n;
    while(nleft > 0)
    {
        if((nwrite = send(fd, ptr, nleft, 0)) < 0)
        {
            //µÚÒ»´ÎĞ´Ê§°ÜÁË
           if(nleft ==  n)
                return -1;
           else
                break;
        }
        else if(nwrite == 0) // Ğ´ÍêÁË
            break;
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n - nleft;
}

ssize_t readn(int fd, void *ptr, size_t n)
{
    int result;
    result = recv(fd, ptr, n, MSG_WAITALL);
    return result;
}

ssize_t net_socket(int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol);
    if( fd < 0)
        return -1;
    return fd;
}

int nepoll_create(int size)
{
    return epoll_create(size);    
}


//epfd fd ÎÄ¼şÃèÊö·û£¬ÎªË÷Òı×÷ÓÃ£¬ËùÒÔ²»ÓÃ´«Ö¸Õë
int nepoll_add(int epfd, int fd)
{
    struct epoll_event ev;
    int ret;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}


int nepoll_del(int epfd, int fd, int closed)
{
    int ret;
    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    if(ret < 0 && closed == 0)
    {
        close(fd);
        return ret;
    }
    return ret; 
}

int set_fd(int fd, int flags, int closed)
{
    int val;
    if ( (val = fcntl(fd, F_GETFL, 0)) < 0)
    {
        std::cout << "fd get error" << std::endl;
        if(closed == 0)
        {
            close(fd);
        }
        return -1;
    }
    val |= flags;
    if ( (fcntl(fd, F_SETFL, val)) < 0)
    {
        std::cout << "fd get error" << std::endl;
        if(closed == 0)
            close(fd);
        return -1;
    }
    return 0;
}


int set_clc_fd(int fd, int flag, int closed)
{
    int val;
    if ( (val = fcntl(fd, F_GETFL, 0)) < 0)
    {
        std::cout << "fd get error" << std::endl;
        if(closed == 0)
            close(fd);
        return -1;
    }
    
    if ( val & O_NONBLOCK)
    {
        val &= ~O_NONBLOCK;
        //std::cout << "need change" << std::endl;
        if ( (fcntl(fd, F_SETFL, val)) < 0)
        {
            std::cout << "fd get error" << std::endl;
            if(closed == 0)
                close(fd);
            return -1;
        
        }
    }
    
    return 0;
}

int set_fd_noblock(int fd)
{
   return set_fd(fd, O_NONBLOCK, 1);
}

int set_fd_block(int fd)
{
   return set_clc_fd(fd, O_NONBLOCK, 1);
}

int naccept(int fd, struct sockaddr * addr, socklen_t *len)
{
    int cfd;
    if( ( cfd = accept(fd, addr, len)) < 0)
    {
        if(errno == EAGAIN)
        { std::cout << "eagain discard accept error" << std::endl; return 0;}
        else
        { std::cout << strerror(errno) << std::endl;  return -1;}     
    }
    return cfd; 
}

