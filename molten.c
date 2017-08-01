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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_molten.h"
#include "zend_extensions.h"
#include "SAPI.h"

#include "molten_chain.h"
#include "molten_log.h"
#include "molten_util.h"
#include "php7_wrapper.h"

/* Check sapi name */
#define  CHECK_SAPI_NAME do {                                                                   \
    if ( (strncmp(sapi_module.name, "fpm-fcgi", sizeof("fpm-fcgi") -1) != 0)                    \
            && (strncmp(sapi_module.name, "apache", sizeof("apache") -1) != 0)                  \
            && (strncmp(sapi_module.name, "cli-server", sizeof("cli-server") -1) != 0)) {       \
            if ((PTG(tracing_cli) == 0) || (PTG(tracing_cli) != 0                               \
                        && strncmp(sapi_module.name, "cli", sizeof("cli") -1) != 0) ) {         \
                PTG(enable_sapi) = 0;                                                           \
                return SUCCESS;                                                                 \
            }                                                                                   \
    }                                                                                           \
}while(0)                                                                                 
PHP_FUNCTION(molten_set_service_name);
PHP_FUNCTION(molten_curl_setopt);
PHP_FUNCTION(molten_curl_exec);
PHP_FUNCTION(molten_curl_setopt_array);
PHP_FUNCTION(molten_curl_reset);

static void frame_build(mo_frame_t *frame, zend_bool internal, unsigned char type, zend_execute_data *caller, zend_execute_data *ex, zend_op_array *op_array TSRMLS_DC);
static void frame_destroy(mo_frame_t *frame);
#if PHP_VERSION_ID < 70000
static void frame_set_retval(mo_frame_t *frame, zend_bool internal, zend_execute_data *ex, zend_fcall_info *fci TSRMLS_DC);
#endif

static smart_string repr_zval(zval *zv, int limit TSRMLS_DC);

#if PHP_VERSION_ID < 50500
static void (*ori_execute)(zend_op_array *op_array TSRMLS_DC);
static void (*ori_execute_internal)(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC);
ZEND_API void mo_execute(zend_op_array *op_array TSRMLS_DC);
ZEND_API void mo_execute_internal(zend_execute_data *execute_data, int return_value_used TSRMLS_DC);
#elif PHP_VERSION_ID < 70000
static void (*ori_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
static void (*ori_execute_internal)(zend_execute_data *execute_data_ptr, zend_fcall_info *fci, int return_value_used TSRMLS_DC);
ZEND_API void mo_execute_ex(zend_execute_data *execute_data TSRMLS_DC);
ZEND_API void mo_execute_internal(zend_execute_data *execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC);
#else
static void (*ori_execute_ex)(zend_execute_data *execute_data);
static void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value);
ZEND_API void mo_execute_ex(zend_execute_data *execute_data);
ZEND_API void mo_execute_internal(zend_execute_data *execute_data, zval *return_value);
#endif

/* Replace error call back functions */
void (*trace_original_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void molten_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);

static inline zend_function *obtain_zend_function(zend_bool internal, zend_execute_data *ex, zend_op_array *op_array TSRMLS_DC);

/**
 * PHP Extension Init
 * --------------------
 */

ZEND_DECLARE_MODULE_GLOBALS(molten)

/* Make sapi_module accessable */
extern sapi_module_struct sapi_module;

/* Every user visible function must have an entry in trace_functions[]. */
const zend_function_entry molten_functions[] = {
    PHP_FE(molten_set_service_name, NULL)
    PHP_FE(molten_curl_setopt, NULL)
    PHP_FE(molten_curl_setopt_array, NULL)
    PHP_FE(molten_curl_exec, NULL)
    PHP_FE(molten_curl_reset, NULL)
#ifdef PHP_FE_END
    PHP_FE_END  /* Must be the last line in trace_functions[] */
#else
    { NULL, NULL, NULL, 0, 0 }
#endif
};

/* {{{ molten_deps
*/
static const zend_module_dep molten_deps[] = {
    ZEND_MOD_REQUIRED("json")
    ZEND_MOD_REQUIRED("standard")
    ZEND_MOD_REQUIRED("curl")
    ZEND_MOD_OPTIONAL("memcached")    
    ZEND_MOD_OPTIONAL("redis")    
    ZEND_MOD_OPTIONAL("mongodb")    
    ZEND_MOD_END
};
/* }}} */

typedef struct mo_reload_def_st {
    char *orig_func;
    char *over_func;
    char *save_func;
} mo_reload_def;

/* {{{ molten_reload_def */ 
static const mo_reload_def prd[] = {
    {"curl_setopt",         "molten_curl_setopt",         "origin_molten_curl_setopt"},
    {"curl_exec",           "molten_curl_exec",           "origin_molten_curl_exec"},
    {"curl_setopt_array",   "molten_curl_setopt_array",   "origin_molten_curl_setopt_array"},
    {"curl_reset",          "molten_curl_reset",          "origin_molten_curl_reset"},
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ origin_funtion_handler */
zend_function *origin_curl_setopt =  NULL;
zend_function *origin_curl_exec =  NULL;
zend_function *origin_curl_setopt_array =  NULL;
zend_function *origin_curl_reset = NULL;
/* }}} */


/* {{{ molten reload curl function for performance */
static void molten_reload_curl_function()
{
#if PHP_MAJOR_VERSION < 7
    zend_function *orig, *replace;
    const mo_reload_def *p = &(prd[0]);
    while(p->orig_func != NULL) {
        if (zend_hash_find(CG(function_table), p->save_func, strlen(p->orig_func) + 1, (void **)&orig) != SUCCESS) {
            zend_hash_find(CG(function_table), p->over_func, strlen(p->over_func) + 1, (void **)&replace);
            if (zend_hash_find(CG(function_table), p->orig_func, strlen(p->orig_func) + 1, (void **)&orig) == SUCCESS) {
                zend_hash_add(CG(function_table), p->save_func, strlen(p->save_func)+1, orig, sizeof(zend_function), NULL);
                zend_hash_update(CG(function_table), p->orig_func, strlen(p->orig_func) + 1, replace, sizeof(zend_function), NULL);
            }
        }
        p++;
    }
#else
    zend_function *orig, *replace;
    const mo_reload_def *p = &(prd[0]);
    while(p->orig_func != NULL) {
        if (zend_hash_str_find_ptr(CG(function_table), p->save_func, strlen(p->orig_func)) == NULL) {
            replace = zend_hash_str_find_ptr(CG(function_table), p->over_func, strlen(p->over_func));
            if ((orig = zend_hash_str_find_ptr(CG(function_table), p->orig_func, strlen(p->orig_func))) != NULL) {
                if (orig->type == ZEND_INTERNAL_FUNCTION) {
                    zend_hash_str_add_mem(CG(function_table), p->save_func, strlen(p->save_func), orig, sizeof(zend_internal_function));
                    function_add_ref(orig);
                    zend_hash_str_update_mem(CG(function_table), p->orig_func, strlen(p->orig_func), replace, sizeof(zend_internal_function));
                    function_add_ref(replace);
                }
            }
        }
        p++;
    }
#endif


    /* retrieve function from function table */
#if PHP_MAJOR_VERSION < 7
    zend_function *orig_func;
    if (zend_hash_find(CG(function_table), "origin_molten_curl_setopt", sizeof("origin_molten_curl_setopt"), (void **)&orig_func) == SUCCESS ) {
        origin_curl_setopt = orig_func;
    }
    if (zend_hash_find(CG(function_table), "origin_molten_curl_exec", sizeof("origin_molten_curl_exec"), (void **)&orig_func) == SUCCESS ) {
        origin_curl_exec = orig_func;
    }
    if (zend_hash_find(CG(function_table), "origin_molten_curl_setopt_array", sizeof("origin_molten_curl_setopt_array"), (void **)&orig_func) == SUCCESS ) {
        origin_curl_setopt_array = orig_func;
    }
    if (zend_hash_find(CG(function_table), "origin_molten_curl_reset", sizeof("origin_molten_curl_reset"), (void **)&orig_func) == SUCCESS ) {
        origin_curl_reset = orig_func;
    }
#else
    zend_function *orig_func;
    if ((orig_func = zend_hash_str_find_ptr(CG(function_table), "origin_molten_curl_setopt", sizeof("origin_molten_curl_setopt") - 1)) != NULL) {
        origin_curl_setopt = orig_func;
    }
    if ((orig_func = zend_hash_str_find_ptr(CG(function_table), "origin_molten_curl_exec", sizeof("origin_molten_curl_exec") - 1)) != NULL) {
        origin_curl_exec = orig_func;
    }
    if ((orig_func = zend_hash_str_find_ptr(CG(function_table), "origin_molten_curl_setopt_array", sizeof("origin_molten_curl_setopt_array") - 1)) != NULL ) {
        origin_curl_setopt_array = orig_func;
    }
#endif
}
/* }}} */


/* {{{ clear replace function */
static void molten_clear_reload_function()
{
    const mo_reload_def *p = &(prd[0]);
#if PHP_MAJOR_VERSION < 7
    zend_function *orig;
    while (p->orig_func != NULL) {
        if (zend_hash_find(CG(function_table), p->save_func, strlen(p->save_func)+1, (void **)&orig) == SUCCESS) {
              zend_hash_update(CG(function_table), p->orig_func, strlen(p->orig_func)+1, orig, sizeof(zend_function), NULL);
              zend_hash_del(CG(function_table), p->save_func, strlen(p->save_func)+1); 
         }
        p++;
    }
#else
    zend_function *orig;
    while (p->orig_func != NULL) {
        if ((orig = zend_hash_str_find_ptr(CG(function_table), p->save_func, strlen(p->save_func))) != NULL) {
              zend_hash_str_update_mem(CG(function_table), p->orig_func, strlen(p->orig_func), orig, sizeof(zend_internal_function));
              function_add_ref(orig);
              zend_hash_str_del(CG(function_table), p->save_func, strlen(p->save_func)); 
         }
        p++;
    }
#endif
}
/* }}} */


/* {{{ molten_curl_setopt */
PHP_FUNCTION(molten_curl_setopt)
{
    /* before */
    if (PTG(pit).curl_header_internel_call != HEADER_INTERNAL_CALL) {
#if PHP_MAJOR_VERSION < 7
    // todo remove mo_* function
        zval *zid, **zvalue;
        long        options;
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlZ", &zid, &options, &zvalue) == SUCCESS) {
            if (options == Z_LVAL(PTG(pit).curl_http_header_const) && MO_Z_TYPE_PP(zvalue) == IS_ARRAY) {
                zval *copy_header;
                MO_ALLOC_INIT_ZVAL(copy_header);
                ZVAL_ZVAL(copy_header, *zvalue, 1, 0);
                add_index_zval(PTG(pit).curl_header_record, Z_RESVAL_P(zid), copy_header);
                MO_FREE_ALLOC_ZVAL(copy_header);
            }
        }
#else
        zval *zid, *zvalue;
        zend_long  options;
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlz", &zid, &options, &zvalue) == SUCCESS) {
            if (options == Z_LVAL(PTG(pit).curl_http_header_const) && MO_Z_TYPE_P(zvalue) == IS_ARRAY) {
                zval copy_header;
                ZVAL_DUP(&copy_header, zvalue);
                add_index_zval(PTG(pit).curl_header_record, Z_RESVAL_P(zid), &copy_header);
            }
        }
#endif
    } else {
        PTG(pit).curl_header_internel_call = HEADER_OUT_CALL;
    }
   
    /* execute origin function */
    if (origin_curl_setopt != NULL) {
        origin_curl_setopt->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    /* after */
    /* nothing */
}
/* }}} */

/* {{{ molten_curl_exec */
PHP_FUNCTION(molten_curl_exec)
{
    /* before */
    zval *res;
    char *span_id;
    uint64_t entry_time;
    
    /* build span_id */
    if (PTG(pct).pch.is_sampled == 1) {
        entry_time = mo_time_usec();
        PTG(pit).span_count++;
        PTG(psb).build_span_id(&span_id, PTG(pct).pch.span_id->val, PTG(pit).span_count);
    }

    int result = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res);
    if (result == SUCCESS) {

        /* build header */
        mo_interceptor_t *pit = &PTG(pit);
        zval *tmp = NULL;
        zval *option = NULL;
        int is_init = 0;
        if (mo_zend_hash_index_zval_find(Z_ARRVAL_P(pit->curl_header_record), Z_RESVAL_P(res), (void **)&tmp) == SUCCESS) {
            option = tmp;
        } else {
            MO_ALLOC_INIT_ZVAL(option);
            array_init(option);
            is_init = 1;
        }
        
        build_http_header(pit->pct, option, span_id);

        /* curl set header internel call */
        pit->curl_header_internel_call = HEADER_INTERNAL_CALL;
        
        zval func;
        zval *argv[3];
        zval ret;
        MO_ZVAL_STRING(&func, "curl_setopt", 1);
        argv[0] = res;
        argv[1] = &pit->curl_http_header_const;
        argv[2] = option;
        mo_call_user_function(EG(function_table), (zval **)NULL, &func, &ret, 3, argv);
        zval_dtor(&ret);

        if (is_init == 1) {
#if PHP_VERSION_ID < 70000
            zval_ptr_dtor(&option);
#else
            zval_ptr_dtor(option);
#endif
            MO_FREE_ALLOC_ZVAL(option);
        }
        mo_zval_dtor(&func); 
    }
    
    /* execute origin function */
    if (origin_curl_exec != NULL) {
        origin_curl_exec->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    /* after */
    /* must sampling will do this */
    if (result == SUCCESS && PTG(pct).pch.is_sampled == 1) {
        uint64_t current_time = mo_time_usec(); 
        uint64_t duration = current_time - entry_time;
        zval *curl_span;
        PTG(psb).start_span(&curl_span, "php_curl", PTG(pct).pch.trace_id->val, span_id, PTG(pct).pch.span_id->val, entry_time, current_time, &PTG(pct), AN_CLIENT);
        build_curl_bannotation(curl_span, (long)current_time, &PTG(pit), res, "curl_exec", 1);

        /* record response if response exist */
        if (return_value != NULL && MO_Z_TYPE_P(return_value) == IS_STRING && Z_STRLEN_P(return_value) > 0) {
#define RESPONSE_MAX_LEN 128
            int response_len = Z_STRLEN_P(return_value);
            char *response_string = NULL;
            if (response_len > RESPONSE_MAX_LEN) {
                response_string = emalloc(RESPONSE_MAX_LEN);
                memset(response_string, 0x00, RESPONSE_MAX_LEN);
                strncpy(response_string, Z_STRVAL_P(return_value), RESPONSE_MAX_LEN-1);
                response_string[RESPONSE_MAX_LEN - 1] = '\0';
                PTG(psb).span_add_ba_ex(curl_span, "http.response", response_string, current_time, &PTG(pct), BA_NORMAL);
                efree(response_string);
            } else {
                PTG(psb).span_add_ba_ex(curl_span, "http.response", Z_STRVAL_P(return_value), current_time, &PTG(pct), BA_NORMAL);
            }
        }
        mo_chain_add_span(&PTG(pcl), curl_span);
        efree(span_id);
    }

}
/* }}} */

/* {{{ molten_curl_setopt_array */
PHP_FUNCTION(molten_curl_setopt_array)
{
    /* before */
    zval *zid, *arr;
    long        options;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra", &zid, &arr) == SUCCESS) {
        HashTable *ht = Z_ARRVAL_P(arr);
        zval *http_header = NULL;
        if (mo_zend_hash_index_zval_find(ht, Z_LVAL(PTG(pit).curl_http_header_const), (void **)&http_header) == SUCCESS) {
#if PHP_MAJOR_VERSION < 7
            zval *copy_header;
            MO_ALLOC_INIT_ZVAL(copy_header);
            ZVAL_ZVAL(copy_header, http_header, 1, 0);
            add_index_zval(PTG(pit).curl_header_record, Z_RESVAL_P(zid), copy_header);
            MO_FREE_ALLOC_ZVAL(copy_header);
#else
            zval copy_header;
            ZVAL_DUP(&copy_header, http_header);
            add_index_zval(PTG(pit).curl_header_record, Z_RESVAL_P(zid), &copy_header);
#endif
        }
    }
    
    /* execute origin function */
    if (origin_curl_setopt_array != NULL) {
        origin_curl_setopt_array->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    /* after */
    /* donothing */
}
/* }}} */

/* {{{ molten_curl_reset */
PHP_FUNCTION(molten_curl_reset)
{
    /* before */
    zval *zid;

    /* if reset resource, curl_header_record map will be delete */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zid) == SUCCESS) {
        HashTable *ht = Z_ARRVAL_P(PTG(pit).curl_header_record);
        mo_zend_hash_index_del(ht, Z_RESVAL_P(zid));
    }
    
    /* execute origin function */
    if (origin_curl_reset != NULL) {
        origin_curl_reset->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    /* after */
    /* donothing */
}
/* }}} */

/* {{{ molten_module_entry
*/
zend_module_entry molten_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    molten_deps,
    "molten",
    molten_functions,
    PHP_MINIT(molten),
    PHP_MSHUTDOWN(molten),
    PHP_RINIT(molten),
    PHP_RSHUTDOWN(molten),
    PHP_MINFO(molten),
#if ZEND_MODULE_API_NO >= 20010901
    MOLTEN_EXT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}}} */

#if PHP_VERSION_ID >= 70000 && defined(COMPILE_DL_MOLTEN) && defined(ZTS)
ZEND_TSRMLS_CACHE_DEFINE();
#endif

#ifdef COMPILE_DL_MOLTEN
ZEND_GET_MODULE(molten)
#endif

/* PHP_INI */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("molten.enable",                "1",            PHP_INI_SYSTEM, OnUpdateBool, enable, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.sink_log_path",        DEFAULT_PATH,   PHP_INI_SYSTEM, OnUpdateString, chain_log_path, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.service_name",          "default",      PHP_INI_SYSTEM, OnUpdateString, service_name, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.tracing_cli",           "0",            PHP_INI_SYSTEM, OnUpdateLong, tracing_cli, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.sampling_type",         "1", PHP_INI_SYSTEM, OnUpdateLong, sampling_type, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.sampling_request",      "1000",           PHP_INI_SYSTEM, OnUpdateLong, sampling_request, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.sampling_rate",         "64",          PHP_INI_SYSTEM, OnUpdateLong, sampling_rate, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.span_format",           "zipkin",       PHP_INI_SYSTEM, OnUpdateString, span_format, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.span_id_format",         "level",       PHP_INI_SYSTEM, OnUpdateString, span_id_format, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.report_interval",       "60",           PHP_INI_SYSTEM, OnUpdateLong, report_interval, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.notify_uri",       "",           PHP_INI_SYSTEM, OnUpdateString, notify_uri, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.report_limit",          "100",          PHP_INI_SYSTEM, OnUpdateLong, report_limit, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.sink_type",             "1",            PHP_INI_SYSTEM, OnUpdateLong, sink_type, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.output_type",           "1",            PHP_INI_SYSTEM, OnUpdateLong, output_type, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.sink_http_uri",         "",             PHP_INI_SYSTEM, OnUpdateString, sink_http_uri, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.sink_kafka_brokers",    "",             PHP_INI_SYSTEM, OnUpdateString, sink_kafka_brokers, zend_molten_globals, molten_globals)
    STD_PHP_INI_ENTRY("molten.sink_kafka_topic",      "",             PHP_INI_SYSTEM, OnUpdateString, sink_kafka_topic, zend_molten_globals, molten_globals)
PHP_INI_END()

/* php_trace_init_globals */
static void php_trace_init_globals(zend_molten_globals *ptg)
{
    ptg->enable_sapi = 1;
    ptg->in_request = 0;
}

/* {{{ PHP MINIT Function */
PHP_MINIT_FUNCTION(molten)
{
    ZEND_INIT_MODULE_GLOBALS(molten, php_trace_init_globals, NULL);
    REGISTER_INI_ENTRIES();

    if (!PTG(enable)) {
        return SUCCESS;
    }

    CHECK_SAPI_NAME;

    /* Replace executor */
#if PHP_VERSION_ID < 50500
    ori_execute = zend_execute;
    zend_execute = mo_execute;
#else
    ori_execute_ex = zend_execute_ex;
    zend_execute_ex = mo_execute_ex;
#endif
    ori_execute_internal = zend_execute_internal;
    zend_execute_internal = mo_execute_internal;

    /* Overload function */
    molten_reload_curl_function();

    /* Replace error call back */
    trace_original_error_cb = zend_error_cb;
    zend_error_cb = molten_error_cb;
    
    /* Set common data */
    /* http request */
    PTG(pct).sapi = sapi_module.name;

    /* almost user use fpm or cli */
    if ((strcmp(PTG(pct).sapi, "cli") == 0)) {
        PTG(pct).is_cli = 1;
    } else {
        PTG(pct).is_cli = 0;
    }

    /* module ctor */
    mo_obtain_local_ip(PTG(ip));
    mo_shm_ctor(&PTG(msm));   
    mo_ctrl_ctor(&PTG(prt), &PTG(msm), PTG(notify_uri), PTG(ip), PTG(sampling_type), PTG(sampling_rate), PTG(sampling_request));
    mo_span_ctor(&PTG(psb), PTG(span_format), PTG(span_id_format));
    mo_chain_log_ctor(&PTG(pcl), PTG(chain_log_path), PTG(sink_type), PTG(output_type), PTG(sink_http_uri));
    mo_intercept_ctor(&PTG(pit), &PTG(pct), &PTG(psb));
    mo_rep_ctor(&PTG(pre), PTG(report_interval), PTG(report_limit));

    return SUCCESS;
}
/* }}} */

/* {{{ PHP MSHUTDOWN Function */
PHP_MSHUTDOWN_FUNCTION(molten)
{
    UNREGISTER_INI_ENTRIES();

    if (!PTG(enable)) {
        return SUCCESS;
    }
    
    CHECK_SAPI_NAME;

    /* Restore original executor */
#if PHP_VERSION_ID < 50500
    zend_execute = ori_execute;
#else
    zend_execute_ex = ori_execute_ex;
#endif
    zend_execute_internal = ori_execute_internal;

    zend_error_cb = trace_original_error_cb;

    /* Clear overload function */
    molten_clear_reload_function();
    
    /* module dtor */
    mo_shm_dtor(&PTG(msm));   
    mo_ctrl_dtor(&PTG(prt));
    mo_chain_log_dtor(&PTG(pcl));
    mo_intercept_dtor(&PTG(pit));
    mo_rep_dtor(&PTG(pre));

    return SUCCESS;
}
/* }}} */

/* {{{ PHP RINIT Function */
PHP_RINIT_FUNCTION(molten)
{
#if PHP_VERSION_ID >= 70000 && defined(COMPILE_DL_MOLTEN) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    if (!PTG(enable)) {
        return SUCCESS;
    }

    if (!PTG(enable_sapi)) {
        return SUCCESS;
    }

    /* Set in request life time */
    PTG(in_request) = 1;
    
    /* execute begin time */
    PTG(execute_begin_time) = (long) SG(global_request_time) * 1000000;

    /* Join domain and path */
    join_ori_url(&PTG(request_uri), 1);

    /* Output status */
    mo_request_handle(&PTG(prt));

    /* Judge sampling */ 
    mo_ctrl_sampling(&PTG(prt), &PTG(pct));

    /* Tracing basic info generate */
    mo_chain_ctor(&PTG(pct), &PTG(pcl), &PTG(psb), PTG(service_name), PTG(ip));

    /* Init  intercept module */
    mo_intercept_init(&PTG(pit));

    /* Init log module */
    if (PTG(pct).pch.is_sampled == 1) {
        mo_chain_log_init(&PTG(pcl));
    }
    
    return SUCCESS;
}
/* }}} */

/* {{{ PHP RSHUTDOWN Function */
PHP_RSHUTDOWN_FUNCTION(molten)
{
    if (!PTG(enable)) {
        return SUCCESS;
    }

    if (!PTG(enable_sapi)) {
        return SUCCESS;
    }

    /* all actions are the reverse of RINIT */
    /* dtor tracing basic info */
    /* Set out request life time */
    PTG(in_request) = 0;
    
    /* Chain dtor */
    mo_chain_dtor(&PTG(pct), &PTG(psb));

    /* Flush and dtor log */
    if (PTG(pct).pch.is_sampled == 1) {
        mo_chain_log_flush(&PTG(pcl));
    }

    /* Ctrl record */
    mo_ctrl_record(&PTG(prt), PTG(pct).pch.is_sampled);

    /* Report some info*/
    mo_rep_record_data(&PTG(pre), PTG(prt).mri, &PTG(pcl), &PTG(request_uri), PTG(pct).pch.is_sampled, PTG(execute_begin_time));

    /* Uninit intercept module */
    mo_intercept_uninit(&PTG(pit));

    /* Free url */
    smart_string_free(&PTG(request_uri));

    return SUCCESS;
}
/* }}} */

/* {{{ PHP MINFO Function */
PHP_MINFO_FUNCTION(molten)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "molten support", "enabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ Ptracing set service name */
PHP_FUNCTION(molten_set_service_name)
{      
#if PHP_VERSION_ID < 70000
    char *service_name;
    int service_name_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &service_name, &service_name_len) == FAILURE) {
        RETURN_FALSE;   
    }
#else
    zend_string *service_name;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &service_name) == FAILURE) {
        RETURN_FALSE;   
    }
#endif
    efree(PTG(pct).service_name);
    PTG(pct).service_name = estrdup(MO_STR(service_name)); 
    RETURN_TRUE;
}
/* }}} */

/* {{{ Obtain zend function */
static inline zend_function *obtain_zend_function(zend_bool internal, zend_execute_data *ex, zend_op_array *op_array TSRMLS_DC)
{
#if PHP_VERSION_ID < 50500
    if (internal || ex) {
        return ex->function_state.function;
    } else {
        return (zend_function *) op_array;
    }
#elif PHP_VERSION_ID < 70000
    return ex->function_state.function;
#else
    return ex->func;
#endif
}
/* }}} */

/* {{{ Destroy frame */
static void frame_destroy(mo_frame_t *frame)
{
    int i;

    smart_string_free(&frame->class);
    smart_string_free(&frame->function);
    if (frame->args && frame->arg_count) {
        for (i = 0; i < frame->arg_count; i++) {
            smart_string_free(&frame->args[i]);
        }
        efree(frame->args);
    }
    efree(frame->span_id);
}
/* }}} */

/* {{{ Build molten frame */
static void frame_build(mo_frame_t *frame, zend_bool internal, unsigned char type, zend_execute_data *caller, zend_execute_data *ex, zend_op_array *op_array TSRMLS_DC)
{
    unsigned int i;
    zval **args;
    zend_function *zf;

    /* init */
    memset(frame, 0, sizeof(mo_frame_t));

#if PHP_VERSION_ID < 50500
    if (internal || ex) {
        op_array = ex->op_array;
    }
#endif

    /* zend function */
    zf = obtain_zend_function(internal, ex, op_array);

    /* types, level */
    frame->type = type;
    frame->level = PTG(level);

    /* args init */
    args = NULL;
    frame->arg_count = 0;
    frame->args = NULL;

    /* class name */
#if PHP_VERSION_ID < 70000
    if (caller && MO_EX_OBJ(caller)) {
#else
    if (ex && MO_EX_OBJ(ex)) {
#endif
        if (zf->common.scope) {
            smart_string_appends(&frame->class, MO_STR(zf->common.scope->name));
        }
    } else if (zf->common.scope) {
        smart_string_appends(&frame->class, MO_STR(zf->common.scope->name));
    }

    /* function name */
    if (zf->common.function_name) {
        smart_string_appends(&frame->function, MO_STR(zf->common.function_name));
    } 
#if PHP_VERSION_ID >= 50414
    if (zf->common.scope && zf->common.scope->trait_aliases) {
        /* Use trait alias instead real function name.
         * There is also a bug "#64239 Debug backtrace changed behavior
         * since 5.4.10 or 5.4.11" about this
         * https://bugs.php.net/bug.php?id=64239.*/
        smart_string_appends(&frame->function, MO_STR(zend_resolve_method_name(MO_EX_OBJ(ex) ? MO_EX_OBJCE(ex) : zf->common.scope, zf)));
    }
#endif

    /* obj */
    frame->object = MO_EX_OBJ_ZVAL(ex);

    /* args */
#if PHP_VERSION_ID < 50300
    if (EG(argument_stack).top >= 2) {
        frame->arg_count = (int)(zend_uintptr_t) *(EG(argument_stack).top_element - 2);
        args = (zval **)(EG(argument_stack).top_element - 2 - frame->arg_count);
    }
#elif PHP_VERSION_ID < 70000
    if (caller && caller->function_state.arguments) {
        frame->arg_count = (int)(zend_uintptr_t) *(caller->function_state.arguments);
        args = (zval **)(caller->function_state.arguments - frame->arg_count);
    }
#else
    frame->arg_count = ZEND_CALL_NUM_ARGS(ex);
#endif

    if (frame->arg_count > 0) {
        frame->args = ecalloc(frame->arg_count, sizeof(smart_string));
    }
#if PHP_VERSION_ID < 70000
    for (i = 0; i < frame->arg_count; i++) {
        frame->args[i] = repr_zval(args[i], 32 TSRMLS_CC);
    }
    frame->ori_args = args;
#else
    if (frame->arg_count) {
        i = 0;
        zval *p = ZEND_CALL_ARG(ex, 1);
        if (ex->func->type == ZEND_USER_FUNCTION) {
            uint32_t first_extra_arg = ex->func->op_array.num_args;

            if (first_extra_arg && frame->arg_count > first_extra_arg) {
                while (i < first_extra_arg) {
                    frame->args[i++] = repr_zval(p++, 32);
                }
                p = ZEND_CALL_VAR_NUM(ex, ex->func->op_array.last_var + ex->func->op_array.T);
            }
        }
        frame->ori_args = p;
        while(i < frame->arg_count) {
            frame->args[i++] = repr_zval(p++, 32);
        }
    }
#endif

    smart_string_0(&frame->class);
    smart_string_0(&frame->function);
    PTG(pit).span_count++;
    PTG(psb).build_span_id(&frame->span_id, PTG(pct).pch.span_id->val, PTG(pit).span_count);
}
/* }}} */

/* {{{ Build Frame Return Val */
#if PHP_VERSION_ID < 70000
static void frame_set_retval(mo_frame_t *frame, zend_bool internal, zend_execute_data *ex, zend_fcall_info *fci TSRMLS_DC)
{
    zval *retval = NULL;

    if (internal) {
        /* Ensure there is no exception occurs before fetching return value.
         * opline would be replaced by the Exception's opline if exception was
         * thrown which processed in function zend_throw_exception_internal().
         * It may cause a SEGMENTATION FAULT if we get the return value from a
         * exception opline. */
#if PHP_VERSION_ID >= 50500
        if (fci != NULL) {
            retval = *fci->retval_ptr_ptr;
        } else if (ex->opline && !EG(exception)) {
            retval = EX_TMP_VAR(ex, ex->opline->result.var)->var.ptr;
        }
#else
        if (ex->opline && !EG(exception)) {
#if PHP_VERSION_ID < 50400
            retval = ((temp_variable *)((char *) ex->Ts + ex->opline->result.u.var))->var.ptr;
#else
            retval = ((temp_variable *)((char *) ex->Ts + ex->opline->result.var))->var.ptr;
#endif
        }
#endif
    } else if (*EG(return_value_ptr_ptr)) {
        retval = *EG(return_value_ptr_ptr);
    }

    if (retval) {
        frame->ori_ret = retval;
    }
}
#endif
/* }}} */

/* {{{ Ptrace GenVal Function */
static smart_string repr_zval(zval *zv, int limit TSRMLS_DC)
{
    int tlen = 0;
    char buf[256] = {0}, *tstr = NULL;
    smart_string result = {0};

#if PHP_VERSION_ID >= 70000
    zend_string *class_name;
#endif

    /* php_var_export_ex is a good example */
    switch (Z_TYPE_P(zv)) {
#if PHP_VERSION_ID < 70000
        case IS_BOOL:
            if (Z_LVAL_P(zv)) {
                smart_string_appends(&result, "true");
                return result;
            } else {
                smart_string_appends(&result, "false");
                return result;
            }
#else
        case IS_TRUE:
            smart_string_appends(&result, "true");
            return result;
        case IS_FALSE:
            smart_string_appends(&result, "false");
            return result;
#endif
        case IS_NULL:
            smart_string_appends(&result, "NULL");
            return result;
        case IS_LONG:
            snprintf(buf, sizeof(buf), "%ld", Z_LVAL_P(zv));
            smart_string_appends(&result, buf);
            return result;
        case IS_DOUBLE:
            snprintf(buf, sizeof(buf), "%.*G", (int) EG(precision), Z_DVAL_P(zv));
            smart_string_appends(&result, buf);
            return result;
        case IS_STRING:
            tlen = (limit <= 0 || Z_STRLEN_P(zv) < limit) ? Z_STRLEN_P(zv) : limit;
            smart_string_appendl(&result, Z_STRVAL_P(zv), tlen);
            if (limit > 0 && Z_STRLEN_P(zv) > limit) {
                smart_string_appends(&result, "...");
            }
            return result;
        case IS_ARRAY:
            /* TODO more info */
            snprintf(buf, sizeof(buf), "array(%d)", zend_hash_num_elements(Z_ARRVAL_P(zv)));
            smart_string_appends(&result, buf);
            return result;
        case IS_OBJECT:
#if PHP_VERSION_ID < 70000
            if (Z_OBJ_HANDLER(*zv, get_class_name)) {
                Z_OBJ_HANDLER(*zv, get_class_name)(zv, (const char **) &tstr, (zend_uint *) &tlen, 0 TSRMLS_CC);
                snprintf(buf, sizeof(buf), "object(%s)#%d", tstr, Z_OBJ_HANDLE_P(zv));
                smart_string_appends(&result, buf);
                efree(tstr);
            } else {
                snprintf(buf, sizeof(buf), "object(unkown)#%d", Z_OBJ_HANDLE_P(zv));
                smart_string_appends(&result, buf);
            }
#else
            class_name = Z_OBJ_HANDLER_P(zv, get_class_name)(Z_OBJ_P(zv));
            snprintf(buf, sizeof(buf), "object(%s)#%d", MO_STR(class_name), Z_OBJ_HANDLE_P(zv));
            smart_string_appends(&result, buf);
            zend_string_release(class_name);
#endif
            return result;
        case IS_RESOURCE:
#if PHP_VERSION_ID < 70000
            tstr = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_P(zv) TSRMLS_CC);
            snprintf(buf, sizeof(buf), "resource(%s)#%d", tstr ? tstr : "Unknown", Z_LVAL_P(zv));
            smart_string_appends(&result, buf);
            return result;
#else
            tstr = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(zv) TSRMLS_CC);
            snprintf(buf, sizeof(buf), "resource(%s)#%d", tstr ? tstr : "Unknown", Z_RES_P(zv)->handle);
            smart_string_appends(&result, buf);
            return result;
        case IS_UNDEF:
            smart_string_appends(&result, "{undef}");
            return result;
#endif
        default:
            smart_string_appends(&result, "{unknown}");
            return result;
    }
}
/* }}} */


/* {{{ Replace executor core */
#if PHP_VERSION_ID < 50500
ZEND_API void mo_execute_core(int internal, zend_execute_data *execute_data, zend_op_array *op_array, int rvu TSRMLS_DC)
#elif PHP_VERSION_ID < 70000
ZEND_API void mo_execute_core(int internal, zend_execute_data *execute_data, zend_fcall_info *fci, int rvu TSRMLS_DC)
#else
ZEND_API void mo_execute_core(int internal, zend_execute_data *execute_data, zval *return_value)
#endif
{
    zend_bool dobailout = 0;
    zend_execute_data *caller = execute_data;
#if PHP_VERSION_ID < 70000
    zval *retval = NULL;
#else
    zval retval;
#endif
    mo_frame_t frame;

#if PHP_VERSION_ID >= 70000
    if (execute_data->prev_execute_data) {
        caller = execute_data->prev_execute_data;
    }
#elif PHP_VERSION_ID >= 50500
    /* In PHP 5.5 and later, execute_data is the data going to be executed, not
     * the entry point, so we switch to previous data. The internal function is
     * a exception because it's no need to execute by op_array. */
    if (!internal && execute_data->prev_execute_data) {
        caller = execute_data->prev_execute_data;
    }
#endif

    /* Assigning to a LOCAL VARIABLE at begining to prevent value changed
     * during executing. And whether send frame mesage back is controlled by
     * GLOBAL VALUE and LOCAL VALUE both because comm-module may be closed in
     * recursion and sending on exit point will be affected. */

    PTG(level)++;

    zend_bool match_intercept = 1; 
    mo_interceptor_ele_t *i_ele;

    /* If not in request life time, we do nothing. because of the we not init the info
     * for record call stack.
     * if not sampled, we do nothing
     */
    if ((0 == PTG(pct).pch.is_sampled) || (0 == PTG(in_request))) {
        match_intercept = 0;
    }

    if (match_intercept) {
#if PHP_VERSION_ID < 50500
        zend_function *zf = obtain_zend_function(internal, execute_data, op_array);
#else 
        zend_function *zf = obtain_zend_function(internal, execute_data, NULL);
#endif
        const char *class_name = (zf->common.scope != NULL && zf->common.scope->name != NULL)  ? MO_STR(zf->common.scope->name) : NULL;
        const char *function_name = zf->common.function_name == NULL ? NULL : MO_STR(zf->common.function_name);
        match_intercept = mo_intercept_hit(&PTG(pit), &i_ele, (char *)class_name, (char *)function_name);
    }

    if (match_intercept) {
#if PHP_VERSION_ID < 50500
        frame_build(&frame, internal, MO_FRAME_ENTRY, caller, execute_data, op_array TSRMLS_CC);
#else
        frame_build(&frame, internal, MO_FRAME_ENTRY, caller, execute_data, NULL TSRMLS_CC);
#endif

        /* Register return value ptr */
#if PHP_VERSION_ID < 70000
        if (!internal && EG(return_value_ptr_ptr) == NULL) {
            EG(return_value_ptr_ptr) = &retval;
        }
#else
        if (!internal && execute_data->return_value == NULL) {
            ZVAL_UNDEF(&retval);
#if PHP_VERSION_ID < 70100
            Z_VAR_FLAGS(retval) = 0;
#endif
            execute_data->return_value = &retval;
        }
#endif

        frame.entry_time = mo_time_usec();
    }

    /* Call original under zend_try. baitout will be called when exit(), error
     * occurs, exception thrown and etc, so we have to catch it and free our
     * resources. */
    zend_try {
#if PHP_VERSION_ID < 50500
        if (internal) {
            if (ori_execute_internal) {
                ori_execute_internal(execute_data, rvu TSRMLS_CC);
            } else {
                execute_internal(execute_data, rvu TSRMLS_CC);
            }
        } else {
            ori_execute(op_array TSRMLS_CC);
        }
#elif PHP_VERSION_ID < 70000
        if (internal) {
            if (ori_execute_internal) {
                ori_execute_internal(execute_data, fci, rvu TSRMLS_CC);
            } else {
                execute_internal(execute_data, fci, rvu TSRMLS_CC);
            }
        } else {
            ori_execute_ex(execute_data TSRMLS_CC);
        }
#else
        if (internal) {
            if (ori_execute_internal) {
                ori_execute_internal(execute_data, return_value);
            } else {
                execute_internal(execute_data, return_value);
            }
        } else {
            ori_execute_ex(execute_data);
        }
#endif
    } zend_catch {
        dobailout = 1;
        /* call zend_bailout() at the end of this function, we still want to
         * send message. */
    } zend_end_try();

    if (match_intercept) {
        frame.exit_time = mo_time_usec();

        if (!dobailout) {
#if PHP_VERSION_ID < 50500
            frame_set_retval(&frame, internal, execute_data, NULL TSRMLS_CC);
#elif PHP_VERSION_ID < 70000
            frame_set_retval(&frame, internal, execute_data, fci TSRMLS_CC);
#else
            if (return_value) { /* internal */
                frame.ori_ret = return_value;
            } else if (execute_data->return_value) { /* user function */
                frame.ori_ret = execute_data->return_value;
            }
#endif
        }

        frame.type = MO_FRAME_EXIT;

        /* Record call result */
        i_ele->record(&PTG(pit), &frame);  

#if PHP_VERSION_ID < 70000
        /* Free return value */
        if (!internal && retval != NULL) {
            zval_ptr_dtor(&retval);
            EG(return_value_ptr_ptr) = NULL;
        }
#endif
        frame_destroy(&frame);
    }

    PTG(level)--;

    if (dobailout) {
        zend_bailout();
    }
}

#if PHP_VERSION_ID < 50500
ZEND_API void mo_execute(zend_op_array *op_array TSRMLS_DC)
{
    mo_execute_core(0, EG(current_execute_data), op_array, 0 TSRMLS_CC);
}

ZEND_API void mo_execute_internal(zend_execute_data *execute_data, int return_value_used TSRMLS_DC)
{
    mo_execute_core(1, execute_data, NULL, return_value_used TSRMLS_CC);
}
#elif PHP_VERSION_ID < 70000
ZEND_API void mo_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
{
    mo_execute_core(0, execute_data, NULL, 0 TSRMLS_CC);
}

ZEND_API void mo_execute_internal(zend_execute_data *execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC)
{
    mo_execute_core(1, execute_data, fci, return_value_used TSRMLS_CC);
}
#else
ZEND_API void mo_execute_ex(zend_execute_data *execute_data)
{
    mo_execute_core(0, execute_data, NULL);
}

ZEND_API void mo_execute_internal(zend_execute_data *execute_data, zval *return_value)
{
    mo_execute_core(1, execute_data, return_value);
}
#endif

/* {{{ Ptracing Custom Error CallBack */
void molten_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
    TSRMLS_FETCH();
    error_handling_t  error_handling;
    char *error_info = NULL;
    int total_len = 200;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
    error_handling  = EG(error_handling);
#else
    error_handling  = PG(error_handling);
#endif
    
    /* record error */
    if (error_handling == EH_NORMAL) {
        switch (type) {
            case E_ERROR:
            //case E_WARNING:
            case E_CORE_ERROR:
            //case E_CORE_WARNING:
            case E_USER_ERROR:
            //case E_USER_WARNING:
                error_info = emalloc(total_len);
                bzero(error_info, total_len); 
                int pos = snprintf(error_info, total_len, "type:%d, file:%s, line:%d ", type, error_filename, error_lineno);
                if (pos < total_len-1) {
                    va_list copy_args;
                    va_copy(copy_args, args);
                    vslprintf(error_info + pos, total_len-pos-1, format, copy_args);
                    va_end(copy_args);
                }
                error_info[total_len-1] = '\0';

                /* set to sampled error string */
                if (PTG(pct).pch.is_sampled == 1 && PTG(in_request) == 1) {
                    mo_add_next_index_string(PTG(pct).error_list, error_info, 1);
                }

                /* report */
                mo_rep_record_error(&PTG(pre), &PTG(request_uri), error_info, PTG(execute_begin_time));

                efree(error_info);
        }
    }

    trace_original_error_cb(type, error_filename, error_lineno, format, args);
}
/* }}} */
