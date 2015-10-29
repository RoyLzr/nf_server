#include <stdlib.h>
#include <stdio.h>
#include "ThreadPool.h"

static pthread_key_t pkey;
static pthread_once_t ponce = PTHREAD_ONCE_INIT; 

CThreadNotify :: CThreadNotify()
{
    pthread_mutexattr_init(&m_mutexattr);
    pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
    
    pthread_mutex_init(&m_mutex, &m_mutexattr);
    pthread_cond_init(&m_cond, NULL); 
}

CThreadNotify :: ~CThreadNotify()
{
    pthread_mutexattr_destroy(&m_mutexattr);
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

CWorkerThread::CWorkerThread()
{
	m_task_cnt = 0;
    m_work_len = 0;
    m_work_buff = NULL;
}

CWorkerThread::~CWorkerThread()
{
    if(m_work_buff != NULL)
    {
        free(m_work_buff);
        m_work_buff = NULL;
    }
}

void* CWorkerThread::StartRoutine(void* arg)
{
	CWorkerThread* pThread = (CWorkerThread*)arg;
    
    set_pthread_data(pThread);
	
    pThread->Execute();

	return NULL;
}

void CWorkerThread::Start()
{
	(void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
}

void CWorkerThread::Execute()
{
	while (true) 
    {
		m_thread_notify.Lock();

		// put wait in while cause there can be spurious wake up (due to signal/ENITR)
		while (m_task_list.empty()) 
        {
			m_thread_notify.Wait();
		}

		CTask* pTask = m_task_list.front();
		m_task_list.pop_front();
		m_thread_notify.Unlock();

		pTask->run();

		delete pTask;

		m_task_cnt++;
	}
}

void CWorkerThread::PushTask(CTask* pTask)
{
	m_thread_notify.Lock();
	m_task_list.push_back(pTask);
	m_thread_notify.Signal();
	m_thread_notify.Unlock();
}

//////////////
CThreadPool::CThreadPool()
{
	m_worker_size = 0;
	m_worker_list = NULL;
}

CThreadPool::~CThreadPool()
{
    Destory();
}

int CThreadPool::init(int worker_size, int buff_size)
{
    m_worker_size = worker_size;
	m_worker_list = new CWorkerThread [m_worker_size];
	if (!m_worker_list) 
    {
		return 1;
	}

    if(buff_size <= 0)
    {
        for (int i = 0; i < m_worker_size; i++) 
        {
            m_worker_list[i].SetThreadIdx(i);
            m_worker_list[i].Start();
        }
    }
    else
    {
        for (int i = 0; i < m_worker_size; i++) 
        {
            m_worker_list[i].SetThreadIdx(i);
            m_worker_list[i].SetBuff(buff_size);
            m_worker_list[i].Start();
        }
    }
	return 0;
}

void CThreadPool::Destory()
{
    if(m_worker_list)
        delete [] m_worker_list;
}

void CThreadPool::AddTask(CTask* pTask)
{
	/*
	 * select a random thread to push task
	 */
	int thread_idx = pTask->idx % m_worker_size;
	m_worker_list[thread_idx].PushTask(pTask);
}


void 
create_key_once(void)
{
    pthread_key_create(&pkey, NULL);
}

void 
pthread_key_del(void)
{
    pthread_key_delete(pkey);
}

int 
set_pthread_data(CWorkerThread * data)
{
    void *ptr = NULL;
    pthread_once(&ponce, create_key_once);
    if ((ptr = pthread_getspecific(pkey)) == NULL) 
    {
    #ifndef WORK
        printf("data : %d \n", data->GetThreadIdx());
    #endif
        ptr = data;
        pthread_setspecific(pkey, ptr);
    }
    return 0;
}

CWorkerThread *
get_pthread_data()
{
    void * ptr = pthread_getspecific(pkey);
    #ifndef WORK
        if(ptr == NULL)
            printf("EMPTY THREAD data \n");
    #endif
    return (CWorkerThread *) ptr;
}


