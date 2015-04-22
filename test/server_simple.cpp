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

int nf_default_read(void *data)
{
    nf_server_pdata_t *pdata = (nf_server_pdata_t *)data;
    int ret;
    if ( (ret = readn(pdata->fd, pdata->read_buf, 5)) <= 0)
        return -1;
    std::cout << pdata->read_buf << " : read value" <<std::endl;
    return ret;
}
int nf_default_write(void *data)
{
    nf_server_pdata_t *pdata = (nf_server_pdata_t *)data;
    pdata->write_buf = (char *)pdata->write_buf;
    strncpy((char *)pdata->write_buf, (char *)pdata->read_buf, 5);
    char * temp = (char *)pdata->write_buf;
    temp[5] = '\0';
    int ret;
    if ( (ret = sendn(pdata->fd, pdata->write_buf, 5, 20)) <= 0 )
        return -1;
    return ret;
}


int main(int argc, char *argv[])
{
    nf::NfServer *test =  new nf::NfServer();
    char s[] = "test server";
    test->set_server_name(s);
    test->load_conf("server.conf");
   
    test->get_server_data()->p_read = nf_default_read;
    test->get_server_data()->p_write = nf_default_write;
     
    if (test->run() < 0)
        std::cout << strerror(errno) << std::endl; 
    
    sleep(120);
    
    test->stop();
    
    test->destroy();
    
    free(test);
}
