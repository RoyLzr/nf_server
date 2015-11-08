#include "../event.h"
#include "../reactor.h"
#include <pthread.h>
#include "../commonn/memCache.h"
#include "../nf_base_work.h"
#include "../nf_server.h"
#include "../sapool.h"


void test_read_fun(int fd, short events, void * arg)
{
    struct evepoll * eve = (struct evepoll *) arg;
    WriteEvent * w_ev = (WriteEvent *) eve->evwrite;
    ReadEvent * r_ev = (ReadEvent *) eve->evread;
    Buffer & r_buff = r_ev->get_buffer();

#ifndef WORK
    printf("handle fun \n");
#endif
    void * src = r_buff.get_unhandle_cache();
    w_ev->add_buffer(src, r_buff.get_unhandle_num());
    
    r_buff.add_handl_num(r_buff.get_unhandle_num());
}

void test_write_fun(int fd, short events, void * arg)
{
    printf("write handle fun\n");
}

const int nevents = 10;

int main()
{
    string s = "./svr.conf";
    SaServer svr;
    
    svr.set_read_handle(test_read_fun);
    svr.set_write_handle(test_write_fun);
    svr.set_parse_read_handle(parseLine());
    svr.set_parse_write_handle(writeData());

    svr.init("./svr.conf");
    svr.run();

    return 0;
}
