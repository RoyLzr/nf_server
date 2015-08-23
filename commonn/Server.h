/*
 * brief nf_server
 * server Àà
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
         * @brief ÔËÐÐ·þÎñÆ÷  
         * @return  int 0 ³É¹¦ -1 Ê§°Ü   
         * @author liuzhaorui
         *    
        **/
        
        virtual int stop() = 0;
        /**
        * @brief Ô stop server
        * @return  int 0 ³É¹¦ -1 Ê§°Ü   
        * @author liuzhaorui
        *    
        **/
        
        virtual int destroy() = 0; 
        /**
        * @brief Ô destroy server
        * @return  int 0 ³É¹¦ -1 Ê§°Ü   
        * @author liuzhaorui
        *    
        **/
        
        virtual int join() = 0;
        /**
        * @brief Ô join server
        * @return  int 0 ³É¹¦ -1 Ê§°Ü   
        * @author liuzhaorui
        *    
        **/
        
        virtual int pause() = 0;
        /**
        * @brief pause server
        * @return  int 0 ³É¹¦ -1 Ê§°Ü   
        * @author liuzhaorui
        *    
        **/
        
        virtual int resume() = 0;
        /**
        * @brief  server
        * @return  int 0 ³É¹¦ -1 Ê§°Ü   
        * @author liuzhaorui
        *    
        **/
};

#endif
