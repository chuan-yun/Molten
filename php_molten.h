/**
 * Copyright 2017 chuan-yun silkCutKs <silkcutbeta@gmail.com>
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

#ifndef PHP_MOLTEN_H
#define PHP_MOLTEN_H

#define PHP_MOLTEN_VERSION    "0.1.2beta"

extern zend_module_entry molten_module_entry;
#define phpext_trace_ptr &molten_module_entry

#ifdef PHP_WIN32
#   define PHP_MOLTEN_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_MOLTEN_API __attribute__ ((visibility("default")))
#else
#   define PHP_MOLTEN_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "molten_chain.h"
#include "molten_intercept.h"
#include "molten_span.h"
#include "molten_shm.h"
#include "molten_util.h"
#include "molten_ctrl.h"
#include "molten_status.h"
#include "molten_report.h"
#include "molten_stack.h"
#include "php7_wrapper.h"

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

#define molten_php_fatal_error(level, fmt_str, ...)   php_error_docref(NULL TSRMLS_CC, level, fmt_str, ##__VA_ARGS__)

PHP_MINIT_FUNCTION(molten);
PHP_MSHUTDOWN_FUNCTION(molten);
PHP_RINIT_FUNCTION(molten);
PHP_RSHUTDOWN_FUNCTION(molten);
PHP_MINFO_FUNCTION(molten);

ZEND_BEGIN_MODULE_GLOBALS(molten)
    zend_bool               enable;
    int                     level;

    long                    sampling_type;          /* sampling type */
    long                    sampling_request;       /* sampling by request one minute */
    long                    sampling_rate;          /* tracing sampling rate */
    char                    *chain_log_path;        /* chain log path */
    char                    *service_name;          /* service name */
    zend_bool               tracing_cli;            /* enable cli  tracing */
    char                    *span_format;           /* the span format */
    long                    report_interval;        /* call ctrl interval */
    long                    report_limit;           /* report limit */
    char                    *socket_host;           /* report socket host */
    int                     socket_port;            /* report socket port */
    char                    *notify_uri;            /* notify uri */
    long                    sink_type;              /* log sink type */
    long                    output_type;            /* sink spans output type */
    char                    *sink_http_uri;         /* sink http uri */
    char                    *sink_kafka_brokers;    /* sink kafka brokers */
    char                    *sink_kafka_topic;      /* sink kafka topic */
    char                    *sink_syslog_unix_socket; /* sink syslog unix */
    smart_string            request_uri;            /* request url */

    mo_chain_t              pct;                    /* chain module */
    mo_interceptor_t        pit;                    /* intercept module */
    mo_chain_log_t          pcl;                    /* log module */
    mo_span_builder         psb;                    /* span builder */
    mo_ctrl_t               prt;                    /* control module */
    mo_report_t             pre;                    /* report module */
    mo_shm_t                msm;                    /* shm module */

    mo_stack                span_stack;             /* span stack */

    char                    ip[INET_ADDRSTRLEN];    /* device ip */
    char                    host_name[HOST_NAME_MAX];   /* host name */
    long                    execute_begin_time;     /* execute begin time */
    zend_bool               enable_sapi;            /* enable_sapi */
    zend_bool               in_request;             /* determine in requeset life time */
ZEND_END_MODULE_GLOBALS(molten)


#ifdef ZEND_ENGINE_3
    /* Always refer to the globals in your function as TRACE_G(variable). You are
     * encouraged to rename these macros something shorter, see examples in any
     * other php module directory. */
    #define PTG(v) ZEND_MODULE_GLOBALS_ACCESSOR(molten, v)

    #if defined(ZTS) && defined(COMPILE_DL_MOLTEN)
    ZEND_TSRMLS_CACHE_EXTERN();
    #endif
#else
    /* In every utility function you add that needs to use variables in
     * php_molten_globals, call TSRMLS_FETCH(); after declaring other variables used
     * by that function, or better yet, pass in TSRMLS_CC after the last function
     * argument and declare your utility function with TSRMLS_DC after the last
     * declared argument.  Always refer to the globals in your function as
     * TRACE_G(variable).  You are encouraged to rename these macros something
     * shorter, see examples in any other php module directory. */
    #ifdef ZTS
    #define PTG(v) TSRMG(molten_globals_id, zend_molten_globals *, v)
    #else
    #define PTG(v) (molten_globals.v)
    #endif
#endif

#endif  /* PHP_MOLTEN_H */
