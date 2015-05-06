#ifndef _singleton_h
#define _singleton_h

#include <vector>
#include <map>
#include <pthread.h>

class MutexLock
{
    public:
        MutexLock()
        {
            pthread_mutex_init(&mutex, NULL);
        }
        ~MutexLock()
        {
            pthread_mutex_destroy(&mutex);
        }
        void lock()
        {
            pthread_mutex_lock(&mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&mutex);
        }
    private:
        pthread_mutex_t mutex;
};

template <class T>
class Singleton
{
    public:
        static T * instance()
        {
            if (!_initialized)
            {
                Lock.lock();
                if (!_initialized)
                {
                    _instance = new T();
                    _initialized = true;
                }
                Lock.unlock();
            }
            return _instance;
        }
  
        static void destroy()
        {
            if(_instance != NULL)
            {
                delete _instance;
            } 
            _instance = NULL;       
        }

    private:
        static T * _instance;
        static bool _initialized;
        static MutexLock Lock;
};

template<class T>
bool Singleton<T>::_initialized = false;

template<class T>
T * Singleton<T>::_instance = NULL;

template<class T>
MutexLock Singleton<T>::Lock;

#endif
