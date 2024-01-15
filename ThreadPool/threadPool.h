#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_
#include <pthread.h>

//任务队列
struct task_t
{
    void *(*worker_hander)(void *arg);
    //参数
    void *arg;
};
typedef struct task_t task_t;

//线程池结构体
struct Threadpool
{
    //任务队列
    task_t *taskQueue;
    //任务队列容量
    int queueCapacity;
    //任务队列的任务数
    int queueSize;
    //任务队列的队头 队尾
    int queueFront;
    int queueRear;
    
    //线程池中的线程
    pthread_t *threadId;

    //最小线程数
    int minThreads;

    //最大线程数
    int maxThreads;
};
typedef struct Threadpool Threadpool;


//线程池的初始化
int threadPoolInit(Threadpool *pool, int minThreads, int maxThreads, int queueCapacity);

//线程池销毁
int threadPoolDestroy(Threadpool *pool);

#endif