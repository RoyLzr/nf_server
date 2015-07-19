#ifndef _NFSERVER_APP_H
#define _NFSERVER_APP_H

#include "nf_server_core.h"

void
nf_default_handle();

class RaBaseWork : public BaseWork
{};

class RaReadLine : public RaBaseWork
{
    public:
        int work(void *);
};

class LfBaseWork : public BaseWork
{};

class LfReadLine : public LfBaseWork
{
    public:
        int work(void *);
};

class SaBaseWork : public BaseWork
{};

class SaReadLine : public SaBaseWork
{
    public:
        int work(void *);
};



#endif
