#include "reactor.h"


void Reactor :: event_queue_insert(Event *ev,
                                  int status)
{
    ev->ev_flags |= status;
    switch(status)
    {
        case EV_INSERTED:
        {
            ev_list.push_back(ev);
            ev->ev_pos = --ev_list.end();
        
            ev->ev_reactor = this;
            event_count++;
            ev->ev_flags |= EV_INSERTED;
            break;
        }
        default:
            printf("this status %d not support\n", status);    
    }
}

int Reactor :: add_event(Event * ev,
                         struct timeval * tv)
{
     
    printf("add event fd : %d event: %s%s\n",
            ev->ev_fd,
            ev->ev_events&EV_READ ? "READ":"",
            ev->ev_events&EV_WRITE ?"WRITE": "",
            ev->ev_events&EV_TIMEOUT?"TIME":"");      
    assert((ev->ev_flags & EV_INIT));
    
    if(tv != NULL)
    {
        assert(ev->ev_events & EV_TIMEOUT);
        printf("add timeout event\n");
    }
   
    pthread_mutex_lock(&event_mutex); 
    if((ev->ev_events & (EV_READ|EV_WRITE)) &&
       !(ev->ev_flags & (EV_INSERTED|EV_ACTIVE)))
    {
       event_queue_insert(ev, EV_INSERTED);
       pthread_mutex_unlock(&event_mutex); 
       return epoll_add_event(ev); 
    }
    pthread_mutex_unlock(&event_mutex); 

    return 0;
}  

int Reactor :: epoll_add_event(Event * ev)
{
    struct evepoll * evep;
    int fd, events, op;
    Event * evread, *evwrite;
     
    //each fd match read/write event
    //fds store read/write event
    fd = ev->ev_fd;
    if(fd > nfile)
    {
        printf("TOO much fd\n");
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
       (evread->ev_flags & (EV_ACTIVE|EV_READUNFIN))        && (ev->ev_events & EV_READ))
    {
        pthread_mutex_unlock(&event_mutex);
        return -1; 
    }

    if( (evwrite != NULL)&&
     (evwrite->ev_flags & (EV_ACTIVE|EV_WRITEUNFIN))     && (ev->ev_events & EV_WRITE))
    {
        pthread_mutex_unlock(&event_mutex);
        return -1; 
    }
    
    if(ev->ev_events & EV_READ)
        evep->evread = ev;
    if(ev->ev_events & EV_WRITE)
        evep->evwrite = ev;
    
    pthread_mutex_unlock(&event_mutex);
   //unfinish active add fail 
 
    if(net_ep_add(epfd, fd, events, evep, op) ==-1)
    {
        printf("epoll add event error\n");
        return -1;
    }

    return 0; 
}

int Reactor :: init(int files)
{
    nevents = files;
    nfile = files;
    if((epfd = epoll_create(nfile)) == -1)
    {
        //Log :: ERROR("REACTOR EPOLL CREATE ERROR"); 
        printf("epoll_create error\n");
        return -1;
    } 
    set_fd_block(epfd);   

    //events is used to accept epoll event
    events = (struct epoll_event *) malloc(nevents * sizeof(struct epoll_event));
    if(events == NULL)
    {
        //Log :: ERROR("REACTOR can not malloc %d epoll_events", nevents);
        printf("reactor malloc error\n");
        return -1;
    }
    fds = (struct evepoll *)calloc(nfile, sizeof(struct evepoll));
    
    if(fds == NULL)
    {
        free(events);
        //Log :: ERROR("REACTOR can not malloc %d evepoll", nfile);
        printf("reactor malloc error\n");
        return -1;
    }
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
            printf("empty event\n");
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
    struct evepoll * evep;    

    res = epoll_wait(epfd, events, nevents, 3000);
    if(res == -1)
    {
        if(errno != EINTR)
        {
            printf("epoll loop error\n");
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

        if( active & EPOLLHUP)
        {
            //TODO : 
            pthread_mutex_lock(&event_mutex);
            if( (evread->ev_flags|evwrite->ev_flags) & EV_ACTIVE)
            {
                pthread_mutex_unlock(&event_mutex);
                printf("active epoll can not delete");
                continue;
            }
            evread->ev_flags = EV_ACTIVE;
            evwrite->ev_flags = EV_ACTIVE;
            //TODO : work
            pthread_mutex_unlock(&event_mutex);
            continue;
        }

        if(active & EPOLLERR)
        {
            //TODO : 
            pthread_mutex_lock(&event_mutex);
            if((evread->ev_flags|evwrite->ev_flags) & EV_ACTIVE)
            {
                pthread_mutex_unlock(&event_mutex);
                printf("active epoll can not delete");
                continue;
            }
            evread->ev_flags = EV_ACTIVE;
            evwrite->ev_flags = EV_ACTIVE;
            //TODO : work
            pthread_mutex_unlock(&event_mutex);
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
                    pthread_mutex_lock(&event_mutex);
                    if(evread->ev_flags & EV_ACTIVE)
                    {
                        pthread_mutex_unlock(&event_mutex);
                        continue;
                    }
                    evread->ev_flags |= EV_ACTIVE;
                    pthread_mutex_unlock(&event_mutex);
                    break;
            }
        }        
        if(active & EPOLLOUT )
        {
            switch(status)
            {
                case EV_ONCE:
                    evwrite->excute();
                    break;
                case EV_THREAD:
                    pthread_mutex_lock(&event_mutex);
                    if(evwrite->ev_flags & EV_ACTIVE)
                    {
                        pthread_mutex_unlock(&event_mutex);
                        continue;
                    }
                    evwrite->ev_flags |= EV_ACTIVE;
                    pthread_mutex_unlock(&event_mutex);
                    break;
            }
        }        
        
    }
    return 0;
}
