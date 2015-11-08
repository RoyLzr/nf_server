#ifndef _PARSE_WORK_
#define _PARSE_WORK_

#include "reactor.h"
#include "event.h"
#include "Buffer.h"

class ParseFun
{
    public:
        ParseFun(){};
        virtual ~ParseFun(){};
        virtual int work(int, void *) = 0;
};

class NonBlockFun : public ParseFun
{
    public:
        NonBlockFun(){};
        virtual ~NonBlockFun(){};
};


class NonBlockReadLine : public NonBlockFun
{
    public:
        NonBlockReadLine(){}
        virtual ~NonBlockReadLine(){}

        int work(int, void *);
};

class NonBlockWrite : public NonBlockFun
{
    public:
        NonBlockWrite(){}
        virtual ~NonBlockWrite(){}

        int work(int, void *);
};


NonBlockWrite * writeData();

NonBlockReadLine * parseLine();


#endif
