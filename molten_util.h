/**
 * Copyright 2017 chuan-yun silkcutks <silkcutbeta@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOLTEN_UTIL_H
#define MOLTEN_UTIL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include "php.h"
#include "php7_wrapper.h"
#include "molten_common.h"

#ifdef USE_ZIPKIN_HEADER
#define MOLTEN_HEADER_PREFIX "X-B3-"
#define MOLTEN_REC_HEADER_PREFIX "HTTP_X_B3_"
#else
#define MOLTEN_HEADER_PREFIX "X-W-"
#define MOLTEN_REC_HEADER_PREFIX "HTTP_X_W_"
#endif

#define MOLTEN_HEADER_PREFIX_LEN (sizeof(MOLTEN_HEADER_PREFIX) - 1)
#define MOLTEN_HEADER_TRACE_ID MOLTEN_HEADER_PREFIX"TraceId"
#define MOLTEN_HEADER_SPAN_ID MOLTEN_HEADER_PREFIX"SpanId"
#define MOLTEN_HEADER_PARENT_SPAN_ID MOLTEN_HEADER_PREFIX"ParentSpanId"
#define MOLTEN_HEADER_SAMPLED MOLTEN_HEADER_PREFIX"Sampled"
#define MOLTEN_HEADER_FLAGS MOLTEN_HEADER_PREFIX"Flags"

#define MOLTEN_REC_TRACE_ID MOLTEN_REC_HEADER_PREFIX"TRACEID"
#define MOLTEN_REC_SPAN_ID MOLTEN_REC_HEADER_PREFIX"SPANID"
#define MOLTEN_REC_PARENT_SPAN_ID MOLTEN_REC_HEADER_PREFIX"PARENTSPANID"
#define MOLTEN_REC_SAMPLED MOLTEN_REC_HEADER_PREFIX"SAMPLED"
#define MOLTEN_REC_FLAGS MOLTEN_REC_HEADER_PREFIX"FLAGS"

#define SPAN_VERSION            "php-4" 

#define MO_FRAME_ENTRY          1
#define MO_FRAME_EXIT           2
#define MO_USEC_PER_SEC         1000000l

static inline long mo_time_sec()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (long) tv.tv_sec;
}

static inline long mo_time_m()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    long sec = tv.tv_sec;
    return (long) sec/60 ;
}

static inline long mo_time_u2s(long usec)
{
    return (long) usec/(MO_USEC_PER_SEC);
}

static inline long mo_time_usec()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (long) tv.tv_sec * MO_USEC_PER_SEC + (long) tv.tv_usec;
}

/* {{{ find server variables */
static int find_server_var(char *key, int key_size, void **ret) 
{
    /* for jit preload server */
    if (PG(auto_globals_jit)) {
        mo_zend_is_auto_global("_SERVER", sizeof("_SERVER") - 1);
    }

#if PHP_MAJOR_VERSION < 7
    zval **server = (zval **)&PG(http_globals)[TRACK_VARS_SERVER];
    return mo_zend_hash_zval_find(Z_ARRVAL_P(*server), key, key_size, ret);
#else 
    zval *server = &PG(http_globals)[TRACK_VARS_SERVER];
    return mo_zend_hash_zval_find(Z_ARRVAL_P(server), key, key_size, ret);
#endif
}
/* }}} */

/* {{{ join origin url */
static inline zend_bool join_ori_url(smart_string *url, zend_bool trim_query_string)
{
    smart_string tmp = {0};
    zval *http_host;
    if (find_server_var("HTTP_HOST", sizeof("HTTP_HOST"), (void **)&http_host) != SUCCESS) {
        return 0;
    }
    
    zval *request_uri;
    if (find_server_var("REQUEST_URI", sizeof("REQUEST_URI"), (void **)&request_uri) != SUCCESS) {
        return 0;
    }

    if (http_host != NULL && request_uri != NULL && (MO_Z_TYPE_P(http_host) == IS_STRING) && (MO_Z_TYPE_P(request_uri) == IS_STRING)) {
        smart_string_appendl(&tmp, Z_STRVAL_P(http_host), Z_STRLEN_P(http_host));
        smart_string_appendl(&tmp, Z_STRVAL_P(request_uri), Z_STRLEN_P(request_uri));

        if (trim_query_string) {
            int i; 
            char *string = smart_string_str(tmp);
            for(i=Z_STRLEN_P(http_host); i < smart_string_len(tmp); i++) {
                if(string[i] == '?') {
                     smart_string_appendl(url, smart_string_str(tmp), i);
                     break;
                }
            }

            if (i == smart_string_len(tmp)) {
                smart_string_appendl(url, smart_string_str(tmp), smart_string_len(tmp));
            }
            smart_string_0(url);
        } else {
            smart_string_appendl(url, smart_string_str(tmp), smart_string_len(tmp));
            smart_string_0(url);
        }
        smart_string_free(&tmp);
        return 1;
    } else {
        return 0;
    }
}
/* }}} */


uint64_t rand_uint64(void);
void b2hex(char **output, const unsigned char *input, int input_len);
void bin2hex64(char **output, const uint64_t *input);
void rand64hex(char **output);
void build_span_id_random(char **span_id, char *parent_span_id, int span_count);
void build_span_id_level(char **span_id, char *parent_span_id, int span_count);
int check_hit_ratio(int probability , int divisor);
smart_string repr_zval(zval *zv, int limit TSRMLS_DC);
#endif
