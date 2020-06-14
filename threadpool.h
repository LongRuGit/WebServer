#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

template<typename T>
class threadpool
{
public:
    //參數thread_number是線程池中線程的數量,max_requests是請求隊列中最多允許的等待處理的請求的數量
    threadpool(int iThread_number=8,int iMax_requests=10000);
    ~threadpool();
    //往請求隊列中添加任務
    bool append(T * pRequest);
    //工作线程运行的函数,它不断从请求队列中取出请求并运行
    static void * worker(void * arg);
    void run();  
private:
    int m_thread_numer;              //線程池中的線程數
    int m_max_requests;              //請求隊列中的最大請求數
    pthread_t *   m_pThreads;        //描述線程池的數組,其大小爲m_thread_numer
    std::list<T*> m_workqueue;       //請求隊列
    locker        m_queuelocker;     //保護請求隊列的互斥鎖
    sem           m_queuestat;       //是否有任務需要處理
    bool          m_stop;            //是否結束線程
};

template<typename T>
threadpool<T>::threadpool(int iThread_number,int iMax_requests)
:m_thread_numer(iThread_number),m_max_requests(iMax_requests),m_pThreads(NULL),m_stop(false)
{
    if(iThread_number<=0||iMax_requests<=0)
        throw std::exception();
    
    m_pThreads=new pthread_t(m_thread_numer);
    if(!m_pThreads)
    {
        throw std::exception();
    }

    //创建thread_number个线程,并将它们都设置为脱离线程
    for(int i=0;i<m_thread_numer;++i)
    {
        printf("create tthe %d th thread\n",i);
        //该参数的第三个参数必须指向一个静态对象
        if(pthread_create(m_pThreads+i,NULL,worker,this)!=0)
        {
            delete [] m_pThreads;
            throw std::exception();
        }
        if(pthread_detach(m_pThreads[i]))
        {
            delete [] m_pThreads;
            throw std::exception();
        }
    }
}

template<typename T>
threadpool<T>::~threadpool()
{
    delete [] m_pThreads;
    m_stop=true;
}

template<typename T>
bool threadpool<T>::append(T * pRequest)
{
    //操作工作队列是一定要加锁,因为它被所有的线程共享
    m_queuelocker.lock();
    if(m_workqueue.size()>m_max_requests)
    {
        //已經超過最大請求數量,等待即可
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(pRequest);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template<typename T>
void * threadpool<T>::worker(void * arg)
{
    //在静态函數中使用动态的方法
    threadpool * poolCur=(threadpool *) arg;
    poolCur->run();
    return poolCur;
}

template<typename T>
void threadpool<T>::run()
{
    while (!m_stop)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T *request=m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if (!request)
        {
            continue;
        }
        request->process();
    }
}

#endif