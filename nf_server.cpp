#include "nf_server.h"

NfSvr :: NfSvr()
{
    _sev_data = nf_server_create();
    _name     = "Defult";
}

NfSvr :: NfSvr(IReactor * act)
{
    assert(act != NULL);

    _sev_data = nf_server_create();
    _sev_data->_svr_reactor  = act;
    _name     = "Defult";
}

NfSvr::~NfSvr()
{
    if(NULL != _sev_data)
        delete _sev_data;
}

nf_server_t * 
NfSvr :: nf_server_create()
{

    nf_server_t *sev = (nf_server_t *)malloc(sizeof(nf_server_t));
    assert(sev != NULL);
    memset(sev, 0, sizeof(nf_server_t));
     
    sev->_svr_type = SYNC;
    
    sev->_backlog = 2048;
    sev->_port = 80;
    sev->_connect_to = 60;
    sev->_read_to    = 60;
    sev->_write_to   = 60;
    sev->_session_to = 120;
    
    sev->_max_session = 500000;
    sev->_session_cnt = 0;
    
    sev->_sock_family   = AF_INET;
    sev->_listen_socket = -1;
    sev->_sockopt = 0;

    sev->_svr_reactor = NULL;
    sev->_status      = INIT;
    
    return sev;
}


int NfSvr :: init(const Section & sec)
{
    int ret = 0;
    Log::init(sec.get("logPath").c_str());
    Log::set_level( atoi(sec.get("logLevel").c_str()));

    Allocate::init();
    
    nf_server_t * sev = _sev_data;

    //conf 相关内容初始化
    //监听端口
    sev->_port       = (size_t)atoi((sec.get("listenPort").c_str()));
    //超时设置
    sev->_connect_to = (size_t)atoi((sec.get("connectTo").c_str()));

    sev->_read_to    = (size_t)atoi((sec.get("readTo").c_str()));

    sev->_write_to   = (size_t)atoi((sec.get("writeTo").c_str()));
    
    sev->_session_to = (size_t)atoi((sec.get("sessionTo").c_str()));

    //server_type
    sev->_svr_type   = (size_t)atoi((sec.get("type").c_str()));
    
    sev->_max_session = (size_t)atoi((sec.get("sockNum").c_str()));

    Log :: NOTICE("SVR INIT OK");
    return 0;
}

int NfSvr :: run()
{
    int ret = 0;

    IReactor        * coreAct = _sev_data->_svr_reactor;
    SockEventBase   * coreEv  = _sev_data->_acc_event;
    
    if((ret = nf_server_bind()) < 0 )
    {    
        Log :: ERROR("NfSvr::run::nf_server_bind ERROR"); 
        return -1;
    }

    if((ret = nf_server_listen()) < 0)
    {    
        Log :: ERROR("SVR LISTEN ERROR"); 
        return -1;
    }

    assert(coreAct != NULL);
    assert(coreEv  != NULL);
    
    coreEv->registerAccept(_sev_data->_listen_socket);
    coreEv->setReUsed(true);

    coreAct->post(coreEv);
    coreAct->run();

    return 0;
}

int NfSvr :: stop()
{
    nf_server_t * sev = _sev_data;
    sev->_status = STOP;
    if(sev->_svr_reactor != NULL)
        (sev->_svr_reactor)->stop();
}

int NfSvr :: join()
{
    return 0;
}

int NfSvr :: resume()
{
    return 0;
}

int NfSvr :: pause()
{
    return 0;
}

int NfSvr :: destroy()
{
    return 0;
}

//1. SO_LINGER 2. TCP_NODELAY 3. DEFER_ACCEPT
int NfSvr::set_sev_socketopt()
{
    const int on = 1;
    nf_server_t * sev = _sev_data;
    int fd = sev->_listen_socket;

    if(sev->_sockopt & LINGER)
    {
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    }
    
    if(sev->_sockopt & TCP_NODELAY)
    {
        struct linger li;
        memset(&li, 0, sizeof(li)); 
        li.l_onoff = 1;
        li.l_linger = 1;
        setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&li, sizeof(li) );     
    }

    if(sev->_sockopt & DEFER_ACC)
    {
        int timeout = sev->_connect_to;
        setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, sizeof(timeout));
    }

    return 0; 
}

int NfSvr::nf_server_bind()
{
    //set svr : listen socket 
    const int on = 1;
    struct sockaddr_in addr;
    
    if(_sev_data->_listen_socket < 0)
    {
        if((_sev_data->_listen_socket=socket(PF_INET, SOCK_STREAM, 0))<0)
        {
            Log :: ERROR("NF_SVR_CORE : CREATE SOCKET ERROR : %s", strerror(errno));
            return -1; 
        }
    }
    //复用
    setsockopt(_sev_data->_listen_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    set_sev_socketopt();
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(_sev_data->_port);
    
    if (bind(_sev_data->_listen_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) 
    {
        std::cout << "bind sock: " << strerror(errno) << std::endl;
        close(_sev_data->_listen_socket);
        return -1;    
    }
    
    return 0;
}

int NfSvr::nf_server_listen()
{
    nf_server_t * sev = _sev_data;
    int backlog = sev->_backlog;

    if(backlog <= 0)
        backlog = 5;

    if(listen(sev->_listen_socket, backlog) < 0)
    {
        Log :: ERROR("SET LISTEN SOCKET ERROR");
        close(sev->_listen_socket);
        return -1;
    }
    Log :: NOTICE("SET LISTEN SOCKET SUCC");
    
    return 0;
}

