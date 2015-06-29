//**********************************************************
//          Net Utility 1.0
//
//  Description:
//  ÍøÂç »ù±¾ ²Ù×÷
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
#include <signal.h>
#include <limits.h>
#include "commonn/asynLog.h"
#include "commonn/timer.h"
#include <stdarg.h>

typedef struct _rio_t
{
    int rio_fd;
    int rio_cnt;      // ÄÚÖÃ·â×°readÊ±Ê¹ÓÃ
    size_t rio_len;   // ²»½¨ÒéÊ¹ÓÃ·â×°read²Ù×÷
    char * rio_bufptr;// ½¨ÒéÖ±½Ó¶Áµ½Ïß³Ì¹¤×÷¿¿Õ¼ä 
    char * rio_ptr;   

    char * cache;
    int cache_len;

    char * w_cache;
    int w_cache_len;
    int w_allo_len;    
 
} rio_t;

void  
rio_init(rio_t *rp, int fd, int len);

ssize_t 
rio_readn(rio_t *rp, void *usrbuf, size_t n, int * st = NULL);

ssize_t
rio_readline(rio_t *rp, void *usrbuf, size_t maxlen, int * st = NULL);

ssize_t
sendn(int fd, void *usrbuf, size_t n);

int 
set_fd(int fd, int flags);

int 
set_fd_noblock(int fd);

int 
set_clc_fd(int fd, int flags);

int
set_fd_block(int fd);

int
set_tcp_sockaddr(char * addr, int port, 
                 struct sockaddr_in * soin);

const char *
get_tcp_sockaddr(char * addr, int * port, 
                 struct sockaddr_in * soin, int len);

int
net_connect_to_tv(int fd, struct sockaddr * sa, 
                socklen_t socklen, timeval * tv, int isclose = 1);


int
net_connect_to_ms(int sockfd, struct sockaddr *sa, 
                  socklen_t socklen, int msecs, int isclose = 1);


int
net_accept(int sockfd, struct sockaddr *sa, socklen_t * addrlen);


int
net_tcplisten(int port, int queue);


ssize_t 
readn_to_ms(int fd, void *ptr, size_t nbytes, int msecs);


ssize_t 
rio_readn_to_ms(rio_t *rp, void *usrbuf, size_t n, int msecs);


ssize_t
rio_readline_to_ms(rio_t *rp, void *usrbuf, size_t maxlen, int msecs);


ssize_t 
sendn_to_ms(int sock, const void *ptr, size_t nbytes, int msecs);

extern int net_ep_create(int);
/**
 *  epoll_create()µÄ°ü×°º¯Êý
 *  @note epoll_create
 *        
 */

extern int net_ep_add(int, int, int);
/**
 *  epoll_ctl()µÄ°ü×°º¯Êý
 *  @note add 
 *  
 */

extern int net_ep_add_in(int, int);
/**
 *  epoll_ctl()µÄ°ü×°º¯Êý
 *  @note add 
 *   EPOLLIN | EPOLLHUP | EPOLLERR;
 */

extern int net_ep_add_in1(int, int);
/**
 *  epoll_ctl()µÄ°ü×°º¯Êý
 *  @note add 
 *   EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLONESHOT ;
 */

extern int net_ep_del(int, int);
/**
 *  epoll_del()µÄ°ü×°º¯Êý
 *  @note del 
 *   param1: epfd   param2: fd
 *   closed: error close fd or not
 */

void 
default_hand(int sig);

int
set_linger(int fd, int val);

int 
find_line(char * req, int end);

int
readn(int fd, void *usrbuf, size_t n);

void
move_forward(char * req, int start, int end);

#endif
