//**********************************************************
//          Net Utility 1.0
//
//  Description:
//  网络 基本 操作
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef __NET_H__
#define __NET_H__

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <endian.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <netinet/tcp.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/epoll.h>


extern int connect_retry(int family, int type, int protcol, 
                         const struct sockaddr *addr, 
                         size_t len, size_t maxsleep = 64);
/**
 *  connect()的包装函数
 *  @note 参数和返回值与connect()相同
 *        maxsleep 重试最大等待时间
 */

extern ssize_t sendn(int fd, const void *ptr, size_t n, size_t maxtime);


/**
 *  send()的包装函数
 *  @note 发送完长度n
 *        
 */

extern ssize_t readn(int fd, void *ptr, size_t n);

/**
 *  recv()的包装函数
 *  @note 发送完长度n
 *        
 */

extern ssize_t net_socket(int domain, int type, int protocol);

extern int naccept(int, struct sockaddr *, socklen_t *);

extern int nepoll_create(int);

extern int nepoll_add(int, int);

extern int nepoll_del(int, int, int closed = 1);

extern int set_fd(int, int, int closed = 1);

extern int set_clc_fd(int, int, int closed = 1);

extern int set_fd_noblock(int);

extern int set_fd_block(int);


#endif
