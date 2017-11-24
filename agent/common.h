#ifndef __MOLTEN_AGENT_COMMON_H
#define __MOLTEN_AGENT_COMMON_H

#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>

#include "molten_slog.h"
#include "smalloc.h"
#include "sstring.h"
#include "trace.h"
#include "worker_pool.h"

#define AGENT_SLOG(level, format, ...)                slog_record(level, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define A_SUCCESS       0
#define A_FAIL          -1

/* common micro */
#define CLIENT_IP_MAX_LEN   64        /* for ipv6 is 46 */
#define CLIENT_MAX_READ     1024      /* client max read */

/* for RW WRITE */
#define RW_WRITE_CONTINUE   1
#define RW_WRITE_FINISH     2
#define RW_WRITE_ERROR      -1

/* for event */
#define EVENT_NONE  0
#define EVENT_READ  1
#define EVENT_WRITE 2

#define ACTION_SUCCESS  0
#define ACTION_FAIL     -1

typedef struct st_event_loop  event_loop;
typedef void net_event_process(event_loop *el, int fd, void *client, int mask);

/* todo temp add event at here */
typedef struct {
    uint8_t type;
    int mask;
}event;

typedef struct {
    uint8_t type;
    int mask;
    int fd;
}timer_event;

typedef struct {
    uint8_t type;
    int mask;
    int fd;
    void *client;
    net_event_process *r_handler;
    net_event_process *w_handler;
}net_event;

// main loop only for linux use epoll, other platform use select, if your platform without these, you pray for your self.
struct st_event_loop{
    int stop;  
    int max_event;
    net_event *regist_events;
    net_event *fire_events;                 /* for fire event */
    int max_fd;
    void *specific_event_notification;   /* specific event notify struct select or epoll */
};

typedef struct {
    /* common */
    pid_t pid;
    char *pid_file;
    char *log_file;
    int daemon;
    int prepare_stop;

    /* net */
    int port;
    int backlog;
    event_loop *el;
    int listen_fd;

    /* monitor */
    time_t uptime;
    int active_client_num;
    int total_client_num;

    /* sync worker */
    int worker_num;
    worker_pool *wp; 
}m_server;

extern m_server server;

// here we can use clock_gettime, but our timer is millisecond level, not need more precision
// timespec see you goodbye
static inline long get_current_usec() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 * 1000 + time.tv_usec;
}

static inline long get_current_msec() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec/1000;
}
 
sstring server_status();
#endif
