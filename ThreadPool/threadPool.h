#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_
#include <pthread.h>

//线程池结构体
struct Threadpool
{
    //线程池中的线程
    pthread_t *threadId;

    //最小线程数
    int minThreads;

    //最大线程数
    int maxThreads;
};

#endif