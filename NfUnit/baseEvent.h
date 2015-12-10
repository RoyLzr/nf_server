#ifndef  _EVENTBASE_H_
#define  _EVENTBASE_H_

#include "../interface/ievent.h"
#include "../interface/ireactor.h"
#include "../interface/iequeue.h"
#include "../commonn/asynLog.h"
#include "../net.h"
#include <atomic>
	
class EventBase : public IEvent
{
    public:
        void set(int fd, int evs, int type, work_handle cb, void *p);
        
        virtual int handle() { return _fd; }

        virtual void setHandle(int hd) { _fd = hd;}

        virtual IReactor *reactor() { return _reactor; }

        virtual void setReactor(IReactor *r) { _reactor = r; }

        virtual void EventCallback() { _cb(this, _cbp); }

        virtual void setCallback(work_handle cb, void *p)
        {
            _cb = cb;
            _cbp = p;
        }

        virtual timeval * timeout() {return NULL;}

        virtual void setTimeout(int msec) { _to = msec;} 

        virtual int result() { return _events; }
        
        virtual void setResult(int evs) { _events = evs;}
        
        virtual int type() { return _type; }
        
        virtual void setType(int t) { _type = t; }
        
        virtual int getRefCnt() { return _cnt;}

        virtual int addRef() {  return ++_cnt; }
        
        virtual int delRef() { return  --_cnt; }
        
        virtual bool release();

        virtual int status () { return _status;}
        
        virtual void setStatus(int s) { _status = s; }

        virtual bool isReUsed() { return _reused; }
        
        virtual void setReUsed(bool re) { _reused = re; }

        virtual IEvent * next() { return _next; }
        
        virtual void setNext(IEvent *ev) { _next = ev; }
        
        virtual IEvent *previous() { return _pre; }
        
        virtual void setPrevious(IEvent *ev) { _pre = ev; }

        virtual int derived() { return _devided;}

        virtual void setDerived(int div) { _devided = div;}
        
        virtual bool isError() { return false;}
        
        EventBase();
        virtual ~EventBase();

    protected:
        int _fd;
        int _type;
        int _events;
        int _status;
        int _to;
        std::atomic<int> _cnt;

        IReactor *_reactor;
        
        work_handle _cb;
        void * _cbp;

        IEvent *_pre;
        IEvent *_next;

        int _devided;
        bool _reused;
};

class SockEventBase : public EventBase
{
    enum 
    {
        BASE,
        ACCEPT,
        READ,
        WRITE,
        TCPCONNECT,
    };

    public:
        SockEventBase();
        virtual ~SockEventBase();
        
        void setSockType(int t) { _sockType = t;}
        
        int registerAccept(int fd);
        
        int registerBase();
        
        int registerRead(int fd, size_t count);
        
        int registerWrite(int fd, size_t count);
        
        virtual int clear();

        virtual void EventCallback();

    protected:

        virtual void accept_callback() = 0;
        virtual void read_callback() = 0;
        virtual void write_callback() = 0;
        virtual void tcpconnect_callback() = 0;
    
    protected:
        int _sockType;
        int _cnt;
};

#endif

