#ifndef _EVENT_
#define _EVENT_

#include <list>
#include "sys/epoll.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "commonn/asynLog.h"
#include "util.h"

using std::list;

class Reactor;
class ParseBase;
typedef void (*ev_handle)(int, short, void *);
struct Buffer
{
    void * cache;
    int allo_len;
    int used_len;
};



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
                       ev_parse(NULL)
    {}
    inline void init(int fd, 
                     int events, 
                     ev_handle handle,
                     ParseBase * parse = NULL)
    {
        ev_fd = fd;
        ev_events = events;
        ev_active = 0;
        ev_flags = EV_INIT;
        ev_callback = handle;
        ev_parse = parse;
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

    virtual void excute(void * arg = NULL);

    private:
        Event & operator=(Event & ev)
        {
            assert(false);
            return *this;
        }

    protected:
        list<Event *>::iterator ev_pos;
        int ev_fd;
        
        int ev_flags;
        short ev_events;
        int ev_active;

        Reactor * ev_reactor;
        ev_handle ev_callback;
        void *ev_arg;
        ParseBase * ev_parse;
        
};


class ReadEvent : public Event
{

    public:
        friend class Reactor;
        explicit ReadEvent() : cache(NULL)
        {}
        explicit ReadEvent(void * ca, int len) : cache(ca)
        {}
        
    inline void init(int fd,
                     int events,
                     ev_handle handle)
    {
        Event :: init(fd, events, handle);
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

    virtual void excute(void * arg = NULL);
    
    protected:
        Buffer * cache;
};

class WriteEvent : public Event
{

    public:
        friend class Reactor;
        explicit WriteEvent() : cache(NULL)
        {}
        explicit WriteEvent(void * ca, int len) : cache(ca)
        {}
        
    inline void init(int fd,
                     int events,
                     ev_handle handle)
    {
        Event :: init(fd, events, handle);
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

    virtual void excute(void * arg = NULL);
    
    protected:
        Buffer * cache;
};

#endif
