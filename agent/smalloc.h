#ifndef __MOLTEN_AGENT_SMALLOC_H
#define __MOLTEN_AGENT_SMALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trace.h"
#include "atomic.h"

#define MEMORY_PREFIX_SIZE sizeof(size_t)

size_t smalloc_size(void *p);
void *smalloc(size_t size);
void sfree(void *p);
void *srealloc(void *p, size_t size);
char *sstrdup(const char *str);
size_t dump_used_bytes();
#endif
