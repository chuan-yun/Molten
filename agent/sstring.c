#include "sstring.h"

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

/* string reduce */
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

    if (need_realloc > 0) {
        int new_size = s->header.len + need_realloc;
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

/* use sstring cat */
void sstring_vsprintf(sstring *s, const char *fmt, va_list ap) {
    //test_buf;
    // hypotheis  test buf is fmt double size
    int h_len = strlen(fmt) * 2;
    va_list ap_c;
    char *test_buf = smalloc(h_len);

    // vsnprintf the last charater is terminal null
    test_buf[h_len - 2] = '\0';
    while(1) {
        va_copy(ap_c, ap);
        vsnprintf(test_buf, h_len, fmt, ap_c); 
        va_end(ap_c);
        if (test_buf[h_len - 2] != '\0') {
            sfree(test_buf);
            h_len *=2;
            test_buf = smalloc(h_len);
            continue;
        }
        break;
    }
    sstring_cat(s, test_buf);       
    sfree(test_buf);
}

/* sstring vs cat */
void sstring_vscat(sstring *s, const char *fmt, ...) {
    va_list ap;
    sstring tmp = sstring_empty();  
    va_start(ap, fmt);
    sstring_vsprintf(&tmp, fmt, ap);
    va_end(ap);
    sstring_ncat(s, tmp.s, tmp.header.len - tmp.header.free);
    sstring_free(tmp);
}


void sstring_grow(sstring *s, int len) {
    assert(len > s->header.len);
    srealloc(s->s, len);
    s->header.free = len - s->header.len;
    s->header.len = len; 
}

int sstring_length(sstring s) {
    return s.header.len - s.header.free;
}

char *sstring_unwrap(sstring s) {
    return s.s;
}

void sstring_debug(sstring s) {
    printf("len:%d, free:%d, string:%p\n", s.header.len, s.header.free, s.s);
}

void sstring_print(sstring s) {
    printf("%s", s.s);
}
