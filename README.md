NF_SERVER  (轻量级网络服务编程框架)
===================================  

支持操作:
-----------------------------------  

TCP  IPV4通信，全部基于线程操作，提升为实时进程调度，需要root权限运行。

支持模式: 
-----------------------------------  


### 线程模型 leader-follower

      （1）每线程管理一个连接，适用与短链接、快速连接
      （2）采用超时 + 阻塞 进行读写


### Half reactor/Half sync 模型

      (1) main 线程负责启动所有工作线程，启动成功后负责监听外部发送的信号。

      (2) 一个reactor 线程负责监听连接池里所有连接及等待连接的socket
      
         1. 若等待连接socket有事件发生，则注册该socket 到连接池中，置READY状态。
         2. 若连接池中READY状态，则将连接注册到循环事件队列中，置BUSY状态。清理发生状态fd的监控。
         
      (3)worker 线程负责从循环事件队列中获取事件，执行相应操作。结束后，连接池中状态置READY，回设
         监听reactor  对该fd 的监控

     （4）非阻塞读，阻塞+超时写。 内存池管理非阻塞读时，需要保持状态的数据。
      
     （5）比较均衡的模式，长短连接、CPU频繁、IO频繁操作均可使用


### Reactor + 多线程模型

       (1) main 线程负责启动所有工作线程，启动成功后负责监听外部发送的信号。

       (2)一个reactor 线程负责监等待连接的socket, 若监听socket 有事件发生，则accept，并执行： 
      
         1. 注册该socket 到连接池中，置BUSY状态。
         2. 负载均衡，将该socket 注册到一个压力较小的子reactor。

      （3）其余worker线程为子reactor， 每一个子reactor 维护监听 reactor注册过来的事件。

      （4）全程非阻塞读写，不会发生等待现象，最大限度利用CPU。 内存池管理非阻塞读，写时，需要保持状态的数据
      
      （5）适用于IO频繁操作。注册运行的回调函数时，主要不要有阻塞，等待操作。 


框架使用：
-----------------------------------  

### 目的

      （1）托管与client端的连接管理、内存管理、数据的读取和发送， LOG读写等。
      
      （2）应用方只需要设置回调函数，从指定内存中读数据，业务处理，存储数据到发送内存中。
          框架自动进行读写。
  
### simple echo example        
      void
      nf_default_handle()
      {
           char * read_buf = (char *) nf_server_get_read_buf();
           char * write_buf = (char *) nf_server_get_write_buf();
           
           int readed_size = nf_server_get_readed_size();
           strncpy(write_buf, read_buf, readed_size);
           
           nf_server_set_writed_size(readed_size);
           nf_server_set_writed_start(readed_size);
      }
      


#基本架构：

                      server_create(构造)    server_run         server_join    server_destroy
                          _____________________|_____               |               |                           
                          |                         |               |               |      

                        pool_init              pool_run         pool_join       pool_destroy                
                                                  |

                                                worker（用户可自己设置回调，默认读一次，写一次）
                                           _______|________
                                           |               |
                                          p_write()         p_read（）（用户可自己设置回调）     
                                          
                                          
                                          
#文件目录：
nf_server

   --common
   
       -configParser.h (一个simple conf 文件解析类， sectio格式[], item格式key = value)
       
       -configParser.cpp
       
       -Server.h (server 抽象类）
       
       -singleton.h（单例模板）
   -net.h   （公共socket 网络操作封装，适用于非阻塞）
   
   -net.cpp
  
   -nf_server.h   （实现server 抽象类）
   
   -nf_server.cpp
   
   -nf_server_core.h   (nf_server 核心数据结构及函数声明）
   
   -nf_server_core.cpp
   
   -pool_register.h    (pool 管理， 目前只有lfpool）
   
   -lfpool.cpp  (leader-follower 模型核心实现)

