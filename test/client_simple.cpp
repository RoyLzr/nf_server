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
#include "../nf_server/net.h"



#define max 128
class Client
{
    private:
        const int port;
        const char *address;
    public:
        Client(const int _port, const char *_address):port(_port),address(_address){}
        inline const int get_port() const
        {
            return port;
        }
        inline const char * get_address() const
        {
            return address;
        }
        int connect_retry(int, int, int, const struct sockaddr *, socklen_t);
};



int main(int argc, char *argv[])
{
    char buf[128];
    for(int i = 0; i < 10; i++)
    {   Client client(1025, "127.0.0.1");

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = inet_addr(client.get_address());
        server_address.sin_port = htons(client.get_port());

        int fd = connect_retry(AF_INET, SOCK_STREAM, 0, (struct sockaddr *)(&server_address), 
                                      sizeof(server_address));
        
        if (fd == -1)
            std::cout << "error" << std::endl;
        int n;
        while(n = recv(fd, buf, 128, MSG_WAITALL) > 0){}
        
        std::cout << buf  <<  std::endl;
        std::cout << n  <<  std::endl;
        close(fd);
    }
}
