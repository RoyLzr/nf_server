#ifndef  __LOCK_H_
#define  __LOCK_H_
#include <sys/time.h>
#include <pthread.h>

class MLock
{
    public:
        inline MLock() 
        {
            pthread_mutex_init(&_lock, NULL);
        }
        inline ~MLock() 
        {
            pthread_mutex_destroy(&_lock);
        }
        inline int lock() 
        {
            return pthread_mutex_lock(&_lock);
        }
        inline int unlock() 
        {
            return pthread_mutex_unlock(&_lock);
        }
        inline int trylock() 
        {
            return pthread_mutex_trylock(&_lock);
        }

    private:
        pthread_mutex_t _lock;

        friend class MCondition;
};


class SpinLock
{
    public:
        inline SpinLock() 
        {
            pthread_spin_init(&_lock, 0);
        }
        inline ~SpinLock() 
        {
            pthread_spin_destroy(&_lock);
        }
        inline int lock() 
        {
            return pthread_spin_lock(&_lock);
        }
        inline int unlock() 
        {
            return pthread_spin_unlock(&_lock);
        }
        inline int trylock() 
        {
            return pthread_spin_trylock(&_lock);
        }

    private:
        pthread_spinlock_t _lock;
};

template <typename Lock>
class AutoLock
{
    public:
        inline AutoLock(Lock &l) : _lock(l) 
        {
            l.lock();
        }
        inline ~AutoLock() 
        {
            _lock.unlock();
        }
    private:
        Lock & _lock;
};

class MCondition
{
    public:
        MCondition(MLock &l) : _lock(l._lock), _waits(0) 
        {
            pthread_cond_init(&_cond, NULL);
        }

        MCondition(pthread_mutex_t &l) : _lock(l), _waits(0) 
        {
            pthread_cond_init(&_cond, NULL);
        }

        ~MCondition() 
        {
            pthread_cond_destroy(&_cond);
        }

        inline int wait(const timeval *tv) 
        {
            ++_waits;
            int ret = 0;
            if (tv) 
            {
                timespec ts;
                ts.tv_sec = tv->tv_sec;
                ts.tv_nsec = tv->tv_usec * 1000;
                ret = pthread_cond_timedwait(&_cond, &_lock, &ts);
                --_waits;
                return ret;
            }
            ret = pthread_cond_wait(&_cond, &_lock);
            --_waits;
            return ret;
        }

        inline void signal() 
        {
            pthread_cond_signal(&_cond);
        }

        inline void signalAll() 
        {
            pthread_cond_broadcast(&_cond);
        }

        inline size_t waits() { return _waits; }

    private:

        pthread_cond_t _cond;
        pthread_mutex_t & _lock;
        size_t _waits;
};

#endif 

