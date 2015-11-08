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
#include "Buffer.h"
#include "commonn/ThreadPool.h"

using std::list;

class Reactor;
class ParseFun;
class NonBlockFun;

typedef void (*ev_handle)(int, short, void *);
typedef int (*parse_handle)(int, void *);


class Event
{
    public:
    friend class Reactor;
    friend class IOEvent;
    explicit Event() : ev_fd(0),
                       ev_flags(0),
                       ev_events(0),
                       ev_active(0),
                       ev_reactor(NULL),
                       ev_callback(NULL)
    {}

    virtual ~Event(){};

    inline void init(int fd, 
                     int events, 
                     ev_handle handle = NULL)
    {
        ev_fd = fd;
        ev_events = events;
        ev_active = 0;
        ev_flags = EV_INIT;
        ev_callback = handle;
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
    
    inline void add_ev_flags(int flg)
    {
        ev_flags |= flg;
    }
    
    inline void del_ev_flags(int flg)
    {
        ev_flags &= ~flg;
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

    inline ev_handle get_ev_handle()
    {
        return ev_callback;
    }

    void excute();
    
    static void * ThreadExcute(Event * arg)
    {
        assert(arg != NULL);

        arg->excute();
    }

    private:
        Event & operator=(Event & ev)
        {
            assert(false);
            return *this;
        }

        Event(Event & ev)
        {
            assert(false);
        }

    protected:
        virtual int excute_fun();

        list<Event *>::iterator ev_pos;
        int ev_fd;
        
        int ev_flags;
        short ev_events;
        int ev_active;

        Reactor * ev_reactor;
        ev_handle ev_callback;
        void *ev_arg;
        
};


class ReadEvent : public Event
{

    public:
        friend class Reactor;
        explicit ReadEvent() : Event(),
                               ev_parse(NULL)
        {}
        explicit ReadEvent(int len) : Event(),
                                      ev_parse(NULL)
        {
            cache.init(len);
        }
        
    void rd_init(int fd,
                 ev_handle handle = NULL,
                 ParseFun * parse = NULL);

    inline Buffer & get_buffer()
    {
        return cache;
    }
    
    inline int add_buffer(void * tmp,
                          int len)
    {
        return cache.add_data(tmp, len);
    }
    
    inline int get_buf_unhandle_num()
    {
        return cache.get_unhandle_num();
    }
    inline void * get_buf_unhandle_cache()
    {
        return cache.get_unhandle_cache();
    }

    
    protected:
        virtual int excute_fun();
        Buffer cache;
        NonBlockFun * ev_parse;
        
};

class WriteEvent : public Event
{

    public:
        friend class Reactor;
        explicit WriteEvent() : Event(),
                                ev_parse(NULL)
        {}
        explicit WriteEvent(int len) : Event(),
                                       ev_parse(NULL)
        {
            cache.init(len);
        }
        
    void wt_init(int fd,
                 ev_handle handle = NULL,
                 ParseFun * parse = NULL);

    inline Buffer & get_buffer()
    {
        return cache;
    }
    
    inline int add_buffer(void * tmp,
                          int len)
    {
        return cache.add_data(tmp, len);
    }
    inline int get_buf_unhandle_num()
    {
        return cache.get_unhandle_num();
    }
    inline void * get_buf_unhandle_cache()
    {
        return cache.get_unhandle_cache();
    }
    inline NonBlockFun * get_parse_handle()
    {
        return ev_parse;
    }

    protected:
        virtual int excute_fun();
        Buffer  cache;
        NonBlockFun * ev_parse;
};

class EventTask : public CTask
{
    public:
        EventTask() : ev_task(NULL)
        {}
        EventTask(Event * ev) : ev_task(ev)
        {
            //idx = ev->get_ev_fd();
        }
        virtual void run();
    private:
        Event * ev_task; 
};                  

#endif
