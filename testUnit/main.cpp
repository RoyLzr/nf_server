#include "../event.h"
#include "../reactor.h"
#include "../net_svr_cb.h"
#include <pthread.h>


void test_fun(int fd, short events, void * arg)
{
    //std::cout << "fd : " << fd << std::endl;
    const int size = 9999;
    char buff[size];
    int n = read(fd, buff, size);
    std::cout << "out: "<< buff << std::endl;    

}


const int nevents = 10;

int main()
{
    string s = "./svr.log";
    Log :: init(s.c_str());
    Log :: set_level(LOG_DEBUG);

    IOReactor testRa;
    testRa.init(1000);
    IOEvent * ev = new IOEvent();
    ev->init(1, EV_READ, IO_readcb);
    //ev->init(0, EV_READ, test_fun);
    struct timeval *tv = (struct timeval *)calloc(1, sizeof(timeval));
    testRa.add_event(ev);
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
