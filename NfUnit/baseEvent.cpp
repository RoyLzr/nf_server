#include "baseEvent.h"

EventBase::EventBase() : _fd(-1), 
                         _type(0), 
                         _events(0), 
                         _status(IEvent::INIT), 
                         _to(0), 
                         _cnt(1), 
                         _reactor(0), 
                         _cb(0), 
                         _cbp(0),
	                     _pre(0), 
                         _next(0), 
	                     _devided(0),
                         _reused(false)
{}

void EventBase::set(int hd, int evs, int type, work_handle cb, void *p)
{
	this->setHandle(hd);
	this->setResult(evs);
    this->setType(type);
	this->setCallback(cb, p);
}

bool EventBase::release()
{
    if(delRef() <= 0)
    {
        delete this;
        return true;
    }
    return false;
}

EventBase::~EventBase() 
{
    if(_fd > 0)
        close(_fd);
	_fd = -1;
	_pre = _next = 0;
}

SockEventBase::SockEventBase() : _sockType(BASE)
{}

void SockEventBase::EventCallback()
{
	switch (_sockType) 
    {
		case BASE:
            Log ::DEBUG("sockEventBase base_call");
			return EventBase::EventCallback();
		case ACCEPT:
            Log::DEBUG("SockEventBase accept_callback");
			return accept_callback();
		case READ:
            Log::DEBUG("SockEventBase read_callback");
			return read_callback();
		case WRITE:
            Log::DEBUG("SockEventBase write_callback");
			return write_callback();
		case TCPCONNECT:
            Log::DEBUG("SockEventBase tcp_callback");
			return tcpconnect_callback();
		default:
			return EventBase::EventCallback();
	}
}

int SockEventBase::registerAccept(int fd) 
{
	if (fd < 0) 
    {
        Log::WARN("Calling SockEventBase::accept(fd) failed, beacause the param fd < 0");
		return -1;
	}

	this->setHandle(fd);
	this->setType(IEvent::NET);
	this->setResult(IEvent::IOREADABLE);
	_sockType = ACCEPT;

	return 0;
}


int SockEventBase::registerRead(int fd, size_t count)
{
	if (fd < 0) 
    {
        Log::WARN("Calling SockEventBase::read(fd) failed, beacause the param fd < 0");
		return -1;
	}

	this->setHandle(fd);
	this->setType(IEvent::NET);
	this->setResult(IEvent::IOREADABLE);
	_sockType = READ;
	
    _cnt = count;
	return 0;
}

int SockEventBase::registerWrite(int fd, size_t count)
{
	if (fd < 0) 
    {
        Log::WARN("Calling SockEventBase::write(fd) failed, beacause the param fd < 0");
		return -1;
	}

	this->setHandle(fd);
	this->setType(IEvent::NET);
	this->setResult(IEvent::IOWRITEABLE);
	_sockType = WRITE;

	_cnt = count;
	return 0;
}

int SockEventBase::registerBase()
{
    this->setType(IEvent::CPU);
    _sockType = BASE;
    return 0;
}

int SockEventBase::clear()
{
    if (this->handle() > 0) 
    {
	    close(this->handle());
		this->setHandle(-1);
	}
	return 0;
}

SockEventBase::~SockEventBase()
{
	this->clear();
}

