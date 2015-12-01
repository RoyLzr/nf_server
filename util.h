#ifndef _UTIL_
#define _UTIL_

//#define WORK 0x00


//ev_events
#define EV_TIMEOUT  0x01
#define EV_READ     0x02
#define EV_WRITE    0x04
#define EV_LISTEN   0x08


//ev_flags
#define EV_INIT         0x01
#define EV_INSERTED     0x02
#define EV_ACTIVE       0x04   //excute the event
#define EV_READUNFIN    0x08
#define EV_WRITEUNFIN   0x10
#define EV_EPOLL_ACTIVE 0x20   //register to epoll

//reactor
#define RA_THREAD     0x01
#define RA_INIT       0x02
#define RA_ONCE       0x04


#define FD_NOSLEEP 0x00
#define FD_ACTIVE 0x01

#define EPOLL_ACTIVED true
#define EPOLL_UNACTIVED false
/*
struct timeval
{
    long tv_sec;
    long tv_usec;
}
*/

class Uncopyable
{
    protected:
        Uncopyable() {}
        ~Uncopyable() {}

    private:
        Uncopyable(const Uncopyable &);
        Uncopyable & operator=(const Uncopyable &);
};

#endif
