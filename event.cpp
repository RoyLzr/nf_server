#include "event.h"
#include "reactor.h"
#include "nf_base_work.h"

void ReadEvent :: rd_init(int fd,
                          ev_handle handle,
                          ParseFun * parse)
{
    Event :: init(fd, EV_READ, handle);
    if(parse == NULL)
    {
        ev_parse = NULL;
        return;
    }

    ev_parse = dynamic_cast<NonBlockFun *>(parse);
    assert(ev_parse != NULL);

}

void WriteEvent :: wt_init(int fd,
                           ev_handle handle,
                           ParseFun * parse)
{
    Event :: init(fd, EV_WRITE, handle);
    
    if(parse == NULL)
    {
        ev_parse = NULL;
        return;
    }

    ev_parse = dynamic_cast<NonBlockFun *>(parse);
    assert(ev_parse != NULL);

}

void EventTask :: run()
{
    if(ev_task == NULL)
    {
        Log :: WARN("This is task is not inited %d", idx);
    }

#ifdef WORK
    CWorkerThread * th;
#else
    CWorkerThread * th = get_pthread_data();
    Log :: DEBUG("[Start] Task ID : %d, handle Thread %d excute", \
                  idx, th->GetThreadIdx());
#endif

    ev_task->excute();

#ifdef WORK

#else
    Log :: DEBUG("[End] Task ID : %d, handle Thread %d excute", \
                 idx, th->GetThreadIdx());
#endif
}

void Event :: excute()
{
    int ret = excute_fun();

    if(ret < 0)
    {
        Log :: NOTICE("one event will be closed");
        return;
    }

    ev_reactor->set_event_unactive(this);

    return;
}

int Event :: excute_fun()
{
    ev_callback(ev_fd, ev_events, this);
    return 0;
}

int ReadEvent :: excute_fun()
{
    //this event is already active
    int epfd = ev_reactor->get_epfd();
    int res;
    struct evepoll * events = NULL;
    events = ev_reactor->get_fds();

    if((res = net_ep_del(epfd, ev_fd)) < 0)
    {
        Log :: WARN("DEL [Epoll] Read Event error Reactor : \
                    %d, fd %d error : %d", epfd, ev_fd, res);
        return -1;
    } 
    Log :: DEBUG("DEL [Epoll] Read event %d SUCC", ev_fd);
   
    if(ev_parse != NULL)
    {
        del_ev_flags(EV_READUNFIN);
        if((res = ev_parse->work(ev_fd, this)) < 0)
        {
            Log :: WARN("Parse read event excute error");
            return -1;
        }
    }

    //Read Unfinished, next loop read event
    if(ev_flags & EV_READUNFIN)
    {
        Log :: DEBUG("Read Unfinish, Add [Epoll] Read %d", ev_fd);
        if((res = net_ep_add(epfd, ev_fd, EPOLLIN, &events[ev_fd], EPOLL_CTL_ADD)) < 0)
        {
            Log :: WARN("ADD [Epoll] Read Event Error %d, error %d", ev_fd, errno);
            return -1;
        }
        Log :: DEBUG("ADD [Epoll] Read event %d SUCC", ev_fd);
        goto done;
    }
    
    //Read finished, next loop write event
    if((res = net_ep_add(epfd, ev_fd, EPOLLOUT, &events[ev_fd], EPOLL_CTL_ADD)) < 0)
    {
        Log :: WARN("ADD [Epoll] Write Event Error %d, error %d", ev_fd, errno);
        return -1;
    }
    Log :: DEBUG("ADD [epoll] Write event %d SUCC", ev_fd);

done:

    return 0;
} 


int WriteEvent :: excute_fun()
{
    //this event is already active
    int epfd = ev_reactor->get_epfd();
    int res;
    struct evepoll * events = NULL;
    events = ev_reactor->get_fds();
    

    if((res = net_ep_del(epfd, ev_fd)) < 0)
    {
        Log :: WARN("DEL [Epoll] Write Event error Reactor : \
                    %d, fd %d error : %d", epfd, ev_fd, res);
        return -1;
    }
    Log :: DEBUG("DEL [Epoll] Write event %d SUCC", ev_fd);
    
    if(ev_parse != NULL)
    {   
        del_ev_flags(EV_READUNFIN);
        if ((res = ev_parse->work(ev_fd, this)) < 0)
        {
            Log :: WARN("Parse write event excute error");
            return -1;
        }
    }

    //Write Unfinished, next loop read event
    if(ev_flags & EV_WRITEUNFIN)
    {
        Log :: DEBUG("Write Unfinish, Add [Epoll] Write %d", ev_fd);
        if((res = net_ep_add(epfd, ev_fd, EPOLLOUT, &events[ev_fd], EPOLL_CTL_ADD)) < 0)
        {
            Log :: WARN("ADD [Epoll] Write Event Error %d, error %d", ev_fd, errno);
            return -1;
        }
        Log :: DEBUG("ADD [Epoll] Write event %d SUCC", ev_fd);
        goto done;
    }
    
    //Write finished, next loop read event
    if((res = net_ep_add(epfd, ev_fd, EPOLLIN, &events[ev_fd], EPOLL_CTL_ADD)) < 0)
    {
        Log :: WARN("ADD [Epoll] Read Event Error %d, error %d", ev_fd, errno);
        return -1;
    }
    Log :: DEBUG("ADD [epoll] Read event %d SUCC", ev_fd);

done:

    return 0;
}

