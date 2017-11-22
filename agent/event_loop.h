#ifndef __MOLTEN_EVENT_LOOP_H
#define __MOLTEN_EVENT_LOOP_H

#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "common.h"
#include "socket.h"
#include "protocol.h"
#include "timer.h"
#include "atomic.h"

#define MAX_CONNECtiON          1024 /* for debug now */
#define ADDItION_CONNECTION     96   /* keep hole for process used */

void remove_net_event(event_loop *el, int fd, int mask);
int add_net_event(event_loop *el, int fd, void *client, net_event_process process, int mask);
event_loop *create_main_event_loop(int max_size);
void free_main_event_loop(event_loop *el);
void main_loop_stop(event_loop *el);
uint64_t execute_loop(event_loop *el);
void write_to_client(event_loop *el, int fd, void *client, int mask);
void read_from_client(event_loop *el, int fd, void *client, int mask);
void accept_net_client(event_loop *el, int fd, void *client, int mask);

#endif
