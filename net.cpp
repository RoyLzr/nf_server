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


int connect_retry(int family, int type, int protcol, 
                  const struct sockaddr *addr, 
                  size_t len, size_t maxsleep)
{
    int sec, fd;
    //BSD 第一次连接失败，酝幻枋龇� 之 后的尝试都会失败
    //每次连接都生产新的描述符
    for(sec = 1; sec <= maxsleep; sec <<=1)
    {
        if((fd = socket(family, type, protcol)) < 0)
            return -1;
        if(connect(fd, addr, len) == 0)
        {
            return fd;
        }
        //失败关闭 fd
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
            //第一次写失败了
           if(nleft ==  n)
                return -1;
           else
                break;
        }
        else if(nwrite == 0) // 写完了
            break;
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
            //第一次写失败了
           if(nleft ==  n)
                return -1;
           else
                break;
        }
        else if(nread == 0) // 写完了
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


//epfd fd 文件描述符，为索引作用，所以不用传指针
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

