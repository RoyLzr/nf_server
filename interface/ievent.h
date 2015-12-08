
#ifndef  _IEVENT_H_
#define  _IEVENT_H_

#include "../commonn/memCache.h"
#include "iref.h"

class IReactor;

class IEvent : public IRef
{
    public:
        //status
        enum 
        {
            INIT,
            READY,
            RUNNING,
            DONE,
            CANCELED,
        };
        //result
        enum 
        {
            TIMEOUT     =      0x01,
            IOREADABLE  =      0x02,
            IOWRITEABLE =      0x04,
            SIGNAL      =      0x08,
            CLOSESOCK   =      0x10,
            ERROR       =      0x20,
        };
        //type
        enum
        {
            CPU,
            NET,
        };

        typedef void (*work_handle)(IEvent *, void *);

    public:
        
        virtual int handle() = 0;
        
        virtual void setHandle(int) = 0;

        virtual IReactor *reactor() = 0;
        
        virtual void setReactor(IReactor *) = 0;

        virtual void EventCallback() = 0;
        
        virtual bool isReUsed() = 0;
        
        virtual void setReUsed(bool) = 0;
        
        virtual void setCallback(work_handle cb, void *p) = 0;

        virtual timeval * timeout() = 0;
        
        virtual void setTimeout(int msec) = 0;

        virtual int type() = 0;
        
        virtual void setType(int) = 0;

        virtual int status() = 0;
        
        virtual void setStatus(int) = 0;
        
        virtual int result() = 0;
        
        virtual void setResult(int) = 0;
        
        virtual int derived() = 0;
        
        virtual void setDerived(int) = 0;

        virtual IEvent * next() = 0;
        
        virtual void setNext(IEvent *) = 0;
        
        virtual IEvent * previous() = 0;
        
        virtual void setPrevious(IEvent *) = 0;

        IEvent() {}

        virtual ~IEvent() {};

        virtual bool isError() = 0;
};


#endif

