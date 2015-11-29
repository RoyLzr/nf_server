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

IEvent :: ~IEvent() {}
    



