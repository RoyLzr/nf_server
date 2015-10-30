#include "../event.h"
#include "../reactor.h"
#include <pthread.h>
#include "../commonn/memCache.h"
#include "../nf_base_work.h"


void test_fun(int fd, short events, void * arg)
{
    struct evepoll * eve = (struct evepoll *) arg;
    WriteEvent * w_ev = (WriteEvent *) eve->evwrite;
    ReadEvent * r_ev = (ReadEvent *) eve->evread;
    Buffer & r_buff = r_ev->get_buffer();

#ifndef WORK
    printf("user handle fun\n");
#endif

    void * src = r_buff.get_unhandle_cache();
    w_ev->add_buffer(src, r_buff.get_unhandle_num());
    
    r_buff.add_handl_num(r_buff.get_unhandle_num());
}


const int nevents = 10;
CThreadPool pool;

int main()
{
    pool.init(4, 1024);

    string s = "./svr.log";
    Log :: init(s.c_str());
    Log :: set_level(LOG_DEBUG);
    Allocate :: init();

    Reactor testRa;
    testRa.init(1000);
    ReadEvent * r_ev = new ReadEvent();
    WriteEvent * w_ev = new WriteEvent();

    r_ev->init(1, test_fun, parseLine);
    w_ev->init(1, NULL, sendData);
    //ev->init(0, EV_READ, test_fun);
    struct timeval *tv = (struct timeval *)calloc(1, sizeof(timeval));
    testRa.add_event(r_ev);
    testRa.add_event(w_ev);
    
    testRa.start(EV_THREAD);

    std::cout << "hello" << std::endl;
}
