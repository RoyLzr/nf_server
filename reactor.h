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

struct threadParas
{
    int num;
    int buff_size;
};

class Reactor : private Uncopyable
{

    public:
        Reactor() : event_count(0),
                    event_count_ac(0),   
                    run(false),
                    nfile(0),
                    nevents(0),
                    events(NULL),
                    status(0),
                    fds(NULL)
        {
            pthread_mutex_init(&event_mutex, NULL);
            pthread_mutex_init(&list_mutex, NULL);
            poolPars.num = 0;
            poolPars.buff_size = 0;
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
    
    int del_event(Event * ev);
    
    int pause_event(Event * ev) const;
    
    int resume_event(Event * ev) const;

    int init(int, struct threadParas);

    int start();
    
    //fail event is active already 
    inline bool set_event_active(Event * ev)
    {
        if(ev == NULL)
            return false;
    
        if(ev->ev_flags & EV_ACTIVE)
            return false;
    
        ev->ev_flags |= EV_ACTIVE;
        return true;

    }
        
    //fail event is unactive already
    inline bool set_event_unactive(Event * ev)
    {
        if(ev == NULL)
            return false;

        if(!(ev->ev_flags & EV_ACTIVE))
            return false;
    
        ev->ev_flags &= ~EV_ACTIVE;
        return true;
    }

    bool set_io_event_active(Event *);
    
    bool set_io_event_unactive(Event *);

    pthread_mutex_t list_mutex;
    pthread_mutex_t event_mutex; 

    protected:
        void event_queue_insert(Event *, int );
        void event_queue_remove(Event *, int );
        
        virtual int epoll_add_event(Event *);
        virtual int epoll_del_event(Event *);
        
        int epoll_active_event(Event *) const;
        int epoll_unactive_event(Event *) const;

        int epoll_dispatch(struct timeval *tv);
        
    protected:
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
        int status;
        struct evepoll * fds;
        struct threadParas poolPars;
        CThreadPool pool; 
};



#endif
