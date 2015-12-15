
//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  server/thread 核心数据结构
//
//  核心server调用函数， server 类对该核心库的封装
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef _NFSERVER_CORE_H
#define _NFSERVER_CORE_H


#include <atomic>
#include "net.h"
#include "commonn/singleton.h"
#include "commonn/configParser.h"
#include "commonn/memCache.h"
#include "interface/ireactor.h"
#include "interface/ievent.h"
#include "NfUnit/baseEvent.h"
#include "util.h"

typedef struct _nf_server_t nf_server_t;

class NfSvr;

struct _nf_server_t
{
    size_t _svr_type;

    size_t _backlog;
    size_t _port;
    size_t _connect_to;
    size_t _read_to;
    size_t _write_to;
    size_t _session_to;

    size_t            _max_session;
    std::atomic<int>  _session_cnt;

    int         _sock_family;
    int         _listen_socket; 
    size_t      _sockopt;
   
    IReactor        * _svr_reactor;
    SockEventBase   * _acc_event;

    int _status;
    
};

class NfSvr : private Uncopyable
{
    public:
    enum
    {
        SYNC,
        ASYNC,
    };
    enum
    {
        LINGER       = 0x01,
        NODELAY      = 0x02,
        DEFER_ACC    = 0x04,

    };
    enum
    {
        INIT,
        RUNNIING,
        JOIN,
        PAUSE,
        STOP,
    };

    public:
        NfSvr();
        NfSvr(IReactor *);
        virtual ~NfSvr();
    
        int init(const Section &);

        int run();

        int stop();
        
        int join();

        int pause();

        int resume();
        
        int destroy();

        const nf_server_t * get_svr_data() const
        {
            return _sev_data; 
        }
        
        void set_sev_name(string &name)
        {
            _name = name;
        }

        void set_reactor(IReactor * act)
        {
            _sev_data->_svr_reactor = act;
        }

        void set_acc_event(SockEventBase * ev)
        {
            _sev_data->_acc_event = ev;
        }

    protected:
        
        nf_server_t * nf_server_create();

        int nf_server_listen();
        
        int nf_server_bind();

        int set_sev_socketopt();

        nf_server_t * _sev_data;
        std::string   _name;
};

#endif
