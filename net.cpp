//**********************************************************
//          Net Utility 1.0
//
//  Description:
//  网络 基本 操作
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#include "net.h"

void rio_init(rio_t *rp, int fd, int len)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_len = len;
    rp->rio_bufptr = rp->rio_ptr;

    rp->cache = NULL;
    rp->cache_len = 0;

    rp->w_cache = NULL;
    rp->w_cache_len = 0;
}

static ssize_t
rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;
    while(rp->rio_cnt <= 0)
    {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_ptr, rp->rio_len);
        if(rp->rio_cnt == 0)
            return 0;
        else if(rp->rio_cnt < 0)
        {
            if(errno != EINTR)
                return -1;
        } 
        else
            rp->rio_bufptr = rp->rio_ptr;      
    }
    if((size_t)rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    else
        cnt = n;
    
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;  
    return cnt;
}

int 
readn(int fd, void *usrbuf, size_t n)
{
    int nleft = n;
    int nread = 0;
    char * buf = (char *)usrbuf;
    
    while(nleft > 0)
    {
        nread = read(fd, buf, nleft);
        if(nread == 0)
            break;
        else if(nread < 0)
        {
            if(errno == EAGAIN)
            {
                break;
            }
            if(errno == EINTR)
            {
                nread = 0;
                continue;
            }
            return -1;
        }
        nleft -= nread;
        buf += nread;
    }
    return n - nleft;
}

ssize_t 
rio_readn(rio_t *rp, void *usrbuf, size_t n, int * st)
{
    int nleft = n;
    int nread = 0;
    char * buf = (char *)usrbuf;
    *st = 1;
    while(nleft > 0)
    {
        nread = rio_read(rp, buf, nleft);
        if(nread == 0)
            break;
        else if(nread < 0)
        {
            if(errno == EAGAIN)
            {
                *st = -1;
                break;
            }
            if(errno != EINTR)
                return -1;
            nread = 0;
        }
        nleft -= nread;
        buf += nread;
    }
    return n - nleft;
}

ssize_t
rio_readline(rio_t *rp, void *usrbuf, size_t maxlen, int * st)
{
    char c, *buf = (char *)usrbuf;
    int i;
    int nread;
    *st = 1;
    int len = maxlen;    

    for(i = 1; i < len; i++)
    {
        nread = rio_read(rp, &c, 1);
        if(nread == 0)
        {
            return 0;
        }
        else if(nread < 0)
        {
            if(errno == EAGAIN)
            { 
                *st = -1;   
                break;
            }
            else
                return -1;
        }
        else
        {
            *buf = c;
            buf++;
            if(c == '\n')
                break;
        }     
    }
    *buf = '\0';
    return i;
}

ssize_t 
readn_to_ms(int fd, void *ptr, size_t nbytes, int msecs)
{
    struct timeval tv;
    struct timeval old_tv;
    ssize_t nread;
    int sockflag;
    int ret;
    int timeuse = 0;
    socklen_t oplen = (socklen_t) sizeof (tv);
    
    nread = recv(fd, ptr, nbytes, MSG_DONTWAIT);
    
    if(nread == 0)
        return 0;
    if((nread < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK) &&(errno != EINTR))
        return -1;
    if((size_t)nread == nbytes)
        return nread;
    if (nread < 0) 
    {
        nread = 0;
    }

    if (0 == msecs) 
    {
        errno = ETIMEDOUT;
        return -1;
    }
    if (msecs < 0) {
       msecs = INT_MAX; 
    }

    if(getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &old_tv, &oplen) < 0)
        return -1;
    if((sockflag = fcntl(fd, F_GETFL, 0)) < 0)
        return -1;

    if (sockflag&O_NONBLOCK) 
    {
        if(fcntl(fd, F_SETFL, (sockflag)&(~O_NONBLOCK)) < 0)
            return -1;
    }

    struct timeval cur;
    struct timeval last;
    do 
    {
        tv.tv_sec = msecs/1000;
        tv.tv_usec = (msecs % 1000) * 1000;
        if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, oplen) < 0)
            return -1;
        gettimeofday(&cur, NULL);
        
        do 
        {
            ret = recv(fd, (char*)ptr + nread, nbytes - (size_t)nread, MSG_WAITALL); 
        } while (ret < 0 && errno == EINTR);
        
        if (ret < 0 && EAGAIN==errno) 
        {
            //没数据，超时了
            errno = ETIMEDOUT;
        }
        
        if (ret > 0 && nread + ret != (ssize_t) nbytes) 
        {
            gettimeofday(&last, NULL);
            //判断是不是真的超时了
            timeuse = ((last.tv_usec - cur.tv_usec)/1000 + (last.tv_sec - cur.tv_sec) * 1000);
            if (timeuse >= msecs) 
            {
                //真超时了
                errno = ETIMEDOUT;
                ret = -1;
                nread = -1;
            } 
            else 
            {
                //被中断了？继续读
                nread += ret;
            }
        }  
        else if (ret < 0) 
        {
            nread = -1;
        } 
        else 
        {
            nread += ret;
            msecs -= timeuse;
        }
      //没写完，继续
    } while (ret > 0 && nbytes != (size_t)nread);

    if (sockflag & O_NONBLOCK) 
    {
        if(fcntl(fd, F_SETFL, sockflag) < 0)
            return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &old_tv, oplen)<0 )
        return -1;

    return nread;
}

ssize_t 
rio_readn_to_ms(rio_t *rp, void *usrbuf, size_t n, int msecs)
{
    int nleft = n;
    int nread = 0;
    int sockflag;
    char * buf = (char *)usrbuf;

    if((sockflag = fcntl(rp->rio_fd, F_GETFL, 0)) < 0)
        return -1;
    set_fd_noblock(rp->rio_fd);
    
    struct timeval tv;
    tv.tv_sec = msecs/1000;
    tv.tv_usec = (msecs % 1000) * 1000; 
 
    while(nleft > 0)
    {
        nread = rio_read(rp, buf, nleft);
        if(nread == 0)
            break;
        else if(nread < 0)
        {
            if(errno == EAGAIN)
            {
               fd_set rset;
               FD_ZERO(&rset);
               FD_SET(rp->rio_fd, &rset);
               if(select(rp->rio_fd + 1, &rset, NULL, NULL, &tv) <= 0)
               {
                    errno = ETIMEDOUT;
                    n = nleft - 1;
                    break;
               }   
               else
                   continue; 
            }
            if(errno != EINTR)
                return -1;
            nread = 0;
        }
        nleft -= nread;
        buf += nread;
    }
    if(fcntl(rp->rio_fd, F_SETFL, sockflag) < 0)
        return -1;

    return n - nleft;
}

ssize_t
rio_readline_to_ms(rio_t *rp, void *usrbuf, size_t maxlen, int msecs)
{
    char c, *buf = (char *)usrbuf;
    int i;
    int nread;
    int sockflag;
    
    struct timeval tv;
    tv.tv_sec = msecs/1000;
    tv.tv_usec = (msecs % 1000) * 1000;  
    
    int len = maxlen;
    if((sockflag = fcntl(rp->rio_fd, F_GETFL, 0)) < 0)
        return -1;
    set_fd_noblock(rp->rio_fd);

    for(i = 1; i < len; i++)
    {
        nread = rio_read(rp, &c, 1);
        if(nread == 0)
        {
            return 0;
        }
        else if(nread < 0)
        {
            if(errno == EAGAIN)
            { 
               fd_set rset;
               FD_ZERO(&rset);
               FD_SET(rp->rio_fd, &rset);
               if(select(rp->rio_fd + 1, &rset, NULL, NULL, &tv) <= 0)
               {
                    errno = ETIMEDOUT;
                    i = - i;
                    break;
               }   
               else
                   continue; 
            }
            else
                return -1;
        }
        else
        {
            *buf = c;
            buf++;
            if(c == '\n')
                break;
        }     
    }
    *buf = '\0';
    if(fcntl(rp->rio_fd, F_SETFL, sockflag) < 0)
        return -1;
    return i;
}

ssize_t
sendn(int fd, void *usrbuf, size_t n)
{
    int nleft = n;
    int nwrite = 0;
    char * buf = (char *)usrbuf;
    while(nleft > 0)
    {
        nwrite = write(fd, buf, nleft);
        if(nwrite < 0)
        {
            if(errno == EINTR)
            {
                nwrite = 0;
                continue;
            }
            if(errno == EAGAIN)
            {
                break;
            }
            return -1;
        }
        else if(nwrite == 0)
            break;
        buf += nwrite;
        nleft -= nwrite;
    }
    return n - nleft;
}


ssize_t 
sendn_to_ms(int fd, const void *ptr, size_t nbytes, int msecs)
{
    struct timeval tv;
    struct timeval old_tv;
    ssize_t nwrite = 0;
    int ret = 0;
    int sockflag = 0;
    int timeuse = 0;
    socklen_t oplen = sizeof(tv);

    nwrite = send(fd, ptr, nbytes, MSG_DONTWAIT);

    if (nwrite==(ssize_t)nbytes)
        return nwrite;
    if ((nwrite < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK) && (errno != EINTR))
        return -1;
    if (nwrite == 0)
        return 0;
    if (nwrite < 0) 
    {
        nwrite = 0;
    }

    //0的时候不希望会被堵塞住
    if (0 == msecs) 
    {
        errno = ETIMEDOUT;
        return -1;
    }
    //负数的时候我们要堵塞
    if (msecs < 0) 
    {
       msecs = INT_MAX; 
    }

    if(getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &old_tv, &oplen) < 0)
        return -1;
    if((sockflag = fcntl(fd, F_GETFL, 0)) < 0) 
        return -1;

    if (sockflag & O_NONBLOCK) 
    {
        if(fcntl(fd, F_SETFL, (sockflag)&(~O_NONBLOCK)) < 0)
            return -1;
    }
    
    struct timeval cur;
    struct timeval last;
    do 
    {
        tv.tv_sec = msecs/1000;
        tv.tv_usec = (msecs%1000)*1000;
        gettimeofday(&cur, NULL);
        if(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, oplen)<0)
            return -1; 
        do 
        { 
            ret = send(fd, (char*)ptr + nwrite, nbytes-(size_t)nwrite, MSG_WAITALL); 
        } while (ret < 0 && EINTR == errno);

        if (ret > 0 && nwrite + ret < (ssize_t)nbytes) 
        {
            //判断是否真是超时
            gettimeofday(&last, NULL);
            timeuse = ((last.tv_usec - cur.tv_usec)/1000+(last.tv_sec - cur.tv_sec)*1000);
            if (timeuse >= msecs) 
            {
                //真的超时了
                errno = ETIMEDOUT;
                ret = -1;
                nwrite = -1;
            } 
            else 
            {
                //不是超时，一般是被中断了, 继续写
                msecs -= timeuse;
                nwrite += ret;
            }
        } 
        else if (ret < 0) 
        {
            nwrite = -1;
        } 
        else 
        {
            nwrite += ret;
        }
    } while (ret > 0 && nbytes != (size_t)nwrite);
    
    if (fcntl(fd, F_SETFL, sockflag) < 0)
        return  -1;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &old_tv, oplen) < 0)
        return -1;
    
    return nwrite;
}

int 
set_fd(int fd, int flags)
{
    int val;
    int sockflag;
    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
    {
        close(fd);
        return -1;
    }
    if(val & flags)
        return val;

    sockflag = val;
    val |= flags;
    if ((fcntl(fd, F_SETFL, val)) < 0)
    {
        close(fd);
        return -1;
    }
    return sockflag;
}

int
set_fd_noblock(int fd)
{
    return set_fd(fd, O_NONBLOCK);
}

int 
set_clc_fd(int fd, int flags)
{
    int val;
    int sockflag;
    if ( (val = fcntl(fd, F_GETFL, 0)) < 0)
    {
        close(fd);
        return -1;
    }
    
    sockflag = val;
    val &= ~flags;
    if ((fcntl(fd, F_SETFL, val)) < 0)
    {
        close(fd);
        return -1;
    }
    return sockflag;
}

int
set_fd_block(int fd)
{
    return set_clc_fd(fd, O_NONBLOCK);
}

int
net_accept(int sockfd, struct sockaddr *sa, socklen_t * addrlen)
{
    int connfd = 0;
    do
    {
        connfd = accept(sockfd, sa, addrlen);
        if (connfd < 0) 
        {
            if (errno == EINTR)
                continue; 
            else 
                return -1;
        }
    } while(connfd < 0);
    return connfd;
}

int
net_tcplisten(int port, int queue)
{
    int listenfd;
    const int on = 1;
    struct sockaddr_in soin;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        return -1;
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        return -1;

    bzero(&soin, sizeof(soin));

    soin.sin_family = AF_INET;
    soin.sin_addr.s_addr = htonl(INADDR_ANY);
    soin.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *) &soin, sizeof(soin)) < 0) 
    {
        close(listenfd);
        return -1;
    }
    
    if(queue <= 0)
        queue = 5;

    if (listen(listenfd, queue) < 0) 
    {
        close(listenfd);
        return -1;
    }
    return listenfd;
}

int
net_connect_to_ms(int sockfd, struct sockaddr *sa, 
                  socklen_t socklen, int msecs, int isclose)
{
    if (msecs <= 0) 
        return net_connect_to_tv(sockfd, sa, socklen, NULL, isclose);
    else 
    {
        struct timeval tv;
        tv.tv_sec = msecs / 1000;
        tv.tv_usec = (msecs % 1000) * 1000;
        return net_connect_to_tv(sockfd, sa, socklen, &tv, isclose);
    }
    return 0;
}

int
net_connect_to_tv(int fd, struct sockaddr * sa, 
                socklen_t socklen, timeval * tv, int isclose)
{
    int sockflag;
    int n, error;
    socklen_t len;
    fd_set rset, wset;

    if(sa == NULL)
    {    
        close(fd);
        return -1;
    }

    error = 0;
    sockflag = set_fd_noblock(fd);

    n = connect(fd, sa, (socklen_t) socklen);
    if (n < 0) 
    {
        if (errno != EINPROGRESS) 
        {
            error = 1;
            goto done;
        }
    }
    if (n == 0) 
        goto done;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    wset = rset;
    if((n = select(fd + 1, &rset, &wset, NULL, tv)) ==0)
    {
        errno = ETIMEDOUT;
        error = 1;
        goto done;
    }
    if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))
    {
        len = sizeof(error);
        if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
            errno = error;
            error = 1;
            goto done;
        }
    }

  done:
    fcntl(fd, F_SETFL, sockflag);
    if (error) 
    {
        if(isclose)
            close(fd);
        return -1;
    }
    return 0;
}

int
set_tcp_sockaddr(char * addr, int port, 
                 struct sockaddr_in * soin)
{
    soin->sin_family = AF_INET;
    soin->sin_port = htons(port);
    return inet_pton(AF_INET, addr, &(soin->sin_addr));
}

const char *
get_tcp_sockaddr(char * addr, int * port, 
                 struct sockaddr_in * soin, int len)
{
    *port = ntohs(soin->sin_port);
    return inet_ntop(AF_INET, &(soin->sin_addr), addr, len);
}

int 
net_ep_create(int size)
{
    return epoll_create(size);    
}

int 
net_ep_add(int epfd, int fd, int events)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

int 
net_ep_add_in(int epfd, int fd)
{
    int events = EPOLLIN | EPOLLHUP | EPOLLERR;
    return net_ep_add(epfd, fd, events);
}

int 
net_ep_add_in1(int epfd, int fd)
{
    int events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLONESHOT;
    return net_ep_add(epfd, fd, events);
}

int 
net_ep_del(int epfd, int fd)
{
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}

void 
default_hand(int sig)
{
    std::cout << "default sig fun" << std::endl;
    return;
}

int
set_linger(int fd, int val)
{
    struct linger li;
    li.l_onoff = 1;
    li.l_linger = val;
    return setsockopt(fd, SOL_SOCKET, SO_LINGER, 
                      (const char*)&li, sizeof(li));
}

int
find_line(char * req, int end)
{
    for(int i = 0; i < end; i++)
        if(*(req + i) == '\n')
            return i;
    return -1;
}

void
move_forward(char * req, int start, int end)
{
    int j = 0;
    for(int i = start; i < end; i++)
    {
        *(req + j) = *(req + i);
        j++;
    }
    return;
}

