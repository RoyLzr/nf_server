#ifndef _PARSE_WORK_
#define _PARSE_WORK_

#include "reactor.h"

class ParseBase
{
    public:
        virtual int work(int, 
                         void *
                         ) = 0;
        virtual ~BaseWork(){};
};

class ParseLine : public BaseWork
{
    public:
        virtual  ~ParseLine(){};
};


#endif
