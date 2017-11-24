#include "worker_pool.h"

static task *task_queue_pull(task_queue *tq) {
    task *t = tq->tail;
    tq->tail = t->prev;
    --tq->len;
    return t;
}

static task* task_queue_pull_wait(task_queue *tq, int *running, int worker_id) {
    struct timespec abstime;
    struct timeval  intval; 
    pthread_mutex_lock(&tq->lock);
    while(tq->len == 0 && *running) {
        gettimeofday(&intval, NULL);
        abstime.tv_sec = intval.tv_sec;
        abstime.tv_nsec = (intval.tv_usec + 100000) * 1000;

        // timeout is intval 100ms
        pthread_cond_timedwait(&tq->cond, &tq->lock, &abstime);
        if (!(*running)) {
            pthread_mutex_unlock(&tq->lock);
            return NULL;
        }
    }
    task *t = task_queue_pull(tq);
    pthread_mutex_unlock(&tq->lock);

    return t;
}

static void task_queue_push(task_queue *tq, task *t) {
    pthread_mutex_lock(&tq->lock);
    ++tq->len;
    t->prev = tq->tail;
    tq->tail = t;
    pthread_mutex_unlock(&tq->lock);
    pthread_cond_signal(&tq->cond);
}

static void task_queue_init(task_queue *tq) {
    tq->len = 0;
    tq->head.prev = NULL;
    tq->tail = &tq->head;
    tq->processed = 0;
    pthread_cond_init(&tq->cond, NULL);
    pthread_mutex_init(&tq->lock, NULL);
}

static void task_queue_destroy(task_queue *tq) {
    pthread_mutex_lock(&tq->lock);

    while(tq->len) {
        task *t = task_queue_pull(tq);
        sfree(t);
    }

    pthread_mutex_unlock(&tq->lock);
    pthread_cond_destroy(&tq->cond);
    pthread_mutex_destroy(&tq->lock);
}

static void *work(void *wr) {
    worker *w = (worker *)wr;
    worker_pool *wp = w->wp;

    pthread_barrier_wait(&wp->pb);
    uint64_t start_us, end_us; 
    while(wp->running) {
        task *t = task_queue_pull_wait(&wp->tq, &wp->running, w->worker_id);
        if (t) {
            atomic_incr(&wp->working_num);

            start_us = get_current_usec_1();
            t->exec(t->data);
            atomic_incr(&wp->tq.processed);
            end_us = get_current_usec_1();
            w->all_execute_time += end_us - start_us;
            sfree(t);

            atomic_decr(&wp->working_num);
        }
    }
    pthread_barrier_wait(&wp->pb);
}

static void init_worker(int worker_id, worker *w,  worker_pool *wp) {
    w->worker_id = worker_id;    
    w->wp = wp;
    w->all_execute_time = 0;
    pthread_create(&w->tid, NULL, (void *)work, (void *)w);
    pthread_detach(w->tid);
}

/* create worker pool */
worker_pool* create_worker_pool(int worker_num) {
    worker_pool *wp = (worker_pool *)smalloc(sizeof(worker_pool));
    wp->ws = (worker *)smalloc(sizeof(worker) * worker_num);
    wp->running = 1;
    wp->all_worker_num = worker_num;
    task_queue_init(&wp->tq);

    pthread_barrier_init(&wp->pb, NULL, worker_num + 1);
    for(int i = 0; i < worker_num; i++) {
        init_worker(i, wp->ws + i, wp);
    }
    
    pthread_barrier_wait(&wp->pb);
    
    return wp;
}

void dump_worker_pool_status(worker_pool *wp) {
    printf("-----worker pool status ------------\n");
    printf("worker num:%d\n", wp->working_num);
    for(int i = 0; i < wp->all_worker_num; i++) {
        printf("worker_pool_id:%d, execute_time%d\n", i, (wp->ws+i)->all_execute_time);
    }
}

/* destroy worker pool */
void destroy_worker_pool(worker_pool *wp) {
    task_queue_destroy(&wp->tq);
    int res = pthread_barrier_init(&wp->pb, NULL, wp->all_worker_num + 1);
    wp->running = 0;
    pthread_barrier_wait(&wp->pb);
    pthread_barrier_destroy(&wp->pb);
}

void add_task(worker_pool *wp, worker_executor exec, void *data) {
    task *t = (task *)smalloc(sizeof(task));
    t->exec = exec;
    t->data = data;
    t->prev = NULL; 
    task_queue_push(&wp->tq, t);
}
