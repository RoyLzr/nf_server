#include "../event.h"
#include "../reactor.h"
#include <pthread.h>
#include "../commonn/memCache.h"
#include "../nf_base_work.h"


void test_fun(int fd, short events, void * arg)
{
    WriteEvent * w_ev = (WriteEvent *) arg;
    char * tmp = "12345";
    w_ev->add_buffer(tmp, 6); 
}


const int nevents = 10;

int main()
{
    string s = "./svr.log";
    Log :: init(s.c_str());
    Log :: set_level(LOG_DEBUG);
    Allocate :: init();

    Reactor testRa;
    testRa.init(1000);
    ReadEvent * r_ev = new ReadEvent(50);
    WriteEvent * w_ev = new WriteEvent(50);

    r_ev->init(1, test_fun, parseLine);
    w_ev->init(1, NULL, sendData);
    //ev->init(0, EV_READ, test_fun);
    struct timeval *tv = (struct timeval *)calloc(1, sizeof(timeval));
    testRa.add_event(r_ev);
    testRa.add_event(w_ev);
    
    testRa.start(EV_ONCE);


    /*
    for(int i = 0; i < nevents; i++)
    {
        Event * ev = new Event();
        ev->init(i,1,test_fun);
        testRa.add_event(ev);
        std::cout << "test ra" << (ev->get_reactor())->get_ev_count()<< std::endl;
        (*(ev->get_ev_pos()))->excute();
    }
     
    list<Event *> testList = testRa.get_list();
    list<Event *> :: iterator iter = testList.begin();
    while(iter != testList.end())
    {
        (*iter)->excute();
        iter++;
    } 
    */   

    std::cout << "hello" << std::endl;
}
