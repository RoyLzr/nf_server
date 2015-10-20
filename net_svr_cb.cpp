#include "net_svr_cb.h"


void IO_readcb(int fd,
               short events,
               void * arg)
{
    
    Event * ev = (Event * )arg;
    IOReactor * rect = (IOReactor *)ev->get_reactor();
    pthread_mutex_t * mu = &(rect->event_mutex);  
    
    int flags;

    pthread_mutex_lock(mu);
    flags = ev->get_ev_flags() & ~EV_ACTIVE;      
    ev->set_ev_flags(flags);
    pthread_mutex_unlock(mu);
    
    const int size = 999;
    char buff[size];
    int n = read(fd, buff, size);
    buff[n] = '\0';
    std::cout << "out: "<<n<< " : " <<buff << std::endl;    
}
