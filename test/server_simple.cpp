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

#define max 128
#define BUFLEN 128
class Server
{
    private:
        const int port;
    public:
        Server(const int _port):port(_port){}
        inline const int get_port() const
        {
            return port;
        }
        int initserver(int, const struct sockaddr *, socklen_t, int);
        void run(int sockfd);
        
};

int Server::initserver(int type, const struct sockaddr * addr, socklen_t len, int qlen)
{
    int fd, err;
    int value = 1;
    if((fd = socket(addr->sa_family, type, 0)) < 0 )
    {   
        std::cout << strerror(errno) << std::endl;
        return -1;
    }
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int)) < 0)
        goto errout;
    if(bind(fd, addr, len) < 0)
        goto errout;
    if(type == SOCK_STREAM || type == SOCK_SEQPACKET) 
        if(listen(fd, qlen) < 0)
            goto errout;

    return fd;

    errout:
        err = errno;
        close(fd);
        errno = err;
        std::cout << strerror(errno) << std::endl;
        return -1;        
}


void Server::run(int fd)
{
    int test_fd;
    int result;
    fd_set readfds, testfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    char buf[BUFLEN] = {'1', '2', '3', '4', '5'};
    while(true)
    {
        testfds = readfds;
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, 
                        (fd_set *)0, (struct timeval *)0);
        if(result < 1)
            std::cout << strerror(errno) << std::endl;
        
        for(test_fd = 0; test_fd < FD_SETSIZE; test_fd++)
        {
            if(FD_ISSET(test_fd, &testfds))
            {
                if(test_fd == fd)
                {
                    struct sockaddr_in client_address;
                    int client_len = sizeof(client_address);
                    int client_fd = accept(fd, (struct sockaddr *) &client_address, 
                                           (socklen_t *)&client_len);
                    //FD_SET(client_fd, &readfds);
                    std::cout << "get client_sock succ" <<std::endl;
                    std::cout << ntohs(client_address.sin_port) <<std::endl;
                    send(client_fd, buf, strlen(buf), 0);
                    //sleep(5);
                    close(client_fd);
                    break;
                }
            }
        }
    }  
}

int main(int argc, char *argv[])
{
    Server server(1025);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(1025);
    int fd = server.initserver(SOCK_STREAM, (struct sockaddr *)(&server_address), 
                                  sizeof(server_address), 20);
    if (fd == -1)
    {
        std::cout << "error" << std::endl;
        exit(0);
    }
    server.run(fd);
        
}
