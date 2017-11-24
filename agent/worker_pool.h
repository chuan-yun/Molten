#ifndef __MOLTEN_THREAD_POOL_H
#define __MOLTEN_THREAD_POOL_H

/* all abi is depend on posix thread api */
#include <pthread.h>
#include <sys/time.h>
#include <stdint.h>

#include "atomic.h"
#include "smalloc.h"

#define WP_SUCCESS  0
#define WP_FAIL     -1

typedef void worker_executor(void *data);
typedef struct worker_pool  worker_pool;
typedef struct task         task;

struct task{
    worker_executor *exec;
    void *data;

    task *prev; 
};
 
/* todo use lock free */
/* abb and memory barrier */
typedef struct {
    uint64_t processed;
    uint64_t len;
    task    head;
    task    *tail; 
    
    // notify
    pthread_mutex_t lock;
    pthread_cond_t cond;
}task_queue;

typedef struct {
    uint32_t    worker_id;
    uint64_t    all_execute_time;   /* us(microsecond) */
    pthread_t   tid;
    worker_pool *wp;
}worker;

struct worker_pool{
    worker              *ws;
    pthread_barrier_t   pb;                 /* use for sync, use begin and end barrier */
    uint32_t            all_worker_num;     /* can use volidate keyword, or use atomic func */
    uint32_t            working_num;
    int                 running;
    task_queue          tq;
};

static inline long get_current_usec_1() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 * 1000 + time.tv_usec;
}

worker_pool* create_worker_pool(int worker_num);
void destroy_worker_pool(worker_pool *wp);
void add_task(worker_pool *wp, worker_executor exec, void *data);

#endif
