#include <sys/epoll.h>
#include "pool_register.h"
#include <pthread.h>
#include "commonn/queue.h"

enum{
    IDLE = 0,
    READY,
    BUSY
};

typedef struct _sapool_sock_item_t
{
	int status;
	
	int sock;
	time_t last_active;
	struct sockaddr_in addr;
	// 与client建立的socket链接有请求到来的时间点;
	//当可读时会将此socket放到队列中等待工作（消费）线程处理；
	unsigned long long int enqueue_time;
} sapool_sock_item_t;

typedef struct _sapool_t
{
	sapool_sock_item_t *sockets;
	struct epoll_event *events;
	size_t size;
    long long using_size;

	int epfd;       //池中一个epfd, 监控池中所有连接，
                    //svr 中的listen_fd 也用这个epfd

	size_t timeout;	//epoll的超时时间
	size_t _check_interval;
	//微妙级别支持超时
	time_t _next_check_time;

	int *run;
	int sev_sock_id;

	queue_t queue;
	pthread_mutex_t ready_mutex;
	pthread_cond_t ready_cond;

	pthread_t main; //pool 中监控事件发生的线程
                    //监控 sockets 及 listen fd
                    //发生事件. 存到queue, work
                    //线程处理
} sapool_t;


int sapool_init(nf_server_t * sev)
{
    std::cout << "init normal" << std::endl;
    return 0;
}

//启动线程池内线程
int sapool_run(nf_server_t * sev)
{
    std::cout << "run normal" << std::endl;
    return 0; 
}

int sapool_join(nf_server_t * sev)
{

    std::cout << "join normal" << std::endl;
    return 0;
}

int sapool_listen(nf_server_t * sev)
{

    std::cout << "listen normal" << std::endl;
    return 0;
}

int sapool_destroy(nf_server_t * sev)
{
    std::cout << "destroy normal" << std::endl;
    return 0;
}

long long sapool_get_socknum(nf_server_t *)
{
    return 0;
}

long long sapool_get_queuenum(nf_server_t *)
{
    return 0;
}

int sapool_pause(nf_server_t *)
{
    return 0;
}

int sapool_resume(nf_server_t *)
{
    return 0;
}
