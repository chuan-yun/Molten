#ifndef __MOLTEN_AGENT_TIMER_H
#define __MOLTEN_AGENT_TIMER_H

#include <stdio.h>

typedef (void *executor)(void *data);

typedef struct {
    int timestap;
    executor exec;
}timer;

int add_timer(timer *t);
int del_timer(timer *t);
void tick();

#endif
