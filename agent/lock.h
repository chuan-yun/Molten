#ifndef __MOLTEN_AGENT_LOCK_H
#define __MOLTEN_AGENT_LOCK_H

typedef int spinlock;

static inline void spinlock_init(spinlock *l) {
    *l = 0;
}

static inline void spinlock_lock(spinlock *l) {
    while(__sync_lock_test_and_set(l, 1));
}

static inline void spinlock_unlock(spinlock *l) {
    __sync_lock_release(l);
}

static inline void spinlock_destroy(spinlock *l) {
    (void)(*l);
}

#endif
