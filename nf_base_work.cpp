#include "nf_base_work.h"


int parseLine(int fd, void * arg, void * tmp)
{
    ReadEvent * ev = (ReadEvent *) arg; 
    Reactor * rect = ev->get_reactor();
    struct evepoll * eve = rect->get_fds(fd);
    WriteEvent * w_ev = (WriteEvent *) eve->evwrite;
    char * buff = (char *) tmp;
    int flags;
    const int size = 999;
    int n = read(fd, buff, size);
    buff[n] = '\0';
    std :: cout << (char *)buff << std::endl;
    ev_handle call_back = ev->get_ev_handle();

    if(call_back != NULL)
        call_back(fd, EV_READ, w_ev);
}

int sendData(int fd, void * arg, void * tmp)
{
    WriteEvent * w_ev = (WriteEvent *) arg;
    int num = w_ev->get_buf_handle_num();

    int n = write(fd, w_ev->get_buf_handle_cache(), num);
}

