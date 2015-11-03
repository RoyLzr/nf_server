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
        
        int init(const string & log_path);

        int run();
        /**
         * @brief ����nf������  
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        int stop();
        /**
         * @brief � stop server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        int destroy(); 
        /**
         * @brief � destroy server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        int join();
        /**
         * @brief � join server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        int pause();
        /**
         * @brief pause server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        int resume();
        /**
         * @brief ���� server
         * @return  int 0 �ɹ� -1 ʧ��   
         * @author liuzhaorui
         *    
         **/

        int set_read_handle(ev_handle );

        int set_write_handle(ev_handle );

        int set_parse_read_handle(parse_handle );

        int set_parse_write_handle(parse_handle );
         
        virtual int svr_init();

        virtual int svr_run();

        virtual int svr_join();

        /*
        virtual int svr_listen();

        virtual int svr_destroy();

        virtual int svr_pause();

        virtual int svr_resume();
        */

        int set_server_name(const char *);

        nf_server_t * get_server_data();

    protected:
         
        nf_server_t * 
        nf_server_create(const char * sev_name);

        nf_server_t *sev_data; //server �������ݽṹ

};

#endif
