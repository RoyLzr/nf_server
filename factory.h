#include "nf_server.h"
#include "lfpool.h"
#include "rapool.h"
#include "sapool.h"

static struct NfServer * svr[] = 
{
    new LfServer(),
    new RaServer(),
    new SaServer(),
};

class Factory
{

    public:
        Factory(const std::string  & conf)
        {
            
        };
        virtual ~Factory(){};

    NfServer * create_svr()
    {
        svr[NFSVR_LFPOOL]->load_conf(conf_path);    
        int type = svr[NFSVR_LFPOOL]->sev_data->server_type;

        if(svr[NFSVR_LFPOOL]->sev_data->server_type != NFSVR_LFPOOL)
                
        return svr[2];
    };
    private:
        int svr_type;
};
