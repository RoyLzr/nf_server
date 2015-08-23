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
#include "../factory.h"

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

    string conf("server.conf");
    Factory * f_svr = new Factory(conf);

    NfServer * svr = f_svr->create_svr();
     
    svr->set_work_callback(new RaReadLine()); 
    svr->set_handle(default_handle);
     
    if (svr->run() < 0)
        std::cout << strerror(errno) << std::endl; 
    
    int time = sleep(65535);
    
    svr->stop();
     
    svr->destroy();
    
    free(svr);

    //std::cout << "run time : " << 65535 - time << std::endl;    
}
