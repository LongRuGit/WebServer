#ifndef LOCKER_H
#define LOCKER_H

#include<exception>
#include<pthread.h>
#include<semaphore.h>

//封装信号的类
class sem
{
public:
    //创建并初始化信号量
    sem()
    {
        if(sem_init(&m_sem,0,0)!=0)
        {
            //構造函數沒有返回值,可以靠拋出異常來報告錯誤
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    //等待信號量
    bool wait()
    {
        return sem_wait(&m_sem)==0;
    }
    //增加信號量
    bool post()
    {
        return sem_post(&m_sem)==0;
    }
private:
    sem_t m_sem;
};

//封裝互斥鎖的類
class locker
{
private:
    pthread_mutex_t m_mutex;
public:
    locker()
    {
        if(pthread_mutex_init(&m_mutex,NULL)!=0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    //獲得互斥鎖
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex)==0;
    }
    //釋放互斥鎖
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex)==0;
    }
};

//封装条件变量的类
class cond
{
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
public:
    cond()
    {
        if(pthread_mutex_init(&m_mutex,NULL)!=0)
        {
            throw std::exception();
        }
        if (pthread_cond_init(&m_cond,NULL)!=0)
        {
            //构造函数出现问题就应该立即释放已经分配的资源
            pthread_mutex_unlock(&m_mutex);
            throw std::exception();
        }
    }
    ~cond() 
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }
    //等待条件变量
    bool wait()
    {
        int ret=0;
        pthread_mutex_lock(&m_mutex);
        ret=pthread_cond_wait(&m_cond,&m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret==0;
    }
    //唤醒等待条件变量的线程
    bool signal()
    {
        return pthread_cond_signal(&m_cond)==0;
    }
};


#endif