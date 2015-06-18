#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../net.h"
#include <pthread.h>

#define max 128

void * work(void * arg)
{
    char buf[] = "12345";
    char readbuf[128];
    struct sockaddr_in server_address;
    char ip[] = "127.0.0.1";
    set_tcp_sockaddr(ip, 1025, &server_address);

    for(int i = 0; i < 1000; i++)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        //set_linger(fd, 0); 
        int n = net_connect_to_ms(fd, (struct sockaddr *)&server_address, sizeof(server_address), 3000, 1);
        if(n < 0)
            std::cout << "connect error" << std::endl;

        if (send(fd, buf, sizeof(buf) - 1, 0) <= 0)
            std::cout <<  "error"  << ": " << i << std::endl;
            
        if( (n = recv(fd, readbuf, sizeof(buf) - 1, MSG_WAITALL)) <= 0)
            std::cout <<  "error"  << ": " << i << std::endl;
        
        //readbuf[n] = '\0';
        std::cout <<  readbuf  << ": " << i <<":" << n << std::endl;
        close(fd);        
    }
}

int main(int argc, char *argv[])
{
    char buf[128] = "12345";
    char readbuf[128];
    pthread_t tid[100];
    for(int i = 0; i < 1; i++)
    {
        if( pthread_create(&tid[i], NULL, work, NULL) != 0) 
            return -1; 
    }
    pthread_join(tid[0], NULL);
}
