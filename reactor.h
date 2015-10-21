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

    //TODO : multi_thread handle
    //static pthread_mutex_t ev_list_mutex;
    int add_event(Event * ev, 
                  struct timeval * tv = NULL);
    
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
        int epoll_add_event(Event *, bool added = true);
        int epoll_dispatch(int, struct timeval *tv);
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

class IOReactor : public Reactor 
{
    public:
    IOReactor() : Reactor()
    {}
    
    int add_event(IOEvent * ioev, 
                  struct timeval * tv = NULL)
    {
        Reactor :: add_event(&(ioev->ev), tv);
    }
    
    int init(int size)
    {
        Reactor :: init(size);
    }
    int start(int status)
    {
        Reactor :: start(status);
    }
    
};


#endif
