#ifndef  _EQUEUE_H_
#define  _EQUEUE_H_

#include "../interface/ievent.h"
#include "../interface/iequeue.h"

class EQueue : public IEQueue
{
    public:
        EQueue() : begin(0), 
                   end(0), 
                   cap(0),
                   used(0)
        {}

        ~EQueue() 
        {}
       
        //iequeue operator 
        size_t size() const{ return used;}

        size_t maxSize() const { return cap;}

        void setMaxSize(size_t maxsize) { cap = maxsize;}

        bool empty() const { return begin == 0;}

        bool full() const { return size() >= cap;}

        size_t pushs_ms(IEvent **ev, size_t items, int msec) { return 0;}
        
        size_t push_ms(IEvent *ev, int msec) { return 0;}

        size_t push(IEvent *ev) { return pushs(&ev, 1);}

        size_t pushs(IEvent **ev, size_t items);

        size_t pops_ms(IEvent **ev, size_t items, int msec) { return 0;};
        
        size_t pops(IEvent **ev, size_t items);
        
        IEvent * pop_ms(int msec) { return 0;};
        
        IEvent * pop();
        
        //new ievent operator
        inline IEvent * getBegin() { return begin; }
        
        void erase(IEvent * ev);
        
        void clear() 
        { 
            begin = end = 0;
            used = 0;
        }

    public:
        IEvent * begin;
        IEvent * end;
        int cap;
        int used;
};

#endif
