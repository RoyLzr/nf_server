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
#include "../nf_server.h"
#include "../commonn/singleton.h"

#define max 128
#define BUFLEN 128

int main(int argc, char *argv[])
{
    nf::NfServer *test =  new nf::NfServer();
    char s[] = "test server";
    test->set_server_name(s);
    test->load_conf("../server.conf");
    
    if (test->run() < 0)
        std::cout << strerror(errno) << std::endl; 
    
    sleep(120);
    
    std::cout << test->get_server_data()->run << "out" <<std::endl;
    test->stop();
    
    test->destroy();
    
    free(test);
}
