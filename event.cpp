#include "event.h"
#include "reactor.h"

extern CThreadPool pool;

void Event :: excute()
{
    ev_callback(ev_fd, ev_events, this);
   
    ev_reactor->set_event_unactive(this);

    return;
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


void ReadEvent :: excute()
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
        goto done;
    } 
    Log :: DEBUG("DEL [Epoll] Read event %d SUCC", ev_fd);
    
    if (events[ev_fd].evwrite == NULL)
    {
        Log :: DEBUG("First add write event %d Succ", ev_fd);
    }    
   
    if(ev_parse != NULL)
        ev_parse(ev_fd, this);

    //Read Unfinished, next loop read event
    if(ev_flags & EV_READUNFIN)
    {
        Log :: DEBUG("Read Unfinish, Add [Epoll] Read %d", ev_fd);
        if((res = net_ep_add(epfd, ev_fd, EPOLLIN, &events[ev_fd], EPOLL_CTL_ADD)) < 0)
        {
            Log :: WARN("ADD [Epoll] Read Event Error %d, error %d", ev_fd, errno);
            goto done;
        }
        Log :: DEBUG("ADD [Epoll] Read event %d SUCC", ev_fd);
        goto done;
    }
    
    //Read finished, next loop write event
    if((res = net_ep_add(epfd, ev_fd, EPOLLOUT, &events[ev_fd], EPOLL_CTL_ADD)) < 0)
    {
        Log :: WARN("ADD [Epoll] Write Event Error %d, error %d", ev_fd, errno);
        goto done;
    }
    Log :: DEBUG("ADD [epoll] Write event %d SUCC", ev_fd);

    done:
        ev_reactor->set_event_unactive(this);

    return;
} 


void WriteEvent :: excute()
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
        goto done;
    }
    Log :: DEBUG("DEL [Epoll] Write event %d SUCC", ev_fd);
    
    if(ev_parse != NULL)
        ev_parse(ev_fd, this);

    //Write Unfinished, next loop read event
    if(ev_flags & EV_WRITEUNFIN)
    {
        Log :: DEBUG("Write Unfinish, Add [Epoll] Write %d", ev_fd);
        if((res = net_ep_add(epfd, ev_fd, EPOLLOUT, &events[ev_fd], EPOLL_CTL_ADD)) < 0)
        {
            Log :: WARN("ADD [Epoll] Write Event Error %d, error %d", ev_fd, errno);
            goto done;
        }
        Log :: DEBUG("ADD [Epoll] Write event %d SUCC", ev_fd);
        goto done;
    }
    
    //Write finished, next loop read event
    if((res = net_ep_add(epfd, ev_fd, EPOLLIN, &events[ev_fd], EPOLL_CTL_ADD)) < 0)
    {
        Log :: WARN("ADD [Epoll] Read Event Error %d, error %d", ev_fd, errno);
        goto done;
    }
    Log :: DEBUG("ADD [epoll] Read event %d SUCC", ev_fd);

    done:
        ev_reactor->set_event_unactive(this);

    return;
} 
