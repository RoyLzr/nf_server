#ifndef _UTIL_
#define _UTIL_


#define EV_TIMEOUT 0x01
#define EV_READ 0x02
#define EV_WRITE 0x04
#define EV_PERSIST 0x08

#define EV_INIT 0x01
#define EV_INSERTED 0x02
#define EV_ACTIVE 0x04
#define EV_READUNFIN 0x08
#define EV_WRITEUNFIN 0x10


#define EV_THREAD 0x01
#define EV_ONCE 0x02


#define FD_NOSLEEP 0x00
#define FD_ACTIVE 0x01
/*
struct timeval
{
    long tv_sec;
    long tv_usec;
}
*/


#endif
