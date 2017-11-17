#include "timer.h"

/* create time node */
timer_node *create_timer_node(uint64_t expire, uint8_t repeat, uint8_t exec_async, void *data, executor *exec, node_dtor *dtor) {
    timer_node *tn = smalloc(sizeof(timer_node));
    tn->expire_at = get_current_msec() + expire;
    tn->expire = expire;
    tn->repeat = repeat;
    tn->exec_async = exec_async;
    tn->data = data;
    tn->exec = exec;
    tn->dtor = dtor;
}

/* free timer node */
void free_timer_node(timer_node *tn) {
    if (tn->dtor) tn->dtor(tn->data);
    sfree(tn);
}

/* fire timer event */
void fire_timer_event(tw *t, timer_node *tn) {

    // todo trigger for async
    tn->exec(tn->data);
    if (tn->repeat) {
        tn->expire_at = get_current_msec() + tn->expire;
        add_timer_node(t, tn);
    } else {
        free_timer_node(tn);
    }
}

// order list, min first
static void list_add_node(node_list *l, timer_node *tn) {
    timer_node *current, *prev;
    timer_node *tmp_node = l->head.next;
    prev = &l->head;

    for(;tmp_node != NULL;prev = tmp_node, tmp_node = tmp_node->next) {
        if (tmp_node->expire_at < tn->expire_at) {
            continue;
        }
    }

    tn->next = prev->next;
    prev->next = tn;
}

/* free list */
static void free_list(node_list *l) {
    timer_node *current = l->head.next;
    while(current) {
        free_timer_node(current);
        current = current->next;
    }
}

/* create timer */
tw *create_timer(int duration) {
    tw *t = smalloc(sizeof(tw));       
    t->start_time = get_current_msec();
    t->current_time = t->start_time;
    t->last_time_tick = t->start_time;
    t->duration = duration;  
    t->current_slot = 0;
    spinlock_init(&t->lock);
    return t;
}

/* add timer node */
void add_timer_node(tw *t, timer_node *tn) {
    int slot = (t->current_slot + tn->expire/t->duration) & TIMER_MASK;
    spinlock_lock(&t->lock);
    list_add_node(&t->nodes[slot], tn);
    spinlock_unlock(&t->lock);
}

/* tick */
void tick(tw *t) {
    t->current_time = get_current_msec();

    if (t->last_time_tick - t->current_time < t->duration)  {
        return;
    }
    t->last_time_tick = t->current_time;
    t->current_slot = (++t->current_slot) & TIMER_MASK;
    node_list *nl = &t->nodes[t->current_slot];
    timer_node *tmp_node = nl->head.next;
    timer_node *prev = &nl->head;

    // ordered list
    while(tmp_node != NULL && tmp_node->expire_at < t->current_time) {
        prev->next = tmp_node->next;
        fire_timer_event(t, tmp_node);
        tmp_node = prev->next;
    }
}

/* free timer */
void free_timer(tw *t) {
    for(int i = 0; i < TIMER_CONTAIN; i++) {
        free_list(&t->nodes[i]);      
    }
    sfree(t);
    spinlock_destroy(&t->lock);
}
