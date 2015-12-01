#ifndef  _EXTREACTOR_H_
#define  _EXTREACTOR_H_

#include "../interface/ireactor.h"
#include "../interface/ievent.h"
#include "../interface/iequeue.h"
#include "../commonn/lock.h"
#include "../commonn/ThreadManager.h"
#include "../commonn/configParser.h"
#include "../commonn/asynLog.h"
#include <assert.h>

class EXTReactor : public IReactor
{
	public:
		EXTReactor();

		virtual ~EXTReactor();
		
        virtual int  load(const Section &);

		virtual void setThread(int pnum);

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
		
        virtual IReactor * setExtReactor();
        
        virtual int setQueue(IEQueue *);

	protected:
		IEQueue *_queue;
		MLock _lock;

		ThreadManager _threadma;
		int _threadsnum;

		bool _emptystop;
		bool _run;

		int _status;
		int _maxevs;

	protected:
        virtual void callback();
        static void * TCALLBACK(void *);
    private:
        inline void extfun();
};


#endif
