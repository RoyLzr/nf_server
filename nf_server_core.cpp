
#include "nf_server_core.h"


nf_server_t * nf_server_create(const std::string * sev_name)
{
    nf_server_t *sev = (nf_server_t *)malloc(sizeof(nf_server_t));
    if(sev == NULL)
        return NULL;
   
    sev->server_type = NFSVR_LFPOOL;
    sev->connect_type = NFSVR_LONG_CONNEC;
    sev->pthread_num = 0;
    sev->run_thread_num = -1;
    sev->sock_family = AF_INET;
    sev->backlog = 2048;
    
    sev->cb_work = NULL;
    sev->pdata = NULL;
    sev->p_start = NULL;
    sev->p_end = NULL;

    sev->status = INIT;

    if(sev_name == NULL)
        sev->name = "simple server";
    else
        sev->name = *sev_name;
    
    return sev;
}
