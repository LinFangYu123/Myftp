#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

typedef struct TASK{
    void *(*func)(void *arg);       //任务队列的回调函数
    void *arg;                      //回调函数的参数
    struct TASK *next;              //下一个任务节点
    //struct TASK *prev;                //前一个任务节点
}thread_pool_task;

typedef struct WORKER{
    pthread_t pthread_id;           //工作线程的线程id
    struct MANAGER *pool;           //工作线程所在的线程池
    struct WORKER *next;            //下一个工作线程
    struct WORKER *prev;            //前一个工作线程
    int status;                     //工作线程的状态 1为正在处理任务，0为空闲
}thread_pool_worker;

typedef struct MANAGER{
    thread_pool_worker *workers;        //工作线程的头节点
    thread_pool_worker *workers_end;    //工作线程的最后一个节点
    
    thread_pool_task *tasks;            //任务队列的头节点
    thread_pool_task *tasks_end;        //任务队列的最后一个节点
    
    pthread_cond_t cond;                //线程条件变量
    pthread_mutex_t mutex;              //线程锁
    
    int shutdown;                       //销毁线程池 1为销毁线程池 0为不销毁
    
    int max_thread_num;                 //线程池能够容纳的最大线程数量
    
    int current_thread_num;             //线程池当前的线程数量
    
    int current_thread_task_num;        //线程池当前正在执行任务的线程数量
    
    int current_thread_free_num;        //线程池当前空闲的线程数量
    
    int current_wait_task_num;          //线程池任务队列中等待的任务数量
    
    int max_free_thread_num;            //线程池允许最大的空闲线程数量
    
}pthread_pool;

void *workercallback(void *arg);

int create_thread_pool(void *thread_pool,int max_thread_num,int worker_num,int allow_free_num);

int thread_pool_add_task(void *thread_pool,void *(*newtask)(void *arg),void *arg);

int thread_pool_destroy(void *thread_pool);

int get_max_thread_num(void *thread_pool);

int get_current_thread_num(void *thread_pool);

int get_current_thread_task_num(void *thread_pool);

int get_current_thread_free_num(void *thread_pool);

int get_current_wait_task_num(void *thread_pool);

int get_max_free_thread_num(void *thread_pool);

#endif