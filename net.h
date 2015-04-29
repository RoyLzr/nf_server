//**********************************************************
//          Net Utility 1.0
//
//  Description:
//  ���� ���� ����
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


typedef struct _rio_t 
{
    int rio_fd;
    int rio_cnt;
    size_t rio_len;
    char *rio_bufptr;
    char *rio_buf;
} rio_t;


extern void rio_init(rio_t *rp, int fd);



ssize_t rio_readn(rio_t *rp, void *buf, int n);



ssize_t rio_readline(rio_t * rp, void *buf, int maxlen);



extern int connect_retry(int family, int type, int protcol, 
                         const struct sockaddr *addr, 
                         size_t len, size_t maxsleep = 64);
/**
 *  connect()�İ�װ����
 *  @note �����ͷ���ֵ��connect()��ͬ
 *        maxsleep �������ȴ�ʱ��
 */

extern ssize_t sendn(int fd, const void *ptr, size_t n, size_t maxtime);


/**
 *  send()�İ�װ����
 *  @note �����곤��n
 *        
 */

extern ssize_t readn(int fd, void *ptr, size_t n);

/**
 *  recv()�İ�װ����
 *  @note �����곤��n
 *        
 */

extern ssize_t net_socket(int domain, int type, int protocol);
/**
 *  socket()�İ�װ����
 *  @note � socket
 *        
 */

extern int naccept(int, struct sockaddr *, socklen_t *);
/**
 *  accept()�İ�װ����
 *  @note get accept
 *        
 */

extern int nepoll_create(int);
/**
 *  epoll_create()�İ�װ����
 *  @note epoll_create
 *        
 */

extern int nepoll_add(int, int);
/**
 *  epoll_ctl()�İ�װ����
 *  @note add 
 *   EPOLLIN | EPOLLHUP | EPOLLERR;
 */

extern int nepoll_add_one(int, int);
/**
 *  epoll_ctl()�İ�װ����
 *  @note add 
 *   EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLONESHOT;
 */

extern int nepoll_del(int, int, int closed = 1);
/**
 *  epoll_del()�İ�װ����
 *  @note del 
 *   param1: epfd   param2: fd
 *   closed: error close fd or not
 */

extern int set_fd(int, int, int closed = 1);
/**
 *  fcntl()��װ����
 *  @note add 
 *   param1: fd   param2: flag
 *   closed: error close fd or not
 */

extern int set_clc_fd(int, int, int closed = 1);
/**
 *  fcntl()��װ����
 *  @note clear
 *   param1: fd   param2: flag
 *   closed: error close fd or not
 */

extern int set_fd_noblock(int);
/**
 *  set_fd()��װ����
 *  @note set noblock
 *   param1: fd 
 */

extern int set_fd_block(int);
/**
 *  set_fd()��װ����
 *  @note set block
 *   param1: fd 
 */

extern int read_line(int, void *, int);
/**
 *  readn()��װ����
 *  @note read data from stream as line
 *        param1: fd, param2: buff, param3:size 
 */

extern ssize_t readn_PEER(int fd, void *ptr, size_t n);
/**
 *  readn()��װ����
 *  @note read data as MSG_PEEK
 *        param1: fd, param2: buff, param3:size 
 */

#endif
