#ifndef _REACTOR_
#define _REACTOR_

#include "event.h"
#include "sys/epoll.h"
#include "pthread.h"
#include "net.h"
#include "commonn/asynLog.h"

struct evepoll
{
    Event * evread;
    Event * evwrite;
    int status;
};

class Reactor
{

    public:
        Reactor() : event_count(0),
                    event_count_ac(0),   
                    run(false),
                    nfile(0),
                    nevents(0),
                    events(NULL),
                    fds(NULL)
        {
            pthread_mutex_init(&event_mutex, NULL);
            pthread_mutex_init(&list_mutex, NULL);
        }

    inline int get_ev_count()
    {
        return event_count;
    }
    
    inline int get_epfd()
    {
        return epfd;
    }
    
    inline evepoll * get_fds()
    {
        return fds;
    }
    
    inline evepoll * get_fds(int idx)
    {
        return &fds[idx];
    }

    int add_event(Event * ev, 
                  struct timeval * tv = NULL,
                  bool actived = true);
    
    int init(int);
    int start(int);
    
    //fail event is active already 
    bool set_event_active(Event *);
        
    //fail event is unactive already
    bool set_event_unactive(Event *);

    pthread_mutex_t list_mutex;
    pthread_mutex_t event_mutex; 

    private:
        void event_queue_insert(Event *, int );
        int epoll_add_event(Event *, bool actived);
        int epoll_dispatch(int status, struct timeval *tv);
        int epoll_del_event(Event *, bool removed = true);
        
    private:
        list<Event *> ev_list;
        //TODO: add timer, queu
        
        int event_count;
        int event_count_ac;
        
        bool run;
        //EPOLL INFO
        int epfd;
        struct epoll_event *events;
        int nfile;
        int nevents;
        struct evepoll * fds; 
};



#endif
