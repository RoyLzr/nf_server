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

void 
default_handle()
{
    char * read_buf = (char *) nf_server_get_read_buf(); 
    char * write_buf = (char *) nf_server_get_write_buf(); 
    int readed_size = nf_server_get_readed_size();
    strncpy(write_buf, read_buf, readed_size);

    if (nf_server_set_writed_size(readed_size) < 0)
        std :: cout << "set writed size error " << std :: endl;
    nf_server_set_writed_start(readed_size);     
}


int main(int argc, char *argv[])
{
    
    signal(SIGINT, default_hand);

    nf::NfServer *test =  new nf::NfServer();
    char s[] = "test server";
    test->set_server_name(s);
    string conf("server.conf");
    test->load_conf(conf);
     
    test->set_work_callback(new RaReadLine()); 
    test->set_handle(default_handle);
     
    if (test->run() < 0)
        std::cout << strerror(errno) << std::endl; 
    
    int time = sleep(65535);
    
    test->stop();
     
    test->destroy();
    
    free(test);
    
    std::cout << "run time : " << 65535 - time << std::endl;    

}
