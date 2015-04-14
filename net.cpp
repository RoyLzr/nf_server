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

ssize_t sendn(int fd, const void *ptr, size_t n)
{
    size_t nleft;
    ssize_t nwrite;

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

