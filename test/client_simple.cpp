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

#define max 128

int main(int argc, char *argv[])
{
    char buf[128] = "12345";
    char readbuf[128];
    for(int i = 0; i < 1000; i++)
    {
   
        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_address.sin_port = htons(1025);

        int fd = connect_retry(AF_INET, SOCK_STREAM, 0, (struct sockaddr *)(&server_address), 
                                      sizeof(server_address));
        
        if (fd == -1)
            std::cout << "error" << std::endl;

        int n;
        if (send(fd, buf, strlen(buf), 0) <= 0)
            std::cout <<  "error"  << ": " << i << std::endl;
            
        memset(readbuf, 0, sizeof(readbuf)); 
        if(n = recv(fd, readbuf, strlen(buf), MSG_WAITALL) > 0)
            std::cout <<  readbuf  << ": " << i << std::endl;
        else
            std::cout <<  "error"  << ": " << i << std::endl;
            
        close(fd);
    }
}
