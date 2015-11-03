//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  nf_server ·â×°Àà£¬¶Ôµ×²ãserver·þÎñ·â×°
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
         * @brief ÔËÐÐnf·þÎñÆ÷  
         * @return  int 0 ³É¹¦ -1 Ê§°Ü   
         * @author liuzhaorui
         *    
         **/

        int stop();
        /**
         * @brief Ô stop server
         * @return  int 0 ³É¹¦ -1 Ê§°Ü   
         * @author liuzhaorui
         *    
         **/

        int destroy(); 
        /**
         * @brief Ô destroy server
         * @return  int 0 ³É¹¦ -1 Ê§°Ü   
         * @author liuzhaorui
         *    
         **/

        int join();
        /**
         * @brief Ô join server
         * @return  int 0 ³É¹¦ -1 Ê§°Ü   
         * @author liuzhaorui
         *    
         **/

        int pause();
        /**
         * @brief pause server
         * @return  int 0 ³É¹¦ -1 Ê§°Ü   
         * @author liuzhaorui
         *    
         **/

        int resume();
        /**
         * @brief ÖØÆô server
         * @return  int 0 ³É¹¦ -1 Ê§°Ü   
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

        nf_server_t *sev_data; //server ºËÐÄÊý¾Ý½á¹¹

};

#endif
