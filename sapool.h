//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  sync and reactor model
//  
//  listen thread ʹ��reactor model, ������ӳ����������� +
//  listen fd, ����ص����¼����� �¼����� ȫ�̲���������
//
//  work thread ��ȡ�¼��������ݽ��д���read event ȫ�̷�����
//  �����������������㴦���ʽʱ�� �����ڴ�ط���ռ䣬����״̬��
//  �ȴ��´ζ��ĵ������ָ��ϴ�״̬�� д�¼����� ���� + ��ʱ����
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

