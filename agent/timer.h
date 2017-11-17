#ifndef __MOLTEN_AGENT_TIMER_H
#define __MOLTEN_AGENT_TIMER_H

#include <stdio.h>

#include "common.h"
#include "lock.h"

// use time wheel 
// sigle time wheel, just usec level,
// not multi time wheel
// but it is for complex timer, we timer is simple 
// ordered time wheel

#define TIMER_SHIFT         8
#define TIMER_CONTAIN       (1 << TIMER_SHIFT)
#define TIMER_MASK          (TIMER_CONTAIN - 1)

typedef void executor(void *data);
typedef void node_dtor(void *data);

typedef struct timer_node  timer_node;

struct timer_node {
    uint64_t expire_at;
    uint64_t fire_at;
    uint64_t expire;
    uint8_t repeat:1;
    uint8_t exec_async:1;
    void *data;
    executor *exec;
    node_dtor *dtor;
    timer_node *next;
};

typedef struct{
     timer_node head; 
}node_list;

// all time is base on millisecond
typedef struct {
    node_list nodes[TIMER_CONTAIN];
    uint64_t start_time;
    uint64_t current_time;
    uint32_t duration;          // ms precision
    uint32_t current_slot;
    uint64_t last_time_tick;
    spinlock lock;
}tw; // time wheel

tw *create_timer(int duration);
void add_timer_node(tw *t, timer_node *tn);
void tick(tw *t);
void free_timer(tw *t);
timer_node *create_timer_node(uint64_t expire, uint8_t repeat, uint8_t exec_async, void *data, executor *exec, node_dtor *dtor);
void free_timer_node(timer_node *tn);
#endif
