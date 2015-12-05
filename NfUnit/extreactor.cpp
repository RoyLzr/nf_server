#include "extreactor.h"


EXTReactor::EXTReactor() : _queue(0), 
	                       _threadsnum(2),
	                       _emptystop(false),
	                       _run(false),
	                       _status(IReactor::INIT),
	                       _maxevs(50000)
	                       {}
EXTReactor::~EXTReactor()
{
	_threadma.join();
	if (_queue) 
    {
		IEvent *ev;
		while (!_queue->empty()) 
        {
			ev = _queue->pop();
			ev->release();
		}
		_queue = 0;
	}
}

IReactor * EXTReactor::getExtReactor() 
{
	return NULL;
}

IReactor * EXTReactor::setExtReactor(IReactor *) 
{
	return NULL;
}

void EXTReactor::setMaxEvents(int num)
{
	_maxevs = num;
}
 
void EXTReactor::setThread(int pnum)
{
	_threadsnum = pnum;
}

int EXTReactor::run()
{
    assert(_queue != 0);

	int ret = 0;
	_run = true;
	_status = IReactor::RUNNING;
    Log::NOTICE("EXTReactor begin running");
	if (_threadsnum > 0) 
    {
		ret = _threadma.run(_threadsnum, TCALLBACK, this);
	} 
    else 
    {
		this->callback();
	}
	return ret;
}

int EXTReactor::stop()
{
    _run = false;
	return 0;
}

int EXTReactor::stopUntilEmpty()
{
    _run = false;
	_emptystop = true;
	return 0;
}

int EXTReactor::join() 
{
	_threadma.join();
	_status = IReactor::STOP;
	return 0;
}

void * EXTReactor::TCALLBACK(void *p)
{
	EXTReactor *rect = static_cast<EXTReactor *>(p);
	rect->callback();
	return NULL;
}

void EXTReactor::extfun()
{
    IEvent *ev;
    ev = _queue->pop();
    if (NULL != ev) 
    {
		extEvent(ev);
    }
#ifndef WORK
    if(ev == NULL)
        printf("reactor queue is empty \n");
#endif    
}

void EXTReactor::callback()
{
	while (_run) 
    {
        extfun();
	}

    if (_emptystop) 
    {
        Log::NOTICE("EXTReactor is stopped, _queue still exists %d ev", _queue->size());
        while(_queue->size() > 0)
        {
            extfun();
        }
    }
    Log::NOTICE("EXTReactor stop SUCC");
    return ;
}


int EXTReactor::status() { return _status; }

int EXTReactor::post(IEvent *ev)
{
    if(!ev)
    {
        Log :: ERROR("call EXTReactor post error, ev is NULL");
        return -1; 
    }

	if(ev->type() != IEvent::CPU && ev->type() != IEvent::NET)
    {
        Log :: ERROR("call EXTReactor post error, ev type is not CPU");
        return -1; 
    }
    if(ev->reactor() == NULL)
	    ev->setReactor(this);
	
    ev->setStatus(IEvent::READY);
    
    assert(_queue != NULL);
    
    //only care about internal ref,
    //this is ref for queue    
	int ret = _queue->push(ev);
	if (ret == 1) 
    {
		return 0;
	}
	return -1;
}

int EXTReactor::cancel(IEvent *ev)
{
	if (!ev) 
    {
        Log::ERROR( "Calling EXTReactor::cancel failed, ev is NULL");
		return -1;
	}
	ev->setStatus(IEvent::CANCELED);
	return 0;
}

int EXTReactor::load(const Section &sec)
{
    _maxevs = atoi(sec.get("MaxEvents").c_str());
    _threadsnum = atoi(sec.get("ThreadNum").c_str());
	return 0;
}

int EXTReactor::setQueue(IEQueue * _q)
{
    if(_q == NULL)
        return -1;
    _queue = _q;
    return 0;
}

