#include "ireactor.h"
#include "iequeue.h"
#include "ievent.h"


size_t IEQueue :: push(IEvent * ev)
{
    return this->pushs_ms(&ev, 1, 0);
}

size_t IEQueue :: pushs(IEvent ** ev, size_t items)
{
    return this->pushs_ms(ev, items, 0);
}

size_t IEQueue :: push_ms(IEvent * ev, int msec)
{
    return this->pushs_ms(&ev, 1, msec);
}

size_t IEQueue :: pops(IEvent ** ev, size_t items)
{
    return this->pops_ms(ev, items, 0);
}

IEvent * IEQueue :: pop()
{
    return this->pop_ms(0);
}

int IReactor::extEvent(IEvent * ev)
{
    ev->setStatus(IEvent::RUNNING);
    ev->EventCallback();

    if(ev->status() == IEvent::CANCELED)
    {
        delete ev;
        goto done;
    }
    else if(ev->status() == IEvent::RUNNING)
    {
        if(ev->result() & IEvent::ACCEPT)
            goto done;
        (ev->reactor())->post(ev);
    }
    else if((ev->status() == IEvent::DONE) &&
            (ev->isReUsed()))
        (ev->reactor())->post(ev);
    else
        delete ev;
done:
    return 0;
}

