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
    char buf[2048];
    char readbuf[2048];
    struct sockaddr_in server_address;
    char ip[] = "127.0.0.1";
    set_tcp_sockaddr(ip, 1025, &server_address);

    for(int i = 0; i < 1; i++)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        set_linger(fd, 0);
        int size = createData(buf, i); 
        int n = net_connect_to_ms(fd, (struct sockaddr *)&server_address, sizeof(server_address), 3000, 1);
        if(n < 0)
        {
            std::cout << "connect error" << std::endl;
            continue;
        }
        buf[size] = '\0';
        //std :: cout << buf << std :: endl;
        for(int j = 0; j > -1; j++)
        {
            int tes = 2;
            if (send(fd, buf, 2, 0) <= 0)
                std::cout <<  "send error"  << ": " << strerror(errno) << std::endl;
            sleep(1);
            if (send(fd, buf + 2, size - 2, 0) <= 0)
                std::cout <<  "send error"  << ": " << strerror(errno) << std::endl;
            
            if((n = recv(fd, readbuf, size, MSG_WAITALL)) <= 0)
                std::cout <<  "recv error"  << ": " << strerror(errno) << std::endl;
            //std :: cout << "recv once succ" << std :: endl;
            //std::cout <<  readbuf  << " : " << n << std::endl;
        }
        //readbuf[n] = '\0';
        close(fd);        
    }
}

int main(int argc, char *argv[])
{
    char buf[128] = "12345";
    char readbuf[128];
    pthread_t tid[100];
    for(int i = 0; i < 100; i++)
    {
        if( pthread_create(&tid[i], NULL, work, NULL) != 0) 
            return -1; 
    }
    pthread_join(tid[0], NULL);
}
