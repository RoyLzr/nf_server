#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <pthread.h>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../util.h"

using namespace std;

class CTask 
{
    public:
        friend class CThreadPool;
        CTask()
        {
            idx = random();
        }
        void setTaskIdx(int num)
        {
            idx = num;
        }
        virtual ~CTask(){}
        virtual void run() = 0;
       
    protected:
        int idx;
};


class CThreadNotify
{
public:
	CThreadNotify();
	~CThreadNotify();
	void Lock() 
    { 
        pthread_mutex_lock(&m_mutex); 
    }
	void Unlock() 
    { 
        pthread_mutex_unlock(&m_mutex); 
    }
	void Wait() 
    { 
        pthread_cond_wait(&m_cond, &m_mutex); 
    }
	void Signal() 
    { 
        pthread_cond_signal(&m_cond); 
    }
private:
	pthread_mutex_t 	m_mutex;
	pthread_mutexattr_t	m_mutexattr;
    
	pthread_cond_t 		m_cond;
};

class CWorkerThread 
{
    public:
	    CWorkerThread();
	    ~CWorkerThread();

	    static void* StartRoutine(void* arg);

	    void Start();
	    void Execute();
	    void PushTask(CTask* pTask);

	    void SetThreadIdx(int idx) 
        { 
            m_thread_idx = idx; 
        }
	    
        int GetThreadIdx() 
        { 
            return m_thread_idx; 
        }
        
        void * GetWorkerBuff()
        {
            return m_work_buff;
        }
        
        int GetWorkerLen()
        {
            return m_work_len;
        }

        void SetBuff(int size)
        {
            m_work_buff = malloc(size);
            memset(m_work_buff, '\0', size);
            if(m_work_buff != NULL)
               m_work_len = size; 
        }

    private:

	    int     		m_thread_idx;
	    int		        m_task_cnt;
	    pthread_t		m_thread_id;
	    CThreadNotify	m_thread_notify;
	    list<CTask*>	m_task_list;
        void *          m_work_buff;
        int             m_work_len;
};

class CThreadPool 
{
    public:
	    CThreadPool();
	    virtual ~CThreadPool();

	    int init(int worker_size, int buff_size = 0);
	    int AddTask(CTask* pTask);
        void Stop();
	    void Destory();
    private:
	    int      		m_worker_size;
	    CWorkerThread* 	m_worker_list;
};


int
set_pthread_data(CWorkerThread * data);

CWorkerThread *
get_pthread_data();

#endif
