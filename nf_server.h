//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  nf_server ��װ�࣬�Եײ�server�����װ
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef _NF_SERVER_H
#define _NF_SERVER_H

#include <string>
#include "nf_server_core.h"
#include "commonn/Server.h"
#include <iostream>
#include <string.h>

class NfServer
{
    public:
        NfServer();
        virtual ~NfServer();

        virtual int load_conf(const std::string );

        virtual int run();
        /**
         * @brief ����nf������  
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        virtual int stop();
        /**
         * @brief � stop server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        virtual int destroy(); 
        /**
         * @brief � destroy server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        virtual int join();
        /**
         * @brief � join server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        virtual int pause();
        /**
         * @brief pause server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        virtual int resume();
        /**
         * @brief ���� server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        virtual int set_work_callback(BaseWork *run);
        /**
         * @brief callback ����
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        virtual int svr_init(nf_server_t *) = 0;

        virtual int svr_run(nf_server_t *) = 0;

        virtual int svr_join(nf_server_t *) = 0;

        virtual int svr_listen(nf_server_t *) = 0;

        virtual int svr_destroy(nf_server_t *) = 0;

        virtual int svr_pause(nf_server_t *) = 0;

        virtual int svr_resume(nf_server_t *) = 0;

        virtual int svr_set_stragy(nf_server_t *, BaseWork *) = 0;

        virtual int set_server_startfun(nf_handle_t start);

        virtual int set_thread_startfun(nf_handle_t start); 

        virtual int set_thread_endfun(nf_handle_t end); 

        int set_server_name(const char *);

        virtual int set_handle( nf_handle_t handle ); 

        nf_server_t * get_server_data();

    protected:
        nf_server_t *sev_data; //server �������ݽṹ

};

#endif
