
#include "nf_server.h"
#include "nf_server_core.h"

namespace nf
{
    NfServer :: NfServer()
    {
        sev_data = nf_server_create(NULL);
    }

    int NfServer :: set_server_name(const std::string & sev_name)
    {
        if(sev_data == NULL)
            return -1;
        sev_data->name = sev_name;
        return 0;
    }
}
