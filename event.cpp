#include "event.h"
#include "reactor.h"
#include "nf_base_work.h"
#include "nf_server_core.h"

void ReadEvent :: init(int fd,
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

void WriteEvent :: init(int fd,
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

void ListenEvent :: init(nf_server_t * svr)
{
    assert(svr != NULL);
    sev = svr;

    int fd = sev->sev_socket;
    int events = EV_READ | EV_LISTEN;
    Event :: init(fd, events , NULL);

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
         
        ev_reactor->set_event_unactive(this);
        
        Log :: NOTICE("one event will be closed");
        ev_reactor->del_event(this);

        return;
    }
    if(ev_events & EV_LISTEN || !(ev_flags & EV_ACTIVE))
        return;

    ev_reactor->set_event_unactive(this);

    return;
}

int Event :: excute_fun()
{
    int ret = ev_callback(ev_fd, ev_events, this);
    return ret;
}

int ReadEvent :: excute_fun()
{
    //this event is already active
    int epfd = ev_reactor->get_epfd();
    int res;
    struct evepoll * events = NULL;
    events = ev_reactor->get_fds();

    if((res = ev_reactor->pause_event(this)) < 0)
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
        if((res = ev_reactor->resume_event(this)) < 0)
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
    

    if((res = ev_reactor->pause_event(this)) < 0)
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
        if((res = ev_reactor->resume_event(this)) < 0)
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

int ListenEvent :: excute_fun()
{
    int ret = 0;
    int sev_sock = ev_fd;
    char ip[40];
    int len = 40;
    int port = 0;
    int cli_sock = 0;
    struct sockaddr  addr;
    socklen_t  addrlen;
    
    cli_sock = net_accept(sev_sock, &addr, &addrlen);
    if (cli_sock < 0) 
    {
        Log :: WARN("ACCEPT %d ERROR %s", sev_sock, strerror(errno));
        return 0;
    }
        
    get_tcp_sockaddr(ip, &port, (sockaddr_in *) (&addr), len);
    Log :: NOTICE("ACCEPT SUCC FROM CLIENT: %s:%d  new fd : %d ", 
                  ip, port, cli_sock);

    set_sev_socketopt(sev, cli_sock);
    
    ReadEvent * r_ev = new ReadEvent();
    WriteEvent * w_ev = new WriteEvent();
    if(r_ev == NULL || w_ev == NULL)
        return -1;
    
    r_ev->init(cli_sock, sev->read_handle, sev->read_parse_handle);
    w_ev->init(cli_sock, sev->write_handle, sev->write_parse_handle);
     
    if(sev->svr_reactor->add_event(r_ev) < 0 )
    {
        close(cli_sock);  
        return -1;
    }
    
    if(sev->svr_reactor->add_event(w_ev, NULL, false)<0)
    {
        close(cli_sock); 
        return -1;
    }
    
    sev->svr_reactor->set_cli_data(cli_sock, (sockaddr_in *)(&addr));
    
    return 0; 
}

