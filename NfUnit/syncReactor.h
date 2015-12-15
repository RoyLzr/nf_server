#ifndef  __SYNCREACTOR_H_
#define  __SYNCREACTOR_H_
#include "extreactor.h"
#include "../commonn/lock.h"
#include "../net.h"
#include "eventqueue.h"
#include <atomic>

class SyncReactor : public IReactor
{
    public:
        SyncReactor();
        ~SyncReactor();
       
        typedef AutoLock<MLock> AutoMLock;

        virtual int  load(const Section &);

		void setThread(int pNum) 
        { 
            _pNum=pNum;
        }

		virtual void setMaxEvents(int num);
        
    public:
		virtual int stopUntilEmpty();

		virtual int join();
		
        virtual int stop();
		
        virtual int run();

		virtual int status();

		virtual int post(IEvent *);

		virtual int cancel(IEvent *);
		
		virtual IReactor * getExtReactor();
		
        virtual IReactor * setExtReactor(IReactor *);
        
        virtual int create();       

        virtual void destroy();

        virtual int  callback();


    protected:
        int _check_timer;
        IReactor * _extReactor;
        MLock _lock;
        ELQueue _queue;
         
        struct epoll_event * _epev;
        int  _maxEvents;
        int  _epfd;
        bool _run;
        int  _status;
        bool _emptystop;
        bool _pNum;
        std::atomic<int>  _events;
    protected:
        int epollDispatch();
        
        int epollAdd(IEvent *);
                 
};

#endif
