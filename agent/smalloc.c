#include "smalloc.h"

/* memory alloc */
/* |prefx(size_t)| ********memory *******| padding for sizeof(long),  we pretend it has| */

static size_t used_bytes;

static void incr_used_bytes(size_t size){
    /* for memory align */        
    if(size & (sizeof(long)-1))  {
        size += sizeof(long) - (size & (sizeof(long) -1));
    }
    
    atomic_add(&used_bytes, size);
}

static void decr_used_bytes(size_t size) {
    /* for memory align */        
    if(size & (sizeof(long)-1))  {
        size += sizeof(long) - (size & (sizeof(long) -1));
    }
    atomic_sub(&used_bytes, size);
}

size_t smalloc_size(void *p) {
    size_t size =  *((size_t *)((char*)p - MEMORY_PREFIX_SIZE));
    if(size & (sizeof(long)-1))  {
        size += sizeof(long) - (size & (sizeof(long) -1));
    }
    return size;
}

static void smalloc_oom(size_t size) {
    fprintf(stderr, "Out of memory, alloc %d bytes error\n", size);
    fflush(stderr);
    abort();
}

void *smalloc(size_t size) {
    AGENT_SMALLOC(size);
    void *p = malloc(MEMORY_PREFIX_SIZE + size);
    if (!p) {
        smalloc_oom(size);
    }
    *((size_t *)p) = size;
    incr_used_bytes(MEMORY_PREFIX_SIZE + size); 
    return (char *)p + MEMORY_PREFIX_SIZE;
}

void sfree(void *p) {
    AGENT_SFREE();
    if (p == NULL) return;
    void *r = (char *)p - MEMORY_PREFIX_SIZE;
    size_t size = *(size_t *)r;
    decr_used_bytes(size + MEMORY_PREFIX_SIZE);
    free(r);
}

void *srealloc(void *p, size_t size) {
    if (p == NULL) return smalloc(size);
    void *r = (char *)p - MEMORY_PREFIX_SIZE;
    size_t old_size = *(size_t *)r;
    void *n = realloc(r, size + MEMORY_PREFIX_SIZE); 
    if (!n) {
        smalloc_oom(size);
    }
    *((size_t *)n) = size;
    decr_used_bytes(old_size + MEMORY_PREFIX_SIZE);
    incr_used_bytes(size + MEMORY_PREFIX_SIZE);
    return (char *)n + MEMORY_PREFIX_SIZE;
}

char *sstrdup(const char *str) {
    if(str == NULL)  return NULL;
    size_t size = strlen(str) + 1;
    char *n = smalloc(size);
    memcpy(n, str, size);
    return n;
}

size_t dump_used_bytes() {
    size_t res;
    res = atomic_get(&used_bytes);
    return res;
}
