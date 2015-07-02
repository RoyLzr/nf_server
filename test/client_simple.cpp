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
#include <time.h>

#define max 128

int createData(char * tmp, int off)
{
    srand((unsigned)time(0) + off);
    int num = rand() % 900;
    for(int i = 0; i < num; i++)
    {
        *(tmp + i) = '0' + i % 10; 
    }
    *(tmp + num) = '\n';
    return num + 1; 
}

void * work(void * arg)
{
    char buf[9999];
    char readbuf[9999];
    struct sockaddr_in server_address;
    char ip[] = "127.0.0.1";
    set_tcp_sockaddr(ip, 1025, &server_address);

    for(int i = 0; i < 1; i++)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        set_linger(fd, 0);
        int size = createData(buf, i);

        std::cout << "begin " << i << std::endl; 
        int n = net_connect_to_ms(fd, (struct sockaddr *)&server_address, sizeof(server_address), 1, 1);
        if(n < 0)
        {
            std::cout << "connect error :"  << strerror(errno)<< std::endl;
            continue;
        }
        std :: cout << i << std::endl;
        buf[size] = '\0';

        //std :: cout << buf << std :: endl;
        int j = 0;
        //for(j = 0; j < 1; j++)
        while(true)
        {
            int tes = 2;
              
            if (send(fd, buf, size, 0) <= 0)
                std::cout <<  "send error"  << ": " << strerror(errno) << std::endl;
            if (send(fd, buf, size, 0) <= 0)
                std::cout <<  "send error"  << ": " << strerror(errno) << std::endl;
            
            if((n = recv(fd, readbuf, size * 2, MSG_WAITALL)) <= 0)
                std::cout <<  "recv error"  << ": " << strerror(errno) << std::endl;
        
            std :: cout << "recv once succ : " << j++ << std :: endl;
            //std::cout <<  readbuf  << " : " << n << std::endl;
        }
        readbuf[n] = '\0';
        //close(fd);        
    }
    sleep(65535);
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
