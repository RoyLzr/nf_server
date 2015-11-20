#include "reactor.h"

//**********************************************************
//
//  Description:
//  
//  add event:
//  read ¶ active/unfinish, add adre event fail, write ok
//  write active/unfinish , add write event fail, read ok
//
//  excute event:
//  excute read active, this read event can not excute again
//  unfinish ok
//
//  excute write active, this write event can not excute again
//  unfinish ok
//
//  IO-event:
//  realize fun excute() by del epoll event.
//  excute read, read/write excute fail
//  excute write, read/write excute fail
//  
//  del event:
//  1.work thread excute event error, return to event->excute() del event
//    warning : work thread happen, reactor epoll has deleted event fd 
//    work thread excute event, event is actived, don't need detect
//  
//  2.reactor epoll loop detect error.
//     1. if event is active, pass. work thread will handle
//     2. if event is unactive, del event
//     3. if event is unFinish, del event. No data will come.
//
//  other event:
//  realize its own virsual fun excute_fun()
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

bool Reactor :: set_event_active(Event * ev)
{
    pthread_mutex_lock(&event_mutex);
    if(ev->ev_flags & EV_ACTIVE)
    {
        pthread_mutex_unlock(&event_mutex);
        return false;
    }
    ev->ev_flags |= EV_ACTIVE;
    pthread_mutex_unlock(&event_mutex);
    return true;
}

bool Reactor :: set_event_unactive(Event * ev)
{
    pthread_mutex_lock(&event_mutex);
    if(!(ev->ev_flags & EV_ACTIVE))
    {
        pthread_mutex_unlock(&event_mutex);
        return false;
    }
    ev->ev_flags &= ~EV_ACTIVE;
    pthread_mutex_unlock(&event_mutex);
    return true;
}

void Reactor :: event_queue_insert(Event *ev,
                                  int status)
{
    ev->ev_flags |= status;
    switch(status)
    {
        case EV_INSERTED:
        {
            pthread_mutex_lock(&list_mutex);
            ev_list.push_back(ev);
            ev->ev_pos = --ev_list.end();
            pthread_mutex_unlock(&list_mutex);
        
            ev->ev_reactor = this;
            break;
        }
        default:
            printf("this status %d not support\n", status);    
    }
}

void Reactor :: event_queue_remove(Event *ev,
                                  int status)
{
    if(!(ev->ev_flags & status))
    {
        Log :: WARN("REMOVE event from queue : Status Error");
        return;
    }

    ev->ev_flags &= ~status;
    switch(status)
    {
        case EV_INSERTED:
        {

         #ifndef WORK
            Log :: DEBUG("DEL event frome evnent queue fd : %d", (*(ev->ev_pos))->ev_fd);
         #endif

            pthread_mutex_lock(&list_mutex);
            ev_list.erase(ev->ev_pos);
            pthread_mutex_unlock(&list_mutex);
            
            ev->ev_reactor = NULL;
            break;
        }
        default:
            printf("this status %d not support\n", status);    
    }
}


int Reactor :: add_event(Event * ev,
                         struct timeval * tv,
                         bool actived)
{
    int ret = 0;
    if(ev == NULL)
       return 0;

    Log :: DEBUG("ADD EVENT FD: %d events: %s%s%s reactor: %d",
                 ev->ev_fd,
                 ev->ev_events&EV_READ ? "READ":"",
                 ev->ev_events&EV_WRITE ?"WRITE":"",
                 ev->ev_events&EV_TIMEOUT?"TIME":"",
                 epfd);
      
    assert((ev->ev_flags & EV_INIT));
    
    if(status & RA_THREAD)
    {
        if(!set_event_active(ev))
        {
            Log :: WARN("Add fail, Event is ACtive, FD %d", ev->ev_fd);
            return -1;
        } 
    } 

    if(tv != NULL)
    {
        assert(ev->ev_events & EV_TIMEOUT);
        printf("add timeout event\n");
    }
     
    //if pass, no other can handle this event 
    if((ev->ev_events & (EV_READ|EV_WRITE)) &&
       !(ev->ev_flags & (EV_INSERTED)))
    {
       event_queue_insert(ev, EV_INSERTED);
       Log :: NOTICE("EVENT fd:%d added IN LIST", ev->ev_fd); 
       
       if((ret = epoll_add_event(ev)) < 0)
           goto done;
        
       if(actived)
       {
           if((ret = epoll_active_event(ev)) < 0)
               goto done;
       }

       Log :: NOTICE("Reactor Add Event succ fd %d", ev->ev_fd);
       
       if((status & RA_THREAD) & (!set_event_unactive(ev)))
       {
           Log :: WARN("Add SUcc Set fail, set Event UnaCtive Error,\
                   status broken FD %d", ev->ev_fd);
       }

       return 0;
    }

    Log :: WARN("ADD EVENT fd : %d fail, ALREAD ADDED", ev->ev_fd);
done:
    if((status & RA_THREAD) & (!set_event_unactive(ev)))
    {
        Log :: WARN("Add fail, set Event UnaCtive Error,\
                status broken FD %d", ev->ev_fd);
    }

    return -1;
}  

int Reactor :: epoll_active_event(Event * ev) const
{
    struct evepoll * evep = NULL;
    int fd = 0, events = 0, op = 0;
    Event * evread = NULL, *evwrite = NULL;
     
    //each fd match read/write event
    //fds store read/write event
    fd = ev->ev_fd;
    if(fd > nfile)
    {
        Log :: ERROR("FD : %d OUT OF LINE", fd);
        return -1;
    }

    if(ev->ev_flags & EV_EPOLL_ACTIVE)
        return 0;

    evep = &fds[fd];
    evread = evep->evread;
    evwrite = evep->evwrite;
    op = EPOLL_CTL_ADD;

    //set add event 
    if(evread != NULL && (evread->ev_flags & EV_EPOLL_ACTIVE))
    {
        events |= EPOLLIN;
        op = EPOLL_CTL_MOD;        
    }
    if(evwrite != NULL && (evwrite->ev_flags & EV_EPOLL_ACTIVE))
    {
        events |= EPOLLOUT;
        op = EPOLL_CTL_MOD;
    }
    if(ev->ev_events & EV_READ)
        events |= EPOLLIN;
    if(ev->ev_events & EV_WRITE)
        events |= EPOLLOUT;
    
    if(net_ep_add(epfd, fd, events, evep, op) < 0)
    {
        Log :: WARN("EPOLL : %d ADD FD : %d ERROR", epfd, fd);
        return -1;
    }
    ev->ev_flags |= EV_EPOLL_ACTIVE;

    return 0;
}


int Reactor :: epoll_unactive_event(Event * ev) const
{
    struct evepoll * evep = NULL;
    int fd = 0, events = 0, op = 0;
    Event * evread = NULL, *evwrite = NULL;
     
    //each fd match read/write event
    //fds store read/write event
    fd = ev->ev_fd;
    if(fd > nfile)
    {
        Log :: ERROR("FD : %d OUT OF LINE", fd);
        return -1;
    }

    if(!(ev->ev_flags & EV_EPOLL_ACTIVE))
    {
        return 0;
    }

    evep = &fds[fd];
    evread = evep->evread;
    evwrite = evep->evwrite;
    op = EPOLL_CTL_DEL;
    
    if(ev->ev_events & EV_READ)
        events |= EPOLLIN;
    if(ev->ev_events & EV_WRITE)
        events |= EPOLLOUT;
    
    if((events & EPOLLIN) && (evwrite != NULL) &&
        (evwrite->ev_flags & EV_EPOLL_ACTIVE))
    {
        events = EPOLLOUT;
        op = EPOLL_CTL_MOD;
    }
    else if((events & EPOLLOUT) && (evread != NULL) &&
            (evread->ev_flags & EV_EPOLL_ACTIVE))
    {
        events = EPOLLIN;
        op = EPOLL_CTL_MOD;
    }
   
    if(net_ep_add(epfd, fd, events, evep, op) < 0)
    {
        Log :: WARN("EPOLL : %d DEL FD : %d ERROR %s", epfd, fd, strerror(errno));
        return -1;
    }
    ev->ev_flags &= ~EV_EPOLL_ACTIVE;

    return 0;
}


int Reactor :: epoll_add_event(Event * ev)
{
    struct evepoll * evep = NULL;
    int fd, events, op;
    Event * evread = NULL, *evwrite = NULL;
     
    //each fd match read/write event
    //fds store read/write event
    fd = ev->ev_fd;
    if(fd > nfile)
    {
        Log :: ERROR("FD : %d OUT OF LINE", fd);
        return -1;
    }

    evep = &fds[fd];
    evread = evep->evread;
    evwrite = evep->evwrite;
    
    if(ev->ev_events & EV_READ)
    {
        if(evread != NULL)
            delete evread;
        evep->evread = ev;
    }

    if(ev->ev_events & EV_WRITE)
    {
        if(evwrite != NULL)
            delete evwrite;
        evep->evwrite = ev;
    }
    
    pthread_mutex_lock(&event_mutex);
    event_count++; 
    pthread_mutex_unlock(&event_mutex);

    return 0; 
}

int Reactor :: init(int files, 
                    struct threadParas paras)
{
    nevents = files;
    nfile = files;
    if((epfd = epoll_create(nfile)) == -1)
    {
        Log :: ERROR("REACTOR EPOLL CREATE ERROR"); 
        return -1;
    }

    //Start reactor's threadPool
    pool.init(paras.num, paras.buff_size);

    set_fd_block(epfd);   

    events = (struct epoll_event *) malloc(nevents * sizeof(struct epoll_event));
    if(events == NULL)
    {
        Log :: ERROR("REACTOR can not malloc %d epoll_events", nevents);
        return -1;
    }
    fds = (struct evepoll *)calloc(nfile, sizeof(struct evepoll));
    
    if(fds == NULL)
    {
        free(events);
        Log :: ERROR("REACTOR can not malloc %d evepoll", nfile);
        return -1;
    }
    Log :: NOTICE("REACTOR INIT SUCC epfd : %d \
                   files : %d ", epfd, nevents);
    
    status |= RA_INIT;
    if(paras.num > 0)
        status |= RA_THREAD;
    else
        status |= RA_ONCE;

    return 0;
}

int Reactor :: start()
{
    run = true;

    while(run)
    {
        pthread_mutex_lock(&event_mutex);
        if(event_count <= 0)
        {
            pthread_mutex_unlock(&event_mutex);
            Log :: NOTICE("THIS REACTOR has no events");
            sleep(1);
            continue;
        }
        pthread_mutex_unlock(&event_mutex);
        //TIMEOUT chuli 

        int res = epoll_dispatch(NULL);
        if(res < 0)
            return -1; 
    }
    return 0;
}


int Reactor :: epoll_dispatch(struct timeval * tv)
{
    int res = 0;
    struct evepoll * evep = NULL;    
    int status = this->status & RA_THREAD ? RA_THREAD : RA_ONCE;
    
    assert(this->status & RA_INIT);

    res = epoll_wait(epfd, events, nevents, 3000);

    if(res == -1)
    {
        if(errno != EINTR)
        {
            Log :: WARN("EPOLL DISPATCH LOOP ERROR epfd : %d", epfd);
            return -1;
        }
        return 0;
    }
    for(int i = 0; i < res; i++)
    {
        int active = events[i].events;
        Event * evread = NULL, * evwrite = NULL;
        
        evep = (struct evepoll *) events[i].data.ptr;
        evread = evep->evread;
        evwrite = evep->evwrite;
        int fd = evread->ev_fd;        

        if(active & EPOLLHUP)
        {
            del_event(evread);
            del_event(evwrite);
            continue;
        }

        else if(active & EPOLLERR)
        {
            del_event(evread);
            del_event(evwrite);
            continue;
        }
        else if(active & EPOLLIN )
        {
            if(evread->ev_events & EV_LISTEN)
            {
                evread->excute();
                continue;
            }
            switch(status)
            {
                case RA_ONCE:
                    evread->excute();
                    break;
                case RA_THREAD:
                    if(set_event_active(evread))
                    {
                        pool.AddTask(new EventTask(evread));
                    }    
                    break;
            }
            continue;
        }        

        else if(active & EPOLLOUT)
        {
            switch(status)
            {
                case RA_ONCE:
                    evwrite->excute();
                    break;
                case RA_THREAD:
                    if(set_event_active(evwrite))
                    {
                        //handle
                        pool.AddTask(new EventTask(evwrite));
                    }    
                    break;
            }
            continue;
        }        
        
    }
    return 0;
}

int Reactor :: del_event(Event * ev)
{
    if(ev == NULL)
        return 0;

    int ret = 0;
    int fd = ev->ev_fd;

    Log :: DEBUG("DEL EVENT FD: %d events: %s%s%s reactor: %d",
                 ev->ev_fd,
                 ev->ev_events&EV_READ ? "READ":"",
                 ev->ev_events&EV_WRITE ?"WRITE":"",
                 ev->ev_events&EV_TIMEOUT?"TIME":"",
                 epfd);
    assert(this == ev->ev_reactor);
    
    if(status & RA_THREAD)
    {
        if(!set_event_active(ev))
        {
            Log :: WARN("DEL fail, Event is ACtive, FD %d", ev->ev_fd);
            return -1;
        } 
    } 
    
    if(ev->ev_flags & EV_INSERTED)
    {
        event_queue_remove(ev, EV_INSERTED);
        if((ret = epoll_unactive_event(ev)) < 0)
        {
            goto done;
        }
        
        if((ret = epoll_del_event(ev)) < 0)
        {
            goto done;
        } 
        
        Log :: NOTICE("DEL IO EVENT SUCC FD : %d", fd);
        return 0;
    }

done:
    Log ::WARN("DEL EVENT FAIL FD : %d", fd);
    if(status & RA_THREAD)
    {
        if(!set_event_unactive(ev))
        {
            Log :: WARN("Del fail, set Event UnaCtive Error,\
                    status broken FD %d", ev->ev_fd);
        }
        return -1; 
    }
}

int Reactor :: epoll_del_event(Event * ev)
{
    struct evepoll * evep = NULL;
    int fd, events, op;
    Event * evread = NULL, *evwrite = NULL;
     
    //each fd match read/write event
    //fds store read/write event
    fd = ev->ev_fd;
    if(fd > nfile)
    {
        Log :: ERROR("FD : %d OUT OF LINE", fd);
        return -1;
    }

    evep = &fds[fd];
    evread = evep->evread;
    evwrite = evep->evwrite;
    
    if(ev->ev_events & EV_READ)
    {
        if(evread != NULL)
            delete ev;
        evep->evread = NULL;
        ev = NULL;
        goto done;
    }

    if(ev->ev_events & EV_WRITE)
    {
        if(evwrite != NULL)
            delete ev;
        evep->evwrite = NULL;
        ev = NULL;
    }

done:
    close(fd);
    
    pthread_mutex_lock(&event_mutex);
    event_count--; 
    pthread_mutex_unlock(&event_mutex);

    return 0; 
}

int Reactor :: pause_event(Event * ev) const
{
    if(ev->ev_flags & EV_EPOLL_ACTIVE)
        return epoll_unactive_event(ev);
#ifndef WORK
    printf("pause event but event is unactive\n");
#endif
    return 0;
}

int Reactor :: resume_event(Event * ev) const
{
    if(!(ev->ev_flags & EV_EPOLL_ACTIVE))
        return epoll_active_event(ev);
#ifndef WORK
    printf("resume event but event is active\n");
#endif
    return 0;
}


