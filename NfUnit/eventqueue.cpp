#include "eventqueue.h"

size_t EQueue::pushs(IEvent **ev, size_t items)
{
    if (ev) 
    {
        for(size_t i=0; i<items; ++i)
        {
            if (end) 
            {
                ev[i]->setNext(NULL);
                end->setNext(ev[i]);
                ev[i]->setPrevious(end);
                end = ev[i];
            }
            else 
            {
                ev[i]->setPrevious(NULL);
                ev[i]->setNext(NULL);
                begin = ev[i];
                end = ev[i];
            }
        }
        used += items;
        return items;
    }
    return 0;
}

size_t EQueue::pops(IEvent **ev, size_t items) 
{
    if(NULL == ev) { return 0;}

    size_t i = 0;
    for(i=0; i<items; i++)
    {
        if (begin) 
        {
            ev[i] = begin;
            begin = begin->next();
            ev[i]->setNext(NULL);
            ev[i]->setPrevious(NULL);

            if (begin == NULL) 
            {
                end = NULL;
                used -= i + 1;
                return i + 1;
            } 
            else 
            {
                begin->setPrevious(NULL);
            }
        }
        else
            break;
    }
    used -= i;
    return i;
}


IEvent * EQueue::pop() 
{
    IEvent * ev = NULL;
    if (begin) 
    {
        ev = begin;
        begin = begin->next();
        ev->setPrevious(NULL);
        ev->setNext(NULL);

        if (begin == NULL) 
        {
            end = NULL;
            goto END;
        } 
        else 
        {
            begin->setPrevious(NULL);
        }
    }
    else
        return NULL;

END:
    used --;
    return ev;
}

void EQueue::erase(IEvent * ev) 
{
    if (ev == NULL) { return; }

    if (ev == begin) 
    {
        begin = begin->next();
        if (begin == NULL) 
            end = NULL;
        else 
            begin->setPrevious(NULL);
    } 
    else if (ev == end) 
    {
        end = end->previous();
        if (end == NULL) 
            begin = NULL;
        else 
            end->setNext(NULL);
    } 
    else 
    {
        ev->previous()->setNext(ev->next());
        ev->next()->setPrevious(ev->previous());
    }
    ev->setNext(NULL);
    ev->setPrevious(NULL);
    used--;
}

