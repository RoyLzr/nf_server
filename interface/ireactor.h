
#ifndef  _IREACTOR_H_
#define  _IREACTOR_H_

#include "../commonn/configParser.h"

class IEvent;
class IReactor 
{
    public:
        enum 
        {
            INIT,
            STOP,
            RUNNING,
            PAUSE,
        };

    public:
        IReactor() {};
        virtual ~IReactor() {};
    
    public:
        virtual int load(const Section &) = 0;
        
        virtual void setThread(int) = 0;
        
        virtual void setMaxEvents(int) = 0;
    public:
        
        virtual int run() = 0;
        
        virtual int stop() = 0;
        
        virtual int stopUntilEmpty() = 0;

        virtual int join() = 0;

        virtual int status() = 0;

        virtual int post(IEvent *) = 0;

        virtual int cancel(IEvent *) = 0;
        
        virtual int extEvent(IEvent *);

        virtual IReactor * getExtReactor() = 0;
        
        virtual IReactor * setExtReactor(IReactor *) = 0;
};

#endif

