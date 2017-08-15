/**
 * Copyright 2017 chuan-yun silkcutKs <silkcutbeta@gmail.com>
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

#ifndef MOLTEN_STRUCT_H
#define MOLTEN_STRUCT_H

#include "stdint.h"
#include <netinet/in.h>

#include "php.h"

#include "php7_wrapper.h"
#include "molten_log.h"
#include "molten_stack.h"

/* key val map */
typedef struct {
    char *name;
    char *receive_key;
    int receive_key_len;
    char *pass_key;
    int pass_key_len;
    char *val;
    zend_bool is_pass;
} mo_chain_key_t;

/* chain header */
typedef struct {
    mo_chain_key_t *trace_id;           /* trace id */
    mo_chain_key_t *span_id;            /* span id */
    mo_chain_key_t *parent_span_id;     /* parent sapn id */
    mo_chain_key_t *sampled;            /* sampled */
    mo_chain_key_t *flags;              /* flags */
    HashTable *chain_header_key;        /* chain uri key*/

    char ip[INET_ADDRSTRLEN];           /* device ip */
    int port;                           /* port */
    int is_sampled;                     /* is sampled */
} mo_chain_header_t;

/* chain struct */
typedef struct mo_chain_st {
    
    /* chain header */
    mo_chain_header_t pch;              /* chain header */

    /* service name */
    char *service_name;

    /* error info */
    zval *error_list;

    /* span stack */
    mo_stack *span_stack;               /* link to global stack */ 
    
    /* excute time */
    long execute_begin_time;            /* execute begin time */
    long execute_end_time;              /* execute end time */

    /* http request detail */
    const char *sapi;
    const char *method;
    const char *content_type;
    char *script;
    const char *request_uri;
    const char *query_string;
    zend_bool is_cli;

    /* console paramter */
    int argc; 
    const char **argv;

    /* trace log */
    mo_chain_log_t *pcl; 

} mo_chain_t;

typedef struct {
    uint8_t             type;             /* frame type, entry or exit */
    uint32_t            level;            /* nesting level */
    smart_string        function;         /* function name */
    smart_string        class;            /* class name */

    uint32_t            arg_count;        /* arguments number */

    int64_t             entry_time;       /* entry wall time */ 
    int64_t             exit_time;        /* exit wall time */

#if PHP_VERSION_ID < 70000
    zval                **ori_args;       /* origin args */
#else
    zval                *ori_args;        /* origin args */
#endif
    zval                *object;          /* object */
    zval                *ori_ret;         /* origin ret */
    zend_class_entry    *scope;           /* class entry */

    mo_stack            *span_stack;      /* global span stack */

    zval                *span_extra;      /* because of runtime param will be delete, so we need to get info after execute, extra for user defiend func */
} mo_frame_t;
#endif
