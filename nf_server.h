//**********************************************************
//          Nf_server 1.0
//
//  Description:
//  nf_server 封装类，对底层server服务封装
//
// Author: Liu ZhaoRui
//         liuzhaorui1@163.com
//**********************************************************

#ifndef _NF_SERVER_H
#define _NF_SERVER_H

#include <string>
#include "nf_server_core.h"
#include "nf_server_app.h"
#include "pool_register.h"
#include "commonn/Server.h"
#include <iostream>
#include <string.h>

namespace nf
{
    class NfServer
    {
        public:
            NfServer();
            virtual ~NfServer();
        
        virtual int load_conf(const std::string );

        virtual int run();
        /**
          * @brief 运行nf服务器  
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        virtual int stop();
        /**
          * @brief � stop server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        virtual int destroy(); 
        /**
          * @brief � destroy server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        virtual int join();
        /**
          * @brief � join server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        virtual int pause();
        /**
          * @brief pause server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        virtual int resume();
        /**
          * @brief 重启 server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
         
        virtual int set_work_callback(BaseWork *run);
        /**
          * @brief callback 函数
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/

        virtual int set_server_startfun(nf_handle_t start);
        
        virtual int set_thread_startfun(nf_handle_t start); 
        
        virtual int set_thread_endfun(nf_handle_t end); 
        
        int set_server_name(const char *);
    
        virtual int set_handle( nf_handle_t handle ); 
        
        nf_server_t * get_server_data();
 
    protected:
        nf_server_t *sev_data; //server 核心数据结构
            
    };
}

#endif
