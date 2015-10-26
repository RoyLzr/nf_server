#include "net_svr_cb.h"


void IO_readcb(int fd,
               short events,
               void * arg)
{
    
    ReadEvent * ev = (ReadEvent * )arg;
    int flags;
    const int size = 999;
    char buff[size];
    int n = read(fd, buff, size);
    buff[n] = '\0';
    std::cout << buff << std::endl;
}
