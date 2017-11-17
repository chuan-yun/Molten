#ifndef __MOLTEN_AGENT_SSTRING_H
#define __MOLTEN_AGENT_SSTRING_H

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#include "smalloc.h"

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

void sstring_free(sstring s);
sstring sstring_init(const char *string, int len);
sstring sstring_empty();
sstring sstring_reduce(sstring *s, int start_pos, int len);
sstring sstring_reset(sstring *s);
void sstring_vsprintf(sstring *s, const char *fmt, va_list ap);
void sstring_ncat(sstring *s, const char *string, int size);
void sstring_vscat(sstring *s, const char *fmt, ...);
void sstring_cat(sstring *s, const char *string);
void sstring_grow(sstring *s, int len);
char *sstring_unwrap(sstring s);
void sstring_debug(sstring s);
int sstring_length(sstring s);
void sstring_print(sstring s);
#endif
