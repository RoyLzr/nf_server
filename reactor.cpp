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
    
    if((ev->ev_events & (EV_READ|EV_WRITE)) &&
       !(ev->ev_flags & (EV_INSERTED|EV_ACTIVE)))
    {
       event_queue_insert(ev, EV_INSERTED);
       return epoll_add_event(ev); 
    }

    return 0;
}  

int Reactor :: epoll_add_event(Event * ev)
{
    struct evepoll * evep;
    int fd, events, op;
    
    //each fd match read/write event
    //fds store read/write event
    fd = ev->ev_fd;
    if(fd > nfile)
    {
        printf("TOO much fd\n");
        return -1;
    }
    evep = &fds[fd];
    op = EPOLL_CTL_ADD;
    events = 0;

    //set add event
    if(evep->evread != NULL)
    {
        events |= EPOLLIN;
        op = EPOLL_CTL_MOD;        
    }
    if(evep->evwrite != NULL)
    {
        events |= EPOLLOUT;
        op = EPOLL_CTL_MOD;
    }
    if(ev->ev_events & EV_READ)
        events |= EPOLLIN;
    if(ev->ev_events & EV_WRITE)
        events |= EPOLLOUT;
    
    if(net_ep_add(epfd, fd, events, evep, op) ==-1)
    {
        printf("epoll add event error\n");
        return -1;
    }
    
    if(ev->ev_events & EV_READ)
        evep->evread = ev;
    if(ev->ev_events & EV_WRITE)
        evep->evwrite = ev;

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



