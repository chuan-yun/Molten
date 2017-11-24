#ifndef __MOLTEN_AGENT_ATOMIC_H
#define __MOLTEN_AGENT_ATOMIC_H

/* we use __sync_add_and_fetch group to automic action */
#define atomic_incr(ptr)            __sync_add_and_fetch(ptr, 1)
#define atomic_decr(ptr)            __sync_sub_and_fetch(ptr, 1)
#define atomic_fetch_add(ptr, n)    __sync_fetch_and_add(ptr, n)
#define atomic_add_fetch(ptr, n)    __sync_add_and_fetch(ptr, n)
#define atomic_add(ptr, n)          __sync_add_and_fetch(ptr, n)
#define atomic_get(ptr)             __sync_fetch_and_add(ptr, 0)
#define atomic_sub(ptr, n)          __sync_fetch_and_sub(ptr, n)


/* memory barrier */
#endif
