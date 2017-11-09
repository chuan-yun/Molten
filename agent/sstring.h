#ifndef __MOLTEN_AGENT_SSTRING_H
#define __MOLTEN_AGENT_SSTRING_H

#include <string.h>
#include <stdint.h>

#include "common.h"

/* the design is like sds, but simple */
/* it is called smart string */

typedef struct {
    uint32_t len;    
    uint32_t free;
}sstring_header;

typedef struct {
    sstring_header header; 
    char *s;
}sstring;

void sstring_free(sstring s) {
    sfree(s.s);
}
/* align is | len| real string| \0 | */
/* do not move the string point to real string */
/* so can not use stand string function */
/* why i padding the \0, you guess */
sstring sstring_init(const char *string, int len) {
    sstring s = {0};
    s.s = smalloc(len+1);
    memcpy(s.s, string, len);
    s.header.len = len;
    s.header.free = 0;
    s.s[len] = '\0';
    return s;
}

sstring sstring_empty() {
    sstring s = {0};
    s.s = NULL;
    s.header.len = 0;
    s.header.free = 0;
    return s;
}

sstring sstring_reduce(sstring *s, int start_pos, int len) {
    assert(start_pos + len <= s->header.len);
    sstring n = sstring_init(s->s + start_pos, len);
    sstring_free(*s);
    return n;
}

/* reset string */
sstring sstring_reset(sstring *s) {
    s->header.free = s->header.len;
    memset(s->s, 0x00, s->header.len);
}

/* cat sstring */
void sstring_ncat(sstring *s, const char *string, int size) {
    int string_len = size;   
    int need_realloc = string_len - s->header.free;
    int start_pos = s->header.len - s->header.free;
    printf("[sstring] ncat, orig ptr:%p, need_realloc:%d", s->s, need_realloc);
    if (need_realloc > 0) {
        int new_size = s->header.len + need_realloc;
        printf("[sstring] ncat, new ptr:%p, new_string:%s", s->s, string);
        s->s = srealloc(s->s, new_size);
        memcpy(s->s + start_pos, string, string_len);
        s->header.free = 0;
        s->header.len = new_size;
    } else {
        memcpy(s->s + start_pos, string, string_len);
        s->header.free = s->header.free - string_len;
    }
}

void sstring_cat(sstring *s, const char *string) {
    sstring_ncat(s, string, strlen(string));
}

void sstring_grow(sstring *s, int len) {
    assert(len > s->header.len);
    srealloc(s->s, len);
    s->header.free = len - s->header.len;
    s->header.len = len; 
}

char *sstring_unwrap(sstring s) {
    return s.s;
}

void sstring_debug(sstring s) {
    printf("len:%d, free:%d, string:%p\n", s.header.len, s.header.free, s.s);
}

#endif
