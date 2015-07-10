# nf_server

轻量级网络服务编程框架

#支持操作:

TCP  IPV4通信，全部基于线程操作，提升为实时进程调度，需要root权限运行。

#支持模式: 


线程模型： leader-follower

每线程管理一个连接，试用与短链接、快速连接


Half reactor/Half sync 模型

main 线程负责启动所有工作线程，启动成功后负责外部发送信号。

一个reactor 线程负责监听连接池里所有连接及等待连接的socket, 若监听socket 有事件发生，则注册事件到循环事件队列中。

同步worker 线程负责从循环事件队列中获取事件，执行相应操作。


Reactor + 多线程模型

main 线程负责启动所有工作线程，启动成功后负责外部发送信号。

一个reactor 线程负责监等待连接的socket, 若监听socket 有事件发生，则accept 连接到的socket 并执行： 
1. 注册该socket 到连接池中
2. 负载均衡，将该socket 注册到一个压力较小的子reactor 中

其余worker 线程 为子reactor， 每一个子reactor 维护 监听 reactor注册过来的事件。


#介绍：


（1）多线程模型

（2）支持长连接、短连接

（3）多种的连接池管理模式：lfpool、sapool(未开发)

（4）采用回调方式，开发者只需关心数据处理逻辑部分

（5）快速的开发网络服务


#目的：


托管与client端的连接管理、内存管理、数据的读取和发送。应用方只需要设置读取数据后，进行操作的回调函数，

发送的回调函数，集中于业务逻辑处理。


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

