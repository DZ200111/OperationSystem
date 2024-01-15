#include "threadPool.h"
#include <errno.h>
#include <stdio.h>

#define DEFAULT_MIN 10
#define DEFAULT_MAX 20
#define DEFAULT_QUEUE_CAPACITY 100

enum STATUSCODE
{
    ON_SUCCESS,
    NULL_PTR,
    MALLOC_ERROR,
    ACCESS_INVALID,
    UNKNOWN_ERROR,
};

void * threadHander(void *arg)
{
    Threadpool *pool = (Threadpool *)arg;
    while(1)
    {
        pthread_mutex_lock(&(pool->mutexpool));
        while(pool->queueSize == 0)
        {
            //等待一个条件变量
            pthread_cond_wait(&(pool->notEmpty), &(pool->mutexpool));
        }
        //意味着任务队列有任务

        task_t tmpTask = pool->taskQueue[pool->queueFront];
        pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;

        //任务数减一
        pool->queueSize--;
        pthread_mutex_unlock(&(pool->mutexpool));

        //发一个信号给生产者可以继续生产
        pthread_cond_singal(&pool->notFull);

        //为了提升我们的性能，再创建一把锁
        pthread_mutex_lock(&(pool->mutexbusy));
        pool->busyThreadNums++;
        pthread_mutex_unlock(&(pool->busyThreadNums));

        //执行钩子函数
        tmpTask.worker_hander(tmpTask.arg);

        pthread_mutex_lock(&(pool->mutexbusy));
        pool->busyThreadNums--;
        pthread_mutex_unlock(&(pool->busyThreadNums));

    }
    pthread_exit(NULL);
}

//线程池的初始化
int threadPoolInit(Threadpool *pool, int minThreads, int maxThreads, int queueCapacity)
{
    if(pool == NULL)
    {
        return NULL_PTR;
    }

    do 
    {
        if(minThreads <= 0 || maxThreads <= 0 || minThreads >= maxThreads)
        {
            minThreads = DEFAULT_MIN;
            maxThreads = DEFAULT_MAX;
        }
        //更新线程池属性
        pool->minThreads = minThreads;
        pool->maxThreads = maxThreads;
        
        //初始化忙碌的线程数
        pool->busyThreadNums = 0;
        pool->liveThreadNums = 0;

        if(queueCapacity <= 0)
        {
            queueCapacity = DEFAULT_QUEUE_CAPACITY;
        }
        pool->queueCapacity = queueCapacity;
        pool->queueFront = 0;
        pool->queueRear = 0;
        pool->queueSize = 0;
        pool->taskQueue = (task_t *)malloc(sizeof(task_t) * pool->queueCapacity);
        if(pool->taskQueue == NULL)
        {
            perror("malloc error");
            break;
        }

        pool->threadId = (pthread_t *)malloc(sizeof(pthread_t) * maxThreads);
        if(pool->threadId == NULL)
        {
            perror("malloc error");
            _exit(-1);
        }

        //清楚脏数据
        memset(pool->threadId, 0, sizeof(pthread_t) * maxThreads);

        int ret = 0;
        //创建线程
        for(int idx = 0; idx < minThreads; idx++)
        {
            ret = pthread_create(&(pool->threadId[idx]), NULL, threadHander, pool);
            if(ret != 0)
            {
                perror("create error");
                break;
            }
        }

        if(ret != 0)
        {
            break;
        }

        pool->liveThreadNums = pool->minThreads;

        //初始化锁资源
        pthread_mutex_init(&(pool->mutexpool), NULL);
        pthread_mutex_init(&(pool->mutexbusy), NULL);

        if(pthread_cond_init(&(pool->notEmpty), NULL) != 0 || pthread_cond_init(&(pool->notFull), NULL)!= 0)
        {
            perror("init error");
            break;
        }
        
        return ON_SUCCESS;
    }while(0);
    
    if(pool->taskQueue != NULL)
    {
        free(pool->taskQueue);
        pool->taskQueue = NULL;
    }

    for(int idx = 0; idx < minThreads; idx++)
    {
        if(pool->threadId[idx] != 0)
        {
            pthread_join(pool->threadId[idx], NULL);
        }
    }
    if(pool->threadId != NULL)
    {
        free(pool->threadId);
        pool->threadId = NULL;
    }
    
    //释放锁资源
    pthread_mutex_destroy(&(pool->mutexbusy));
    pthread_mutex_destroy(&(pool->mutexpool));
    
    //释放条件变量资源
    pthread_cond_destroy(&(pool->notEmpty));
    pthread_cond_destroy(&(pool->notFull));

}

//线程池添加任务
int threadPoolAddTask(Threadpool *pool, void *(worker_hander)(void *), void *arg)
{
    if(pool == NULL)
    {
        return NULL_PTR;
    }
    //加锁
    pthread_mutex_lock(&(pool->mutexpool));
    while(pool->queueSize == pool->queueCapacity)
    {
       //等待生产者发送过来的条件变量
        pthread_cond_wait(&(pool->notFull), &(pool->mutexpool));
    }

    //程序到这个地方一定可以放任务
    pool->taskQueue[pool->queueRear].worker_hander = worker_hander;
    pool->taskQueue[pool->queueRear].arg = arg;

    //队尾向后移动
    pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;

    //任务数加一
    pool->queueSize++;

    pthread_mutex_unlock(&(pool->mutexpool));
    //发信号
    pthread_cond_signal(&(pool->notEmpty));
}

//线程池销毁
int threadPoolDestroy(Threadpool *pool)
{

}