
#include "nf_server.h"
#include "nf_server_core.h"

namespace nf
{
    NfServer :: NfServer()
    {
        sev_data = nf_server_create(NULL);
    }
    
    NfServer :: ~NfServer()
    {
        std::cout << "deconstructor" << std::endl;
    }

    int NfServer :: set_server_name(const char * sev_name)
    {
        if(sev_data == NULL)
            return -1;

        strncpy(sev_data->name, sev_name, sizeof(sev_data->name));
        sev_data->name[sizeof(sev_data->name) - 1] = '\0';
        
        return 0;
    }
    nf_server_t * NfServer :: get_server_data()
    {
        return sev_data;
    }

    int NfServer :: run()
    {
        int ret;
        if( (ret = nf_server_init(sev_data) ) < 0 )
        {    std::cout << "init error" << std::endl; return -1;}
        
        if( (ret = nf_server_bind(sev_data) ) < 0 )
        {    std::cout << "bind error" << std::endl; return -1;}
        
        if( (ret = nf_server_listen(sev_data) ) < 0 )
        {    std::cout << "listen error" << std::endl; return -1;}
        
        sev_data->need_join = 1; 
    
        return 0; 
    } 
}
