#include "nf_server.h"
#include "lfpool.h"
#include "rapool.h"
#include "sapool.h"

static struct NfServer * svr[] = 
{
    new LfServer(),
    new SaServer(),
    new RaServer(),
};

class Factory
{

    public:
        Factory(const std::string  & conf)
        {
            Singleton<ConfigParser>::instance()->parser_file(conf);
            Singleton<ConfigParser>::instance()->scan();
            svr_type = (size_t)atoi((Singleton<ConfigParser>::instance()->get("server", "type")).c_str());
        };
        virtual ~Factory(){};

    NfServer * create_svr()
    {
        (svr[svr_type]->get_server_data())->server_type = svr_type; 
                
        return svr[svr_type];
    };
    private:
        int svr_type;
};
