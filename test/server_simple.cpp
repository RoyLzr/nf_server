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

#define max 128
#define BUFLEN 128

int main(int argc, char *argv[])
{
    
    signal(SIGINT, default_hand);
    signal(SIGHUP, default_hand);

    nf::NfServer *test =  new nf::NfServer();
    char s[] = "test server";
    test->set_server_name(s);
    string conf("server.conf");
    test->load_conf(conf);
    
    //test->set_work_callback(nf_SA_readline_worker); 
    test->set_work_callback(nf_RA_readline_worker); 
    //test->set_work_readfun( nf_default_read );
    //test->set_work_writefun( nf_default_write );
     
    if (test->run() < 0)
        std::cout << strerror(errno) << std::endl; 
    
    int time = sleep(65535);
    
    test->stop();
     
    test->destroy();
    
    free(test);
    
    std::cout << "run time : " << 65535 - time << std::endl;    

}
