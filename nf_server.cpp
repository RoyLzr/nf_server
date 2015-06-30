#include "nf_server.h"

namespace nf
{
    NfServer :: NfServer()
    {
        sev_data = nf_server_create(NULL);
    }
    
    NfServer :: ~NfServer()
    {
        if (sev_data != NULL)
        {
            NfServer :: destroy();
        }
        sev_data = NULL;
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

    int NfServer :: load_conf(const std::string & conf_path)
    {
        Singleton<ConfigParser>::instance()->parser_file(conf_path);
        Singleton<ConfigParser>::instance()->scan();
        //Singleton<ConfigParser>::destroy();
    }

    int NfServer :: run()
    {
        int ret;
        Log :: init("/root/nf_server/test/log");
        if( (ret = nf_server_init(sev_data) ) < 0 )
        {   
            Log :: ERROR("nf_server.cpp : 48 INIT ERROR \n");
            return -1;
        }

        Log :: DEBUG("nf_server.cpp : 52 INIT OK", 11, 12);
        
        if( (ret = nf_server_bind(sev_data) ) < 0 )
        {    
            Log :: ERROR("nf_server.cpp : 56 BIND ERROR"); 
            return -1;
        }
        
        if( (ret = nf_server_listen(sev_data) ) < 0 )
        {    
            Log :: ERROR("nf_server.cpp : 62 LISTEN ERROR"); 
            return -1;
        }
         
        sev_data->need_join = 1; 
        
        Allocate :: init();    
    
        return g_pool[sev_data->server_type].run(sev_data); 
    }
    
    int NfServer :: stop()
    {
        sev_data->run = 0;
        return 0;
    }
    
    int NfServer :: join()
    {
        if(sev_data == NULL)
            return -1;
        if(!sev_data->need_join)
            return 0;
        return g_pool[sev_data->server_type].join(sev_data); 
    }
    
    int NfServer :: destroy()
    {
        NfServer :: stop();
        NfServer :: join();    
        if ( sev_data->sev_socket >= 0)
            close(sev_data->sev_socket);
        g_pool[sev_data->server_type].destroy(sev_data);   
        
        Log :: NOTICE("nf_server.cpp : 97 CLOSE THREAD SUCC \n");
 
        if( sev_data->pdata != NULL)
        { 
            for(int i = 1; i < sev_data->pthread_num; i++)
            {
                std::cout << i << std::endl;
                if( sev_data->pdata[i].read_buf != NULL)
                {    
                    free(sev_data->pdata[i].read_buf);
                    sev_data->pdata[i].read_buf = NULL;
                }
                if( sev_data->pdata[i].write_buf != NULL)
                {
                    free(sev_data->pdata[i].write_buf);
                    sev_data->pdata[i].write_buf = NULL;
                }
                if(sev_data->server_type == NFSVR_LFPOOL 
                   && sev_data->pdata[i].rio.rio_ptr != NULL)
                {
                    free(sev_data->pdata[i].rio.rio_ptr);
                    sev_data->pdata[i].rio.rio_ptr = NULL;
                }
            }
        }
        Singleton<ConfigParser>::destroy();
        free(sev_data->pdata);
        free(sev_data);
        Log :: NOTICE("nf_server.cpp : 115 CLOSE SERVER SUCC \n");
    }
    
    int NfServer :: pause()
    {   return 0 ;}
    
    int NfServer :: resume()
    {   return 0 ;}
        
    int NfServer :: set_work_callback(nf_callback_proc run)
    {
        if( sev_data == NULL)
            return -1;
        sev_data->cb_work = run;
        return 1;
    }
        
    int NfServer :: set_server_startfun( nf_handle_t start )
    {   return 0 ;}
        
    int NfServer :: set_thread_startfun( nf_handle_t start ) 
    {
        if( sev_data == NULL)
            return -1;
        sev_data->p_start = start;
        return 1;
    }
        
    int NfServer :: set_thread_endfun( nf_handle_t end )
    {
        if( sev_data == NULL)
            return -1;
        sev_data->p_end = end;
        return 1;
    }
        
}
