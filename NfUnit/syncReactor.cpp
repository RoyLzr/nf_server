#include "syncReactor.h"

SyncReactor::SyncReactor() : _run(false),
                             _status(IReactor::INIT),
                             _extReactor(0),
                             _check_timer(0),
                             _emptystop(false),
                             _events(0),
                             _epev(0),
                             _maxEvents(10000),
                             _epfd(-1)
                             {}

SyncReactor::~SyncReactor()
{
    destroy();
}

void SyncReactor::destroy()
{
    if(_extReactor != NULL)
    {
        if(_emptystop)
            _extReactor->stopUntilEmpty();
        else
            _extReactor->stop();
        delete _extReactor;
    }
    if(_epev == NULL)
    {
        free(_epev);
        _epev = NULL;
    }
    if(_epfd > 0)
    {
        close(_epfd);
        _epfd = -1;
    }
}

IReactor * SyncReactor::getExtReactor() 
{
	return _extReactor;
}

IReactor * SyncReactor::setExtReactor(IReactor * rec) 
{
    _extReactor = rec;
    return _extReactor;
}

void SyncReactor::setMaxEvents(int num)
{
    _maxEvents = num;
}

int SyncReactor::stop()
{
    _run = false;
    if(_extReactor != NULL)
        _extReactor->stop();
	return 0;
}

int SyncReactor::stopUntilEmpty()
{
    _run = false;
    _emptystop = true;
    if(_extReactor != NULL)
        _extReactor->stopUntilEmpty();
    return 0;
}

int SyncReactor::join()
{
    _status = IReactor::STOP;
    if(_extReactor != NULL)
        _extReactor->join();
    return 0;
}

int SyncReactor::run()
{
    int ret = 0;
    if(_extReactor != NULL)
        _extReactor->run();
    _run = true;
    _status = IReactor::RUNNING;
    
    if(NULL == _extReactor)
        Log::NOTICE("SYNC REActor one thread begin running");
    else
       Log::NOTICE("SYNC Reactor extReactor begin running"); 
   
    if((ret = create()) < 0)
        return -1;
    this->callback();
    return 0;
}

int SyncReactor::create()
{
    struct epoll_event ev;
    int ret = 0;
    assert(_maxEvents > 1);

    _epev = static_cast<epoll_event *>(malloc(sizeof(epoll_event) * _maxEvents));
    if(_epev == NULL)
    {
        Log::ERROR("reactro creat error, can not malloc enough epev");
        goto fail;
    }

    _epfd = epoll_create(_maxEvents);
    if(_epfd < 0)
    {
        Log::ERROR("reactor create error, create epfd error");
        goto fail;
    }
    return 0;

fail:
    destroy();
    return -1;
}

int SyncReactor::status() { return _status; }

int SyncReactor::callback()
{
    int res;
    while(!_queue.empty())
    {
        IEvent * ev = _queue.pop();
        this->post(ev); 
    }
    while(true)
    {
        /*
        if(_events <= 0)
            Log::WARN("Sync Reactor has no event");
        continue;
        */
        res = epollDispatch();
        if(res < 0)
            return res;
    }
    return 0;
}

int SyncReactor::epollDispatch()
{
    int ret = 0;
    int num = epoll_wait(_epfd, _epev, _maxEvents, 3000); 
    Log::DEBUG("wait done %d", num);

    for(int i  = 0; i < num; i++)
    {
        IEvent *ev = static_cast<IEvent *>(_epev[i].data.ptr);
        
        int events = 0;
        if((_epev[i].events & EPOLLHUP) ||
            _epev[i].events & EPOLLERR)
        {
            Log::WARN("EPOLLHUP | EPOLLERR event happen in %d", _epev[i].data.fd);
            events = events | IEvent::CLOSESOCK;
        }

        if(_epev[i].events & EPOLLIN)
            events |= IEvent::IOREADABLE;
        if(_epev[i].events & EPOLLOUT)
            events |= IEvent::IOWRITEABLE;
        if(ev->result() & IEvent::ACCEPT) 
            events |= IEvent::ACCEPT;

        ev->setResult(events);
        if(events & IEvent::CLOSESOCK)
        {
            delete ev;
            continue;
        }
        if(events & IEvent::ACCEPT)
        {
            extEvent(ev);
            continue;
        }
        
        ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, ev->handle(), NULL);
        if(ret < 0)
        {
            Log::WARN("del event fd from epoll error, %d : %s", ev->handle(), strerror(errno));
        }

        if(_extReactor != NULL)
            _extReactor->post(ev);
        else
            extEvent(ev);
    }
    return num;
}

int SyncReactor::load(const Section & sec)
{
    _maxEvents = atoi(sec.get("MaxEvents").c_str());
    return 0;    
}

int SyncReactor::cancel(IEvent *ev)
{
    if(NULL == ev)
    {
        Log::NOTICE("cancel ev is NULL, SyncReactor::cancel");
        return -1;
    }
	ev->setStatus(IEvent::CANCELED);
	return 0;
}

int SyncReactor::post(IEvent * ev)
{
    Log::DEBUG("Post event in syncReactor ev %d", ev->handle());

    if(ev == NULL)
    {
        Log::WARN("SyncReactor::post add ev is null");
        return -1;
    }
    ev->setStatus(IEvent::READY);

    ev->setReactor(this);
    if(this->status() != IReactor::RUNNING)
    {
        Log::WARN("syncReactor::post reactor is not runing, store in _queue");
        _queue.push(ev);
        return 0;
    }
    if(ev->type() & IEvent::CPU)
    {
        if(_extReactor != NULL)
            _extReactor->post(ev);
        else
            extEvent(ev);        
    }
    else if(ev->type() & IEvent::NET)
    {
        this->epollAdd(ev);
    }
    return 0;
}

int SyncReactor::epollAdd(IEvent * ev)
{
    if(NULL == ev)
    {
        Log::WARN("SyncReactor::epollAdd error, ev is NULL");
        return -1;
    }
    if(!(ev->type() & IEvent::NET))
    {
        Log::WARN("SyncReactor::epollAdd error, ev is NULL");
        return -1;
    }

    struct epoll_event epev;
    epev.data.fd = ev->handle();
    epev.data.ptr = ev;
    epev.events = EPOLLHUP | EPOLLERR;
    if(ev->result() & IEvent::IOREADABLE)
       epev.events |= EPOLLIN;
    if(ev->result() & IEvent::IOWRITEABLE)
       epev.events |= EPOLLOUT; 

    int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, ev->handle(), &epev);
    if(ret != 0)
    {
        Log::WARN("syncReactor::epolladd error, add epoll_ctl error :%s", strerror(errno));
    }
    return ret;
}


