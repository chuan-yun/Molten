#ifndef __MOLTEN_AGENT_EPOLL_EVENT_H
#define __MOLTEN_AGENT_EPOLL_EVENT_H

#include <sys/epoll.h>
#include <time.h>

#include "common.h"

typedef struct {
    int epoll_fd;
    int max_event; 
    struct epoll_event *events;         /* prealloc size  for performance, the size is same to main event loop */
}epoll_implement;

/* create epoll implement */
int create_implement_event_loop(event_loop *el) {
    epoll_implement *ei = smalloc(sizeof(epoll_implement));
    ei->max_event = el->max_event;
    ei->events = smalloc(sizeof(struct epoll_event) * el->max_event);
    if (!ei->events) {
        sfree(ei);
        return -1; 
    }
    ei->epoll_fd =epoll_create(256); /* since linxu 2.5.6, size is ignore, what event you input is ok */
    if (ei->epoll_fd < 0) {
        sfree(ei->events);
        sfree(ei);
        return -1;
    }
    el->specific_event_notification = (void *)ei;
    return 0;
}

/* free */
void free_implement_event_loop(event_loop *el) {
   epoll_implement *ei = (epoll_implement *) el->specific_event_notification;
   close(ei->epoll_fd);
   sfree(ei->events);
   sfree(ei);
}

/* for epoll we can use epoll_event->data->ptr to store data point
 * but for other event loop just like select, they do not do this.
 * if only support epoll/kevent  for linux/mac os/freebsd, the event loop mode will change :)
 * */
int add_event_implement(event_loop *el, int fd, int mask) {

    /* init epoll event for add/mod */
    struct epoll_event ee = {0};
    ee.events = 0;      /* init */
    ee.data.fd = fd;

    /* the event mask maintain in epoll events */
    int a = el->regist_events[fd].mask == EVENT_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    mask |= el->regist_events[fd].mask; /* merge */

    if (mask & EVENT_READ)  ee.events |= EPOLLIN;
    if (mask & EVENT_WRITE)  ee.events |= EPOLLOUT;

    epoll_implement * ei = (epoll_implement *)el->specific_event_notification;
    if (epoll_ctl(ei->epoll_fd, a, fd, &ee) == -1)  return A_FAIL;
    AGENT_SLOG(SLOG_DEBUG, "[epoll] modify event fd:%d, action:%d, mask:%d success", fd, a, mask);
    return A_SUCCESS;
}

/* here logic: if the final action is none,use EPOLL_CTL_DEL; else use EPOLL_CTL_MOD */
void remove_event_implement(event_loop *el, int fd, int mask) {
    struct epoll_event ee = {0};
    ee.events = 0;
    ee.data.fd = fd;

    int nmask = el->regist_events[fd].mask & (~mask);
    if (nmask & EVENT_READ) ee.events |= EPOLLIN;
    if (nmask & EVENT_WRITE) ee.events |= EPOLLOUT;

    epoll_implement * ei = (epoll_implement *)el->specific_event_notification;
    if (nmask == EVENT_NONE) {
        epoll_ctl(ei->epoll_fd, EPOLL_CTL_DEL, fd, &ee);
    } else {
        epoll_ctl(ei->epoll_fd, EPOLL_CTL_MOD, fd, &ee);
    }
}

/* return number of event triger, if result is 0 , time reach */
/* current tick, we need to control */
int event_loop_implement(event_loop *el, struct timeval *intval) {
    epoll_implement * ei = (epoll_implement *)el->specific_event_notification;
    int milliseconds = intval ? (intval->tv_sec * 1000 + intval->tv_usec/1000):-1;
    int event_num = epoll_wait(ei->epoll_fd, ei->events, ei->max_event, milliseconds);
    if (event_num > 0) {
        for (int i=0; i< event_num; i++) {
            struct epoll_event *ee = &ei->events[i];
            int mask = 0;

            if (ee->events & EPOLLIN) mask |= EVENT_READ;
            if (ee->events & EPOLLOUT) mask |= EVENT_WRITE;
            if (ee->events & EPOLLERR) mask |= EVENT_WRITE;
            if (ee->events & EPOLLHUP) mask |= EVENT_WRITE;

            el->fire_events[i].fd = ee->data.fd;
            el->fire_events[i].mask = mask;
        }
    } else {
        event_num = 0;  /* timer reach */
    }
    return event_num;
}

#endif
