#ifndef _REACTOR_
#define _REACTOR_

#include "event.h"
#include "sys/epoll.h"
#include "pthread.h"

struct evepoll
{
    Event * evread;
    Event * evwrite;
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
        {}
    inline int get_ev_count()
    {
        return event_count;
    }
    inline list<Event *> get_list()
    {
        return ev_list;
    }

    //TODO : multi_thread handle
    //static pthread_mutex_t ev_list_mutex;
    void add_event(Event * ev)
    {
        
        ev_list.push_back(ev);
        ev->ev_pos = --ev_list.end();
        
        ev->ev_reactor = this;
        event_count++;
    }

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
