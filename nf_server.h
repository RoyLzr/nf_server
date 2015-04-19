/*
 * brief nf_server
 * server 类
 * author liuzhaorui
 * email  liuzhaorui1@163.com
 */


#ifndef _NF_SERVER_H
#define _NF_SERVER_H

#include "net.h"
#include <string>
#include "nf_server_core.h"
#include <iostream>
#include <string.h>
#include "commonn/singleton.h"
#include "commonn/configParser.h"

namespace nf
{
    class NfServer
    {
        public:
            NfServer();
            virtual ~NfServer();
        //测试 暂时 使用 手动设置配置文件
        
        int load_conf(const std::string &);

        int run();
        /**
          * @brief 运行nf服务器  
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        int stop();
        /**
          * @brief � stop server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        int destroy(); 
        /**
          * @brief � destroy server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        int join();
        /**
          * @brief � join server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        int pause();
        /**
          * @brief pause server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
        
        int resume();
        /**
          * @brief 重启 server
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/
         
        int set_work_callback(nf_callback_proc run);
        /**
          * @brief callback 函数
          * @return  int 0 成功 -1 失败   
          * @author liuzhaorui
          *    
          **/

        int set_server_startfun(nf_handle_t start);
        
        int set_thread_startfun(nf_handle_t start); 
        
        int set_thread_endfun(nf_handle_t end); 
   
        int set_server_name(const char *);
        
        nf_server_t * get_server_data();
 
    protected:
        nf_server_t *sev_data;
            
    };
}

#endif
