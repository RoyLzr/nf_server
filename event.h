#ifndef _EVENT_
#define _EVENT_

#include <list>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "util.h"

using std::list;

class Reactor;
typedef void (*ev_handle)(int, short, void *);

class Event
{
    public:
    friend class Reactor;
    friend class IOEvent;
    explicit Event() : ev_fd(0),
                       ev_flags(0),
                       ev_events(0),
                       ev_active(0),
                       ev_reactor(NULL)
    {}
    inline void init(int fd, 
                     int events, 
                     ev_handle handle)
    {
        ev_fd = fd;
        ev_events = events;
        ev_active = 0;
        ev_flags = EV_INIT;
        ev_callback = handle;
        ev_wrap = NULL;
    }
    inline int get_ev_fd()
    {
       return ev_fd;
    }
    inline int get_ev_flags()
    {
        return ev_flags;
    }
    inline int get_ev_active()
    {
        return ev_active;
    }
    inline void set_ev_flags(int flg)
    {
        ev_flags = flg;
    }
    inline int get_ev_events()
    {
        return ev_events;
    }
    inline Reactor * get_reactor()
    {
        return ev_reactor;
    }
    inline list<Event *>::iterator get_ev_pos()
    {
        return ev_pos;
    }
    void excute(void * arg = NULL)
    {
        if(arg == NULL)
            ev_callback(ev_fd, ev_events, this);
        else
            ev_callback(ev_fd, ev_events, arg);
        return;
    }

    private:
        Event & operator=(Event & ev)
        {
            assert(false);
            return *this;
        }
        list<Event *>::iterator ev_pos;
        int ev_fd;
        
        int ev_flags;
        short ev_events;
        int ev_active;
        void * ev_wrap;

        Reactor * ev_reactor;
        ev_handle ev_callback;
        void *ev_arg;
        
};


class IOEvent
{

    public:
        friend class Reactor;
        friend class IOReactor;
        explicit IOEvent() : cache(NULL),
                             c_len(0)
        {}
    inline void init(int fd,
                     int events,
                     ev_handle handle)
    {
        ev.init(fd, events, handle);
        ev.ev_wrap = this; 
    }
    inline Event * get_base_event()
    {
        return &ev;
    }
    inline void * get_cache()
    {
        return cache;
    }
    inline int get_cache_len()
    {
        return c_len;
    }
    inline void set_cache_len(int len)
    {
        c_len = len;
    }
    
    private:
        Event ev;
        void * cache;
        int c_len;
};


#endif
