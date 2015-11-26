
#ifndef  _IEQUEUE_H_
#define  _IEQUEUE_H_


class IEvent;
class IEQueue
{
    public:

        virtual size_t maxSize() const = 0;
        
        virtual void setMaxSize(size_t maxsize) = 0;

        virtual size_t size() const = 0;

        virtual bool empty() const = 0;

        virtual bool full() const = 0;

        virtual size_t pushs_ms(IEvent **ev, size_t items, int msec) = 0;
        
        virtual size_t push_ms(IEvent *ev, int msec) = 0;

        virtual size_t push(IEvent **ev, size_t items);
        
        virtual size_t pops_ms(IEvent **ev, size_t items, int msec);
        
        virtual size_t pops(IEvent **ev, size_t items);
        
        virtual size_t pop(IEvent **ev, size_t items);

        virtual ~IEQueue() {}
};

#endif
