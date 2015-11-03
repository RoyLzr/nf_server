#include "nf_server.h"

NfServer :: NfServer()
{
    sev_data = nf_server_create(NULL);
}

NfServer :: ~NfServer()
{
    if (sev_data != NULL)
    {
        //NfServer :: destroy();
    }
    sev_data = NULL;
}

int NfServer :: init(const std::string & logPath)
{
    Singleton<ConfigParser>::instance()->parser_file(logPath);

    Singleton<ConfigParser>::instance()->scan();
    int ret;
    //INIT LOG
    Log :: init(Singleton<ConfigParser>::instance()->get("server", 
                "logPath").c_str());

    Log :: set_level(atoi(Singleton<ConfigParser>::instance()->get("server", 
                    "logLevel").c_str()));
    
    //INIT memory pool
    Allocate :: init();

    nf_server_t * sev = sev_data;

    sev->run = 1;
    //conf 相关内容初始化
    //监听端口
    sev->listen_port = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("server", "listenPort")).c_str());
    //超时设置
    sev->connect_to = (size_t)atoi((Singleton<ConfigParser>::
                                    instance()->get("server", "connectTo")).c_str());

    sev->read_to = (size_t)atoi((Singleton<ConfigParser>::
                                 instance()->get("server", "readTo")).c_str());

    sev->write_to = (size_t)atoi((Singleton<ConfigParser>::
                                  instance()->get("server", "writeTo")).c_str());

    //connect 方法
    sev->connect_type = (size_t)atoi((Singleton<ConfigParser>::
                                      instance()->get("server", "connectType")).c_str());
    
    //server_type
    sev->server_type = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("server", "type")).c_str());
    
    sev->socksize = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("reactor", "sock_num")).c_str());

    //线程数
    sev->thread_num = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("reactor", "threadNum")).c_str());

    sev->reactor_num = (size_t)atoi((Singleton<ConfigParser>::
                                     instance()->get("reactor", "reactorNum")).c_str());

    //线程栈
    //sev->stack_size = 10485760;  //10M
    sev->stack_size = (size_t)atoi((Singleton<ConfigParser>:: 
                                    instance()->get("thread", "stackSize")).c_str());

    sev->thread_usr_buf = (size_t)atoi((Singleton<ConfigParser>:: 
                                    instance()->get("thread", "threadUsrBuf")).c_str());
  
    sev->svr_reactor = new NfReactor[sev->reactor_num];

    if(sev->svr_reactor == NULL)
        return -1;

    for(size_t i = 0; i < (sev->reactor_num); i++)
    { 
       if( (ret = sev->svr_reactor[i].init(sev->socksize, sev)) < 0 )
            return -1;
    }
    Log :: NOTICE("SVR INIT OK");
    return 0;
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

    nf_server_t * sev = sev_data;
         
    assert(sev->read_handle != NULL);
    assert(sev->write_handle != NULL);
    assert(sev->read_parse_handle != NULL);
    assert(sev->write_parse_handle != NULL);
    
    /*
    ReadEvent * r_ev = new ReadEvent();
    WriteEvent * w_ev = new WriteEvent();
    
    r_ev->init(1, sev->read_handle, sev->read_parse_handle);
    w_ev->init(1, sev->write_handle, sev->write_parse_handle);
   
    sev->svr_reactor->add_event(r_ev);
    sev->svr_reactor->add_event(w_ev);

    sev->svr_reactor->start(EV_THREAD);
    
    return 0;
   */ 

    if((ret = nf_server_bind(sev_data) ) < 0 )
    {    
        Log :: ERROR("nf_server.cpp : 56 BIND ERROR"); 
        return -1;
    }

    if((ret = nf_server_listen(sev_data) < 0)
    {    
        Log :: ERROR("SVR LISTEN ERROR"); 
        return -1;
    }
    
    if((ret = svr_init()) < 0)
    {    
        Log :: ERROR("SVR INIT ERROR"); 
        return -1;
    }

    sev_data->need_join = 1; 

    return svr_run(); 
}

/*
int NfServer :: stop()
{
    sev_data->run = 0;
    Log :: NOTICE("SVR STOPPED");
    //Log :: set_status(2); 
    return 0;
}

int NfServer :: join()
{
    if(sev_data == NULL)
        return -1;
    if(!sev_data->need_join)
        return 0;
    return svr_join(); 
}

int NfServer :: destroy()
{
    NfServer :: join();    
    if ( sev_data->sev_socket >= 0)
        close(sev_data->sev_socket);

    svr_destroy();   
    Log :: NOTICE("nf_server.cpp : 97 CLOSE THREAD SUCC \n");

    delete sev_data->stratgy;

    if( sev_data->pdata != NULL)
    { 
        for(size_t i = 0; i < sev_data->pthread_num; i++)
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
    sleep(1);
    free(sev_data->pdata);
    free(sev_data);
    Log :: NOTICE("nf_server.cpp : 115 CLOSE SERVER SUCC \n");
    sleep(1);
    return 0;
}

int NfServer :: pause()
{   return 0 ;}

int NfServer :: resume()
{   return 0 ;}

*/

int NfServer :: set_read_handle(ev_handle read_handle)
{
    if( sev_data == NULL)
        return -1;
    sev_data->read_handle = read_handle;
    return 1;
}

int NfServer :: set_write_handle(ev_handle write_handle)
{
    if( sev_data == NULL)
        return -1;
    sev_data->write_handle = write_handle;
    return 1;
}

int NfServer :: set_parse_write_handle(parse_handle write_handle)
{
    if( sev_data == NULL)
        return -1;
    sev_data->write_parse_handle = write_handle;
    return 1;
}

int NfServer :: set_parse_read_handle(parse_handle read_handle)
{
    if( sev_data == NULL)
        return -1;
    sev_data->read_parse_handle = read_handle;
    return 1;
}

nf_server_t * 
NfServer :: nf_server_create(const char * sev_name)
{
    nf_server_t *sev = (nf_server_t *)malloc(sizeof(nf_server_t));
    memset(sev, 0, sizeof(nf_server_t));

    if(sev == NULL)
        return NULL;
   
    sev->server_type = NFSVR_SAPOOL;
    sev->connect_type = NFSVR_LONG_CONNEC;
    sev->thread_num = 0;
    sev->reactor_num = 1;
    
    sev->sock_family = AF_INET;
    sev->backlog = 2048;
    sev->thread_usr_buf = 1024;
 
    sev->stack_size = 10485760;  //10M
    sev->sev_socket = -1;    
    
    sev->listen_prio = 10;
    sev->work_prio = 5;  

    sev->socksize = 50000; 

    sev->read_to = 60;
    sev->write_to = 60;
    sev->connect_to = 60;

    sev->read_handle = NULL;
    sev->read_parse_handle = NULL;
    sev->write_handle = NULL;
    sev->write_parse_handle = NULL;
    
    sev->svr_reactor = NULL;

    sev->status = INIT;

    if(sev_name == NULL)
    {
        strncpy(sev->name, "simple server", sizeof(sev->name));
        sev->name[sizeof(sev->name) - 1] = '\0';
    }
    else
    {
        strncpy(sev->name, sev_name, sizeof(sev->name));
        sev->name[sizeof(sev->name) - 1] = '\0';
    } 
    return sev;
}

