/*
 * brief nf_server
 * server ��
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

namespace nf
{
    class NfServer
    {
        public:
            NfServer();
            virtual ~NfServer();
        //���� ��ʱ ʹ�� �ֶ����������ļ�
        //int load_conf(const string &);
        
        int load_conf();

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
         
        int set_work_callback(nf_callback_proc run);
        /**
          * @brief callback ����
          * @return  int 0 �ɹ� -1 ʧ��   
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
