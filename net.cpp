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

ssize_t sendn(int fd, const void *ptr, size_t n)
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

extern ssize_t readn(int fd, void *ptr, size_t n)
{
    int result;
    result = recv(fd, ptr, n, MSG_WAITALL);
    return result;
}
 

