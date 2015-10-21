#include "net_svr_cb.h"


void IO_readcb(int fd,
               short events,
               void * arg)
{
    
    Event * ev = (Event * )arg;
    IOReactor * rect = (IOReactor *)ev->get_reactor();
    pthread_mutex_t * mu = &(rect->event_mutex);  
    
    int flags;

    const int size = 999;
    char buff[size];
    int n = read(fd, buff, size);
    buff[n] = '\0';
    std :: cout << buff << std::endl;
    rect->set_event_unactive(ev); 
}
