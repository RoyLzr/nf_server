//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  sync and reactor model
//  
//  listen thread 使用reactor model, 监控连接池内所有连接 +
//  listen fd, 将监控到的事件放入 事件队列 全程操作非阻塞
//
//  work thread 获取事件队列数据进行处理，read event 全程非阻塞
//  ，读到数据量不满足处理格式时， 利用内存池分配空间，保存状态，
//  等待下次读的到来，恢复上次状态。 写事件采用 阻塞 + 超时处理
//           
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef  __SAPOOL_H_
#define  __SAPOOL_H_

#include "nf_server_core.h"
#include "commonn/queue.h"
#include "nf_server.h"

#define LISTENER_PRIORITY    10
#define WORKER_PRIORITY        5 

class SaServer : public NfServer
{
    public:
        SaServer() : NfServer()
        {};

        virtual ~SaServer(){};
    
    protected:

        virtual int svr_init();

        virtual int svr_run();
        
        /*
        virtual int svr_destroy();

        virtual int svr_pause();

        virtual int svr_resume();
        */
};

class SaListenEvent : public Event
{
    public:
        nf_server_t  * get_sev()
        {
            return sev;
        }

    explicit SaListenEvent() : Event()
    {} 

    inline void init(nf_server_t * sev)
    {
        ev_fd = sev->sev_socket;
        ev_events = EV_READ;
        ev_active = 0;
        ev_flags = EV_INIT;
        ev_callback = NULL;
    }

    private:
        nf_server_t * sev;
};

#endif  

