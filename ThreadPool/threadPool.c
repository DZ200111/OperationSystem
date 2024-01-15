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
            ret = pthread_create(&(pool->threadId[idx]), NULL, threadHander, NULL);
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


}

//线程池销毁
int threadPoolDestroy(Threadpool *pool)
{
    
}