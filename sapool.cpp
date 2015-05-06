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
	// ��client������socket��������������ʱ���;
	//���ɶ�ʱ�Ὣ��socket�ŵ������еȴ����������ѣ��̴߳���
	unsigned long long int enqueue_time;
} sapool_sock_item_t;

typedef struct _sapool_t
{
	sapool_sock_item_t *sockets;
	struct epoll_event *events;
	size_t size;
    long long using_size;

	int epfd;       //����һ��epfd, ��س����������ӣ�
                    //svr �е�listen_fd Ҳ�����epfd

	size_t timeout;	//epoll�ĳ�ʱʱ��
	size_t _check_interval;
	//΢���֧�ֳ�ʱ
	time_t _next_check_time;

	int *run;
	int sev_sock_id;

	queue_t queue;
	pthread_mutex_t ready_mutex;
	pthread_cond_t ready_cond;

	pthread_t main; //pool �м���¼��������߳�
                    //��� sockets �� listen fd
                    //�����¼�. �浽queue, work
                    //�̴߳���
} sapool_t;


int sapool_init(nf_server_t * sev)
{
    std::cout << "init normal" << std::endl;
    return 0;
}

//�����̳߳����߳�
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
