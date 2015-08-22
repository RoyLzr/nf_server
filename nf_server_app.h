#ifndef _NFSERVER_APP_H
#define _NFSERVER_APP_H

#include "nf_server_core.h"

void
nf_default_handle();

class RaBaseWork : public BaseWork
{
    public:
        virtual ~RaBaseWork(){};
};

class RaReadLine : public RaBaseWork
{
    public:
        int work(void *);
        virtual ~RaReadLine(){};
};

class LfBaseWork : public BaseWork
{
    public:
        virtual ~LfBaseWork(){};
};

class LfReadLine : public LfBaseWork
{
    public:
        int work(void *);
        virtual ~LfReadLine(){};
};

class SaBaseWork : public BaseWork
{
    public:
        virtual ~SaBaseWork(){};
};

class SaReadLine : public SaBaseWork
{
    public:
        int work(void *);
        virtual ~SaReadLine(){};
};

#endif
