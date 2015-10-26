#include "reactor.h"

//**********************************************************
//
//  Description:
//  
//  add event:
//  read ¶ active/unfinish, add re event fail, write ok
//  write active/unfinish , add write event fail, read ok
//
//  excute event:
//  excute read, read event excute fail
//  excute write, write event excute fail
//
//  IO-event:
//  realize fun excute() by del epoll event.
//  excute read, read/write excute fail
//  excute write, read/write excute fail
//
//  other event:
//  realize its own virsual fun excute()
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
            ev->ev_flags |= EV_INSERTED;
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
     
    Log :: DEBUG("ADD EVENT FD: %d events: %s%s%s reactor: %d",
                 ev->ev_fd,
                 ev->ev_events&EV_READ ? "READ":"",
                 ev->ev_events&EV_WRITE ?"WRITE":"",
                 ev->ev_events&EV_TIMEOUT?"TIME":"",
                 epfd);
      
    assert((ev->ev_flags & EV_INIT));
    
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
       
       return epoll_add_event(ev, actived); 
    }
    Log :: WARN("ADD EVENT fd : %d fail, ALREAD ADDED", ev->ev_fd); 

    return 0;
}  

int Reactor :: epoll_add_event(Event * ev, 
                               bool actived)
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
    op = EPOLL_CTL_ADD;
    events = 0;

    //set add event
    if(evread != NULL)
    {
        events |= EPOLLIN;
        op = EPOLL_CTL_MOD;        
    }
    if(evwrite != NULL)
    {
        events |= EPOLLOUT;
        op = EPOLL_CTL_MOD;
    }
    if(ev->ev_events & EV_READ)
        events |= EPOLLIN;
    if(ev->ev_events & EV_WRITE)
        events |= EPOLLOUT;
    
    //unfinish active add fail 
    pthread_mutex_lock(&event_mutex);
    if((evread != NULL) &&
       (evread->ev_flags & (EV_ACTIVE|EV_READUNFIN))
       && (ev->ev_events & EV_READ))
    {
        pthread_mutex_unlock(&event_mutex);
        Log :: WARN("ADD READ EVENT fail, \
                     IS ACTIVE|UNFINISH fd : %d",fd); 
        return -1; 
    }

    if((evwrite != NULL) &&
       (evwrite->ev_flags & (EV_ACTIVE|EV_WRITEUNFIN))
       && (ev->ev_events  & EV_WRITE))
    {
        pthread_mutex_unlock(&event_mutex);
        Log :: WARN("ADD WRITE EVENT fail, \
                     IS ACTIVE|UNFINISH fd : %d", fd); 
        return -1; 
    }
    
    if(ev->ev_events & EV_READ)
        evep->evread = ev;
    if(ev->ev_events & EV_WRITE)
        evep->evwrite = ev;
   
    event_count++; 
    pthread_mutex_unlock(&event_mutex);
   //unfinish active add fail 
    if(actived)
    {
        if(net_ep_add(epfd, fd, events, evep, op) ==-1)
        {
            Log :: WARN("EPOLL : %d ADD FD : %d ERROR", epfd, fd);
            return -1;
        }
    }
    return 0; 
}

int Reactor :: init(int files)
{
    nevents = files;
    nfile = files;
    if((epfd = epoll_create(nfile)) == -1)
    {
        Log :: ERROR("REACTOR EPOLL CREATE ERROR"); 
        return -1;
    } 
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
    return 0;
}

int Reactor :: start(int status)
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

        int res = epoll_dispatch(status, NULL);
        if(res < 0)
            return -1; 
    }
    return 0;
}


int Reactor :: epoll_dispatch(int status,
                              struct timeval * tv)
{
    int res;
    struct evepoll * evep = NULL;    

    res = epoll_wait(epfd, events, nevents, 3000);
    if(res == -1)
    {
        if(errno != EINTR)
        {
            Log :: WARN("EPOLL DISPATCH LOOP ERROR epfd : %d", epfd);
            return -1;
        }
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
            if(!set_event_active(evread))
            {
                Log :: NOTICE("EPOLL DISPATCH fd : %d read is active, \
                               can not handle HUP", evread->ev_fd);
                continue; 
            }
            if(!set_event_active(evwrite))
            {
                Log :: NOTICE("EPOLL DISPATCH fd : %d write is active, \
                               can not handle HUP", evwrite->ev_fd);
                continue; 
            }
            //TODO : HUP WORK
            
            //set_event_unactive(evread);
            //set_event_unactive(evwrite);
            continue;
        }

        if(active & EPOLLERR)
        {
            if(!set_event_active(evread))
            {
                Log :: NOTICE("EPOLL DISPATCH fd : %d read is active,\
                               can not handle ERROR", evread->ev_fd);
                continue; 
            }
            if(!set_event_active(evwrite))
            {
                Log :: NOTICE("EPOLL DISPATCH fd : %d write is active, \
                               can not handle ERROR", evwrite->ev_fd);
                continue; 
            }
            //TODO : ERROR WORK
            
            //set_event_unactive(evread);
            //set_event_unactive(evwrite);
            continue;
        }
        if(active & EPOLLIN )
        {
            switch(status)
            {
                case EV_ONCE:
                    evread->excute();
                    break;
                case EV_THREAD:
                    if(set_event_active(evread))
                    {
                        //handle
                        printf("multi handle");
                    }    
                    break;
            }
            continue;
        }        

        if(active & EPOLLOUT)
        {
            switch(status)
            {
                case EV_ONCE:
                    evwrite->excute();
                    break;
                case EV_THREAD:
                    if(set_event_active(evread))
                    {
                        //handle
                        printf("multi handle");
                    }    
                    break;
            }
            continue;
        }        
        
    }
    return 0;
}

