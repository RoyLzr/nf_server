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



void rio_init(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

//´øbuf²Ù×÷
//ÏÈ¶ÁÒ»´Îµ½ buf ÖÐ£¬È»ºóÏûºÄbufÖÐÊý¾Ý
static ssize_t rio_read(rio_t *rp, char *buf, int n)
{
    int cnt = 0;

    while( rp->rio_cnt <= n )
    {
        rp->rio_cnt = recv(rp->rio_fd, rp->rio_buf, 
                           rp->rio_len, 0);
        if( rp->rio_cnt < 0)
        {
            if( errno != EINTR)
                return -1;
        }
        else if( rp->rio_cnt == 0)
        {
            return 0;
        }
        else
            rp->rio_bufptr = rp->rio_buf;
    }
    
    cnt = n;
    if( rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(buf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

ssize_t rio_readn(rio_t *rp, void *buf, int n)
{
    int nleft = n;
    int nread;
    char *tmp = (char *)buf;
    
    while( nleft > 0)
    {
        if( (nread = rio_read(rp, tmp, nleft)) < 0)
        {
            if( errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if( nread == 0)
            break;
        nleft -= nread;
        tmp += nread;
    }
    return n - nleft;
}

ssize_t rio_readline(rio_t * rp, void *buf, int maxlen)
{
    int i, nread;
    char c, *tmp = (char *)buf;
    for( i = 1; i < maxlen; i++)
    {
        if( (nread = rio_read(rp, &c, 1)) == 1)
        {
            *tmp = c;
            tmp++;
            if(c == '\n')
                break;
        }
        else if( nread == 0)
        {
            if( i == 1)
                return 0;
            else
                break;
        }
        else
            return -1;
    }
    *tmp = 0;
    return i;
}

int connect_retry(int family, int type, int protcol, 
                  const struct sockaddr *addr, 
                  size_t len, size_t maxsleep)
{
    int sec, fd;
    //BSD µÚÒ»´ÎÁ¬½ÓÊ§°Ü£¬ÔÍ¬Ò»ÃèÊö·û Ö® ºóµÄ³¢ÊÔ¶¼»áÊ§°Ü
    //Ã¿´ÎÁ¬½Ó¶¼Éú²úÐÂµÄÃèÊö·û
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
    //EAGAIN will end
    nleft = n;
    while(nleft > 0)
    {
        if((nwrite = send(fd, ptr, nleft, 0)) < 0)
        {
            //µÚÒ»´ÎÐ´Ê§°ÜÁË
           if(errno == EINTR)
                nwrite = 0;
           else
                break;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return n - nleft;
}

ssize_t readn(int fd, void *ptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    //EAGAIN will end
    nleft = n;
    while(nleft > 0)
    {
        if((nread = recv(fd, ptr, nleft, 0)) < 0)
        {
           if(errno ==  EINTR)
                nread = 0; 
           else if(errno == EAGAIN)
                break;
           else
                return -1; //ÖÐ¼äÊ§°Ü·µ»Ø-1
        }
        else if(nread == 0) // Ð´ÍêÁË
            break;
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

ssize_t readn_PEER(int fd, void *ptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    //EAGAIN will end
    nleft = n;
    while(nleft > 0)
    {
        if((nread = recv(fd, ptr, nleft, MSG_PEEK)) < 0)
        {
           if(nleft == EINTR)
                nread = 0;
           else
                return -1;
        }
        else if(nread == 0) // Ð´ÍêÁË
            break;
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
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


//epfd fd ÎÄ¼þÃèÊö·û£¬ÎªË÷Òý×÷ÓÃ£¬ËùÒÔ²»ÓÃ´«Ö¸Õë
int nepoll_add(int epfd, int fd)
{
    struct epoll_event ev;
    int ret;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

int nepoll_add_one(int epfd, int fd)
{
    struct epoll_event ev;
    int ret;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLONESHOT;
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


//read line from socket
//µÍÐ§ ÎÞ buf
int read_line(int fd, void * tmp, int size)
{
    char c = '\0';
    char * buf = (char *)tmp;
    int rlen;
    int i = 0;
    while( (i < size -1) && (c != '\n'))
    {
        rlen = readn(fd, &c, 1);
        if( rlen > 0)
        {
            if(c == '\r')
            {
                rlen = readn_PEER(fd, &c, 1);
                if (c == '\n' && rlen > 0)
                    readn(fd, &c, 1);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        }
        else
            c = '\n';
    }
    return i;
}

