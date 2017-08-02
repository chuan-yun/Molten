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

#ifndef MOLTEN_LOG_H
#define MOLTEN_LOG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>

# if defined(__linux__)
#include <linux/limits.h>
# endif

#ifdef HAS_CURL
#include "curl/curl.h"
#include "curl/easy.h"
#endif

#ifdef HAS_KAFKA
#include "librdkafka/rdkafka.h"
#endif

#include "php.h"
#include "Zend/zend_llist.h"
#include "molten_util.h"
#include "molten_log.h"
#include "molten_util.h"
#include "php7_wrapper.h"

#ifndef PATH_MAX
#define PATH_MAX 4096 
#endif

#define ALLOC_LOG_SIZE  1024
#define DEFAULT_LOG_DIR "/var/wd/log/tracing/php/"
#define DEFAULT_PATH    DEFAULT_LOG_DIR"tracing"
#define LOG_FORMAT      "%Y%m%d" 

#define SINK_NONE   0
#define SINK_LOG    1<<0
#define SINK_STD    1<<1

#ifdef HAS_CURL
#define SINK_HTTP   1<<2
#endif

#ifdef HAS_KAFKA 
#define SINK_KAFKA  1<<3
#endif

#define SPANS_WRAP  1<<0
#define SPANS_BREAK 1<<1

/* http sink */
/* current use php_stream or libcurl, we can check */
/* not use union, there has other fields */
typedef struct {
    uint8_t type;
    char *post_uri;

    /*
    struct {
       php_stream_context *c;
       php_stream         *s;
    } stream;
    */
     
} mo_http_sink_t;


/* chain log */
typedef struct {
    uint8_t sink_type;          /* log sink type */
    uint8_t output_type;        /* sink output type */
    uint16_t support_type;      /* sink support type */

    /* sink log file */
    char *path;
    char real_path[128];        /* log real path */
    int tm_yday;                /* day in year, get by ctime */
    int fd;
    ino_t ino;
    char *format; 

    /* sink http */
    char *post_uri;             /* post uri */

    /* kafka */
    char *brokers;              /* brokers */
    char *topic;                /* topic */
    char *buf;
    uint64_t total_size;
    uint64_t alloc_size;
    zval *spans;
} mo_chain_log_t;

/* function */
void mo_chain_log_ctor(mo_chain_log_t *log, char *log_path, long sink_type, long output_type, char *post_uri);
int mo_chain_log_set_file_path(char *new_path);
void mo_chain_log_add(mo_chain_log_t *log, char *buf, size_t size);
void mo_chain_log_flush(mo_chain_log_t *log);
void mo_chain_log_dtor(mo_chain_log_t *log);
void mo_chain_add_span(mo_chain_log_t *log, zval *span);
void mo_log_write(mo_chain_log_t *log, char *bytes, int size);
#ifdef HAS_CURL
void send_data_by_http(char *post_uri, char *post_data);
#endif
#endif
