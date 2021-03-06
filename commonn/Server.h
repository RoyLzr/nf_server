/*
 * brief nf_server
 * server 类
 * author liuzhaorui
 * email  liuzhaorui1@163.com
 */

#ifndef _SERVER_H 
#define _SERVER_H
 
#include <string>

class Server
{
    protected:
        Server(){};
    
        virtual ~Server(){};
    
    public: 

        virtual int run() = 0;
        /**
         * @brief 运行服务器  
         * @return  int 0 成功 -1 失败   
         * @author liuzhaorui
         *    
        **/
        
        virtual int stop() = 0;
        /**
        * @brief � stop server
        * @return  int 0 成功 -1 失败   
        * @author liuzhaorui
        *    
        **/
        
        virtual int destroy() = 0; 
        /**
        * @brief � destroy server
        * @return  int 0 成功 -1 失败   
        * @author liuzhaorui
        *    
        **/
        
        virtual int join() = 0;
        /**
        * @brief � join server
        * @return  int 0 成功 -1 失败   
        * @author liuzhaorui
        *    
        **/
        
        virtual int pause() = 0;
        /**
        * @brief pause server
        * @return  int 0 成功 -1 失败   
        * @author liuzhaorui
        *    
        **/
        
        virtual int resume() = 0;
        /**
        * @brief  server
        * @return  int 0 成功 -1 失败   
        * @author liuzhaorui
        *    
        **/
};

#endif
