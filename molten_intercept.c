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
#include "php.h"
#include "ext/pdo/php_pdo_driver.h"
#include "ext/mysqli/php_mysqli_structs.h"
#include "ext/mysqlnd/mysqlnd_structs.h"
#include "ext/pcre/php_pcre.h"
#include "Zend/zend_exceptions.h"

#include "molten_intercept.h"
#include "molten_log.h"
#include "molten_util.h"
#include "molten_chain.h"

#define RETURN_Z_STRING(zval) (zval && (MO_Z_TYPE_P(zval) == IS_STRING) ? Z_STRVAL_P(zval) : "")
#define RETURN_Z_LONG(zval) (zval && (MO_Z_TYPE_P(zval) == IS_LONG) ? Z_LVAL_P(zval) : 0)

/* pt intercept hit function */
zend_bool mo_intercept_hit(mo_interceptor_t *pit, mo_interceptor_ele_t **eleDest, char *class_name, char *function_name)
{
    mo_interceptor_ele_t *ele;
    /* when the intercept set capture, if not sampled, the hit will return 1 
     * if intercept don`t set capture, if not sampled, the hit will return 0
     */
    /* match name by tag */
    if (class_name != NULL) {
        if (mo_zend_hash_find(pit->tags, class_name, strlen(class_name) + 1, (void **)&ele) != SUCCESS) {
            return 0;
        }
    } else if (function_name != NULL) {
        if (mo_zend_hash_find(pit->tags, function_name, strlen(function_name) + 1, (void **)&ele) != SUCCESS) {
            return 0;
        }
    } else {
        return 0;
    }
    
    smart_string          k = {0};
    int ret = 0;

    /* match */
    if (class_name != NULL) {
        smart_string_appends(&k, class_name);
        smart_string_appendl(&k, "@", sizeof("@") - 1);
        smart_string_appends(&k, function_name);
        smart_string_0(&k);
    } else {
        smart_string_appends(&k, function_name);
        smart_string_0(&k);
    }

     if (mo_zend_hash_find(pit->elements, smart_string_str(k), smart_string_len(k) + 1, (void **)&ele) != SUCCESS) {
         ret = 0;
    } else {
         *eleDest = ele;
         ret = 1;
    }
    smart_string_free(&k);

    return ret;
}

#if PHP_VERSION_ID < 70000
static void hash_destroy_cb(void *pDest)
#else
static void hash_destroy_cb(zval *pDest)
#endif
{
    mo_interceptor_ele_t **pie = (mo_interceptor_ele_t **)pDest;
    pefree(*pie, 1);
}

static char *convert_args_to_string(mo_frame_t *frame)
{
    int i = 0;
    int arg_len = 0;
#define ARGS_MAX_LEN 64
#define ARGS_ELLIPSIS "..."
#define ARGS_ELLIPSIS_LEN (sizeof("...") - 1)
#define ARGS_REAL_LEN (ARGS_MAX_LEN - ARGS_ELLIPSIS_LEN - 1)

    char *string = emalloc(ARGS_MAX_LEN);
    int real_len = 0;
    int stop = 0;
    memset(string, 0x00, ARGS_MAX_LEN);
    arg_len = smart_string_len(frame->function) + 1; 
    string = strncat(string, smart_string_str(frame->function), real_len - 1);
    string = strncat(string, " ", 1);

    for (; i < frame->arg_count; i++) {
        real_len = smart_string_len(frame->args[i])  + 1;
        if ((arg_len + real_len) >= ARGS_REAL_LEN)  {
            real_len = ARGS_REAL_LEN - arg_len;
            string = strncat(string, smart_string_str(frame->args[i]), real_len - 1);
            stop = 1;  
            break;
        } else {
            string = strncat(string, smart_string_str(frame->args[i]), real_len - 1);
            string = strncat(string, ",", 1);
            arg_len += real_len;
        }
    }
    
    if (stop == 1) {
        string = strncat(string, ARGS_ELLIPSIS, ARGS_ELLIPSIS_LEN);
        string[ARGS_MAX_LEN - 1] = '\0';
    }

    return string;
}

#if PHP_VERSION_ID < 70000
#define GET_FUNC_ARG(param, arg_num)    zval *param = (frame->ori_args)[arg_num]
#define GET_FUNC_ARG_UNDEC(param, arg_num) param = (frame->ori_args)[arg_num]
#else
#define GET_FUNC_ARG(param, arg_num)    zval *param = ((zval *)(frame->ori_args) + arg_num)
#define GET_FUNC_ARG_UNDEC(param, arg_num) param = ((zval *)(frame->ori_args) + arg_num)
#endif

/* {{{ Build common record */
static zval *build_com_record(mo_interceptor_t *pit, mo_frame_t *frame, int add_args)
{
    zval *span;
    pit->psb->start_span_ex(&span, smart_string_str(frame->function), pit->pct, frame, AN_CLIENT);

    if (add_args == 1) {
        char *value = convert_args_to_string(frame);                                                                
        pit->psb->span_add_ba_ex(span,  smart_string_str(frame->function), value, frame->exit_time, pit->pct, BA_NORMAL);
        efree(value);
    }
    return span;
}
/* }}} */

/********************curl_multi_add_handle*************************/
static void curl_multi_add_handle_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    if (frame->arg_count < 2) {
        return;
    }
    GET_FUNC_ARG(ch,1);
    if (MO_Z_TYPE_P(ch) != IS_RESOURCE) {
        return;
    }
    
    add_index_long(pit->span_info_cache, Z_RESVAL_P(ch), frame->entry_time); 
}

static void curl_multi_remove_handle_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    if (frame->arg_count < 2) {
        return;
    }
    GET_FUNC_ARG(ch,1);
    if (MO_Z_TYPE_P(ch) != IS_RESOURCE) {
        return;
    }
    zval *start_time = NULL;
    if (mo_zend_hash_index_zval_find(Z_ARRVAL_P(pit->span_info_cache), Z_RESVAL_P(ch), (void **)&start_time) == SUCCESS) {
        if (MO_Z_TYPE_P(start_time) == IS_LONG) {
            zval *curl_span;
            pit->psb->start_span(&curl_span, "php_curl_multi", pit->pct->pch.trace_id->val, frame->span_id, pit->pct->pch.span_id->val, Z_LVAL_P(start_time), frame->exit_time, pit->pct, AN_CLIENT);
            build_curl_bannotation(curl_span, frame->exit_time, pit, ch, "curl_multi_exec", 1);
            mo_chain_add_span(pit->pct->pcl, curl_span);
        }
    }
}

#define LOAD_OMO_HTTPHEADER_VAL do {                                                                                                \
    if (Z_LVAL(pit->curl_http_header_const) == -1) {                                                                                \
        if (mo_zend_get_constant("CURLOPT_HTTPHEADER", sizeof("CURLOPT_HTTPHEADER") - 1, &pit->curl_http_header_const) == 0) {      \
            return;                                                                                                                 \
        }                                                                                                                           \
    }                                                                                                                               \
}while(0)

/* {{{ Build curl bannotation */
void build_curl_bannotation(zval *span, long timestamp, mo_interceptor_t *pit, zval *handle, char *method, zend_bool check_error) 
{
    zval func;
    zval *args[1];
    args[0] = handle;
    long err = 0;
    char *errstr = NULL;
    zval *url = NULL; 
    zval *http_code = NULL;
    zval *primary_ip = NULL;
    zval *primary_port = NULL;
    MO_ZVAL_STRING(&func, "curl_getinfo", 1);
    zval ret1;
    zval ret;
    int result = mo_call_user_function(EG(function_table), (zval **)NULL, &func, &ret1, 1, args);   
    if (result == SUCCESS) {
        if (Z_TYPE(ret1) == IS_ARRAY) {
           if (mo_zend_hash_zval_find(Z_ARRVAL(ret1), "url", sizeof("url"), (void **)&url) == FAILURE) {
                url = NULL;
           }

           if (mo_zend_hash_zval_find(Z_ARRVAL(ret1), "http_code", sizeof("http_code"), (void **)&http_code) == FAILURE) {
                http_code = NULL;
           }
            
           if (mo_zend_hash_zval_find(Z_ARRVAL(ret1), "primary_ip", sizeof("primary_ip"), (void **)&primary_ip) == FAILURE) {
                primary_ip = NULL;
           }

           if (mo_zend_hash_zval_find(Z_ARRVAL(ret1), "primary_port", sizeof("primary_port"), (void **)&primary_port) == FAILURE) {
                primary_port = NULL;
           }
        }
    }
    mo_zval_dtor(&func);

    /* http.url */
    pit->psb->span_add_ba_ex(span, "http.url", RETURN_Z_STRING(url), timestamp, pit->pct, BA_NORMAL);

    if (check_error == 1) {
        /* curl_error */
        MO_ZVAL_STRING(&func, "curl_error", 1);
        result = mo_call_user_function(EG(function_table), (zval **)NULL, &func, &ret, 1, args);
        if (result == SUCCESS) {
            if (Z_TYPE(ret) == IS_STRING && Z_STRLEN(ret) > 0) {
                errstr = estrdup(Z_STRVAL(ret)); 
            } else {
                errstr = NULL;
            }
            mo_zval_dtor(&ret);
        }
        mo_zval_dtor(&func); 
        if (errstr != NULL) {
            /* error */
            pit->psb->span_add_ba_ex(span, "error", errstr, timestamp, pit->pct, BA_ERROR);
        } else {
            /* http.status */
            char tmp_string[32];
            sprintf(tmp_string, "%ld", RETURN_Z_LONG(http_code));

            /* not set the ip and port to sa */ 
            pit->psb->span_add_ba_ex(span, "http.status", tmp_string, timestamp, pit->pct, BA_NORMAL);
        }
    }

    if (errstr != NULL) efree(errstr);
    mo_zval_dtor(&ret1);
}
/* }}} */

/* {{{ curl multi exec intercept now discard */
/*
static void curl_multi_exec_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    zval *ret = frame->ori_ret;
    GET_FUNC_ARG(fir_arg,0);
    GET_FUNC_ARG(sec_arg,1);
    if (Z_LVAL(pit->curlm_call_multi_perform) == -10) {
        if (mo_zend_get_constant("curlm_call_multi_perform", sizeof("curlm_call_multi_perform") - 1, &pit->curlm_call_multi_perform) == 0) {
            return;
        }
    }

    if ((MO_Z_TYPE_P(ret) != IS_LONG) || (Z_LVAL_P(ret) == Z_LVAL(pit->curlm_call_multi_perform) || Z_LVAL_P(sec_arg) != 0)) {
        return;
    }

    zval *span;
    pit->psb->start_span_ex(&span, "http", pit->pct, frame, AN_CLIENT);
    if (MO_Z_TYPE_P(ret) != IS_FALSE && MO_Z_TYPE_P(fir_arg) == IS_RESOURCE) {
        HashTable *ht = Z_ARRVAL_P(pit->curl_multi_handlers); 
        zval *multi_handle; 
        if (mo_zend_hash_index_find(ht, Z_RESVAL_P(fir_arg), (void **)&multi_handle) == SUCCESS) {
            zval *val;
            HashTable *curl_handle = Z_ARRVAL_P(multi_handle);
            for(zend_hash_internal_pointer_reset(curl_handle); 
                    zend_hash_has_more_elements(curl_handle) == SUCCESS;
                    zend_hash_move_forward(curl_handle)) {
                if (mo_zend_hash_get_current_data(curl_handle, (void **)&val) == SUCCESS) {
                    zval *val_res;
                    MO_MAKE_STD_ZVAL(val_res);
                    ZVAL_RESOURCE(val_res, Z_RESVAL_P(val));
                    build_curl_bannotation(span, frame->exit_time, pit, val_res, "curl_multi_exec", 1);
                    FREE_ZVAL(val_res);
                }
            }
        }
    }
    mo_chain_add_span(pit->pct->pcl, span);
}
*/
/* }}} */

#if PHP_VERSION_ID < 70000
#define GET_PDO_DBH  pdo_dbh_t *dbh = zend_object_store_get_object(object);
#else
#define GET_PDO_DBH  pdo_dbh_t *dbh = Z_PDO_DBH_P(object);
#endif                                                                                                                                      
#if PHP_VERSION_ID < 70000
#define SET_SPAN_EXCEPTION(exception_ce,pit,frame,service_name,host,port)          do {                                                   \
        if (instanceof_function(Z_OBJCE_P(EG(exception)), exception_ce) == 1) {                                                 \
            zval *message = mo_zend_read_property(exception_ce, EG(exception), "message", sizeof("message") - 1, 1);            \
            pit->psb->span_add_ba(span, "error", RETURN_Z_STRING(message), frame->exit_time, service_name, host, port, BA_ERROR);                       \
           }                                                                \
}while(0)
#else
#define SET_SPAN_EXCEPTION(exception_ce, pit, frame, service_name, host, port)          do {                                                   \
        if (instanceof_function(EG(exception)->ce, exception_ce) == 1) {                            \
            zval tmp;                                                                               \
            ZVAL_OBJ(&tmp, EG(exception));                                                          \
            zval *message = mo_zend_read_property(exception_ce, &tmp, "message", sizeof("message") - 1, 1);      \
            pit->psb->span_add_ba(span, "error", RETURN_Z_STRING(message), frame->exit_time, service_name, host, port, BA_ERROR);    \
        }                   \
}while(0)
#endif

#define SET_SPAN_EXCEPTION_EX(exception_ce, frame, pit)  SET_SPAN_EXCEPTION(exception_ce, pit, frame, pit->pct->service_name, pit->pct->pch.ip, pit->pct->pch.port)

#define SET_DEFAULT_EXCEPTION(frame, pit) do {                                                                     \
        if (EG(exception) != NULL) {                                                                        \
            zend_class_entry *default_exception = (zend_class_entry *)zend_exception_get_default();         \
            SET_SPAN_EXCEPTION_EX(default_exception, frame, pit);                                                  \
        }                                                                                                   \
}while(0)

#define PDO_SET_EXCEPTION(pit)  do {                                            \
        if (EG(exception) != NULL) {                                            \
            zend_class_entry *pdo_exception_ce = php_pdo_get_exception();       \
            SET_SPAN_EXCEPTION_EX(pdo_exception_ce, frame, pit);                \
        }                                                                       \
}while(0)


static char *pcre_common_match(char *pattern, int len, char *subject) 
{
    zval *result = NULL;
    char *ret = NULL;
    MO_ALLOC_INIT_ZVAL(result);
    zval *subpats = NULL;
    MO_ALLOC_INIT_ZVAL(subpats);
    pcre_cache_entry *cache;
#if PHP_VERSION_ID >= 70000
    zend_string *pattern_str = zend_string_init(pattern, len, 0);
    if ((cache = pcre_get_compiled_regex_cache(pattern_str)) != NULL) {
#else
    if ((cache = pcre_get_compiled_regex_cache(pattern, len)) != NULL) {
#endif
        php_pcre_match_impl(cache, subject, strlen(subject), result, subpats, 0, 0, 0, 0 TSRMLS_CC);
        zval *match = NULL;
        if (Z_LVAL_P(result) > 0 && MO_Z_TYPE_P(subpats) == IS_ARRAY) {
#if PHP_VERSION_ID >= 70000
            if ((mo_zend_hash_index_zval_find(Z_ARRVAL_P(subpats), 1, (void **)&match) == SUCCESS)) {
#else
            if ((mo_zend_hash_index_find(Z_ARRVAL_P(subpats), 1, (void **)&match) == SUCCESS)) {
#endif
                if (MO_Z_TYPE_P(match) == IS_STRING) {
                    ret = estrdup(Z_STRVAL_P(match));           
                }
            }
        }
    }

#if PHP_VERSION_ID >= 70000
    zend_string_release(pattern_str);
#endif

    efree(result);
    mo_zval_dtor(subpats);
    efree(subpats);
    return ret;
}

/* only support 'dbname=duobao;host=192.168.56.102:330' format */
static void analyze_data_source(zval *span, char *db_type, char *data_source, mo_frame_t *frame, mo_interceptor_t *pit)
{
    /* sa and db.instance */
    char *dbname = pcre_common_match("(dbname=([^;\\s]+))", sizeof("(dbname=([^;\\s]+))")-1, data_source);
    if (dbname != NULL) {
        pit->psb->span_add_ba_ex(span, "db.instance", dbname, frame->exit_time, pit->pct, BA_NORMAL);
    }

    char *host = pcre_common_match("(host=([^;\\s]+))", sizeof("(host=([^;\\s]+))")-1, data_source);
    char *port = pcre_common_match("(port=([^;\\s]+))", sizeof("(port=([^;\\s]+))")-1, data_source);
    if (host != NULL && port != NULL) {
        int i_port = atoi((const char*)port);
        pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, db_type, host, i_port, BA_SA);
    }

    /* db.data_source */ 
    if (dbname == NULL || host == NULL || port == NULL) {
        pit->psb->span_add_ba_ex(span, "php.db.data_source", data_source, frame->exit_time, pit->pct, BA_SA_DSN);
    }
    
    if (dbname != NULL) efree(dbname);
    if (host != NULL) efree(host);
    if (port != NULL) efree(port);
}

/****************************pdo**********************/
static void pdo_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    zval *object = frame->object;    
    GET_PDO_DBH
    GET_FUNC_ARG(fir_arg,0);
    zval *span;
    char span_name[64] = {0};
    char db_type[64] = {0};

    /* build span */
    snprintf(span_name, sizeof(span_name), "PDO::%s", smart_string_str(frame->function));
    pit->psb->start_span_ex(&span, span_name, pit->pct, frame, AN_CLIENT);
    if (smart_strncmp(frame->function, "commit", sizeof("commit") -1) != 0 && frame->arg_count >= 1 && MO_Z_TYPE_P(fir_arg) == IS_STRING) {
        pit->psb->span_add_ba_ex(span,  "db.statement", RETURN_Z_STRING(fir_arg), frame->exit_time, pit->pct, BA_NORMAL);
    }

    /* db.type */
    if (dbh != NULL && dbh->driver != NULL && dbh->driver->driver_name != NULL) {
        memcpy(db_type, (char *)dbh->driver->driver_name, dbh->driver->driver_name_len);
        pit->psb->span_add_ba_ex(span,  "db.type", db_type, frame->exit_time, pit->pct, BA_NORMAL);
    }

    /* db.data_source */ 
    if (dbh != NULL && dbh->data_source != NULL && db_type[0] != '\0') {
        analyze_data_source(span, db_type, (char *)dbh->data_source, frame, pit);
    }

    /* error */
    zval *ret = frame->ori_ret;
    if (ret != NULL && MO_Z_TYPE_P(ret) == IS_FALSE) {
        zval error_ret;
        zval error_function;
        MO_ZVAL_STRING(&error_function, "errorInfo", 1);
        if (mo_call_user_function(NULL, &object, &error_function, &error_ret, 0, NULL) == SUCCESS) {
            zval *error_msg;
            if ((Z_TYPE(error_ret) == IS_ARRAY) &&  (mo_zend_hash_index_zval_find(Z_ARRVAL(error_ret), 2, (void **)&error_msg) == SUCCESS)) {
                pit->psb->span_add_ba_ex(span,  "error", Z_STRVAL_P(error_msg), frame->exit_time, pit->pct, BA_NORMAL);
            } else {
                pit->psb->span_add_ba_ex(span,  "error", "unknown", frame->exit_time, pit->pct, BA_NORMAL);
            }
        }
        mo_zval_dtor(&error_function);
        mo_zval_dtor(&error_ret);
    }                                                                                                                                       

    PDO_SET_EXCEPTION(pit);
    mo_chain_add_span(pit->pct->pcl, span);
}

static void pdo_statement_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    zval *object = frame->object;    
    zval *span;
    char db_type[64]={0};
    pit->psb->start_span_ex(&span, "PDOStatement::execute", pit->pct, frame, AN_CLIENT);

#if PHP_VERSION_ID < 70000
    pdo_stmt_t *stmt = (pdo_stmt_t *)zend_object_store_get_object(object); 
#else
    pdo_stmt_t *stmt = (pdo_stmt_t *)Z_PDO_STMT_P(object); 
#endif
    
    if (stmt != NULL) {
        
        /* statement */
        pit->psb->span_add_ba_ex(span,  "db.statement", stmt->query_string, frame->exit_time, pit->pct, BA_NORMAL);

        /* db.type */
        if (stmt->dbh != NULL && stmt->dbh->driver != NULL) {
            memcpy(db_type, (char *)stmt->dbh->driver->driver_name, stmt->dbh->driver->driver_name_len);
            pit->psb->span_add_ba_ex(span,  "db.type", db_type, frame->exit_time, pit->pct, BA_NORMAL);
        }

        /* db.data_source */ 
        if (db_type != NULL && stmt->dbh != NULL && stmt->dbh->driver != NULL) {
            analyze_data_source(span, db_type, (char *)stmt->dbh->data_source, frame, pit);
        }
    }

    /* todo retrive data from stmt->bound_params and stmt->bound_columns */
   
    /* get excuption */
    /* error */
    zval *ret = frame->ori_ret;
    if (ret != NULL && MO_Z_TYPE_P(ret) == IS_FALSE) {
        zval error_ret;
        zval error_function;
        MO_ZVAL_STRING(&error_function, "errorInfo", 1);
        if (mo_call_user_function(NULL, &object, &error_function, &error_ret, 0, NULL) == SUCCESS) {
            zval *error_msg;
            if ((Z_TYPE(error_ret) == IS_ARRAY) &&  (mo_zend_hash_index_zval_find(Z_ARRVAL(error_ret), 2, (void **)&error_msg) == SUCCESS)) {
                pit->psb->span_add_ba_ex(span,  "error", Z_STRVAL_P(error_msg), frame->exit_time, pit->pct, BA_ERROR);
            } else {
                pit->psb->span_add_ba_ex(span,  "error", "unkown", frame->exit_time, pit->pct, BA_ERROR);
            }
        }
        mo_zval_dtor(&error_function);
        mo_zval_dtor(&error_ret);
    }
    mo_chain_add_span(pit->pct->pcl, span);
}

/*****************redis****************************/
#define INIT_BANNOTATION_PARAM(MODULE)                                                                          \
    int size = sizeof(#MODULE) + smart_string_len(frame->function) + 2;                                                   \
    char *key = (char *)emalloc(size);                                                                          \
    memset(key, 0x00, size);                                                                                    \
    strncpy(key, #MODULE, sizeof(#MODULE) - 1);                                                                 \
    key = strcat(key, "::");                                                                                    \
    key = strcat(key, smart_string_str(frame->function));                                                                         \
    char *value = convert_args_to_string(frame);                                                                \
    pit->psb->span_add_ba_ex(span,  key, value, frame->exit_time, pit->pct, BA_NORMAL);      \
    efree(value);                                                                                               \

static void redis_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    zval *object = frame->object;    
    zval *span;

    pit->psb->start_span_ex(&span, smart_string_str(frame->function), pit->pct, frame, AN_CLIENT);

    /* sa */ 
    zval host;
    zval port;
    zval function;
    MO_ZVAL_STRING(&function, "getHost", 1);
    if (mo_call_user_function(NULL, &object, &function, &host, 0, NULL) == SUCCESS) { 
        if (Z_TYPE(host) != IS_STRING) {
            MO_ZVAL_STRING(&host, "unkown", 1);
        }
    } else {
        MO_ZVAL_STRING(&host, "unkown", 1);
    }
    
    mo_zval_dtor(&function);

    MO_ZVAL_STRING(&function, "getPort", 1);
    if (mo_call_user_function(NULL, &object, &function, &port, 0, NULL) == SUCCESS) { 
        if (Z_TYPE(port) != IS_LONG) {
            ZVAL_LONG(&port, 0);
        }
    } else {
        ZVAL_LONG(&port, 0);
    }
    mo_zval_dtor(&function);
    pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "redis", Z_STRVAL(host),  Z_LVAL(port), BA_SA);

    /* db.statement */
    char *value_param = convert_args_to_string(frame);
    pit->psb->span_add_ba_ex(span,  "db.statement", value_param, frame->exit_time, pit->pct, BA_NORMAL);
    efree(value_param);
    
    /* db.type */
    pit->psb->span_add_ba_ex(span,  "db.type", "redis", frame->exit_time, pit->pct, BA_NORMAL);

    /* error */
    zval *ret = frame->ori_ret;
    if (ret != NULL && MO_Z_TYPE_P(ret) == IS_FALSE) {
        zval error;
        MO_ZVAL_STRING(&function, "getLastError", 1);
        if (mo_call_user_function(NULL, &object, &function, &error, 0, NULL) == SUCCESS) {
            if (Z_TYPE(error) == IS_STRING) {
                pit->psb->span_add_ba(span,  "error", Z_STRVAL(error), frame->exit_time, "Redis", Z_STRVAL(host), Z_LVAL(port), BA_SA);
            }
            mo_zval_dtor(&error);
        }
        mo_zval_dtor(&function);
    }

    /* error */
    if (EG(exception) != NULL) { 
        zend_class_entry *redis_exception_ce;
        if (mo_zend_hash_find(CG(class_table), "redisexception", sizeof("redisexception"), (void **)&redis_exception_ce) == SUCCESS) {
            SET_SPAN_EXCEPTION_EX(redis_exception_ce, frame, pit);
        }
    }
   
    mo_zval_dtor(&host);
    mo_chain_add_span(pit->pct->pcl, span);
}

/*******************memcached***************************/
static void memcached_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    zval *span;
    zval *object = frame->object;    

    pit->psb->start_span_ex(&span, smart_string_str(frame->function), pit->pct, frame, AN_CLIENT);

    /* add sa */
    if (strncmp(smart_string_str(frame->function), "addServer", sizeof("addServer") - 1) == 0) {
        if (frame->arg_count >= 2) {
            GET_FUNC_ARG(host,0);
            GET_FUNC_ARG(port,1);
            int iport = 0;
            if (MO_Z_TYPE_P(port) == IS_STRING) {
                iport = atoi((const char*)Z_STRVAL_P(port));
            } else if (MO_Z_TYPE_P(port) == IS_LONG) {
                iport = Z_LVAL_P(port);
            }

            if (MO_Z_TYPE_P(host) == IS_STRING) {
                pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "memcache", Z_STRVAL_P(host), iport, BA_SA);
            }
        }
    }

    /* add full sa */ 
    if (strncmp(smart_string_str(frame->function), "addServers", sizeof("addServers") - 1) == 0) {
        GET_FUNC_ARG(servers, 0);
        if (MO_Z_TYPE_P(servers) == IS_ARRAY) {

            int i, entry_size;
#if PHP_VERSION_ID < 70000
	        zval **entry;
            zval **z_host, **z_port;
	        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(servers)), i = 0;
	        	 zend_hash_get_current_data(Z_ARRVAL_P(servers), (void **)&entry) == SUCCESS;
	        	 zend_hash_move_forward(Z_ARRVAL_P(servers)), i++) {

                if (MO_Z_TYPE_PP(entry) != IS_ARRAY) {
                    continue;
                }
                entry_size = zend_hash_num_elements(Z_ARRVAL_PP(entry));
                if (entry_size > 1) {
			        zend_hash_internal_pointer_reset(Z_ARRVAL_PP(entry));
			        /* Check that we have a host */
			        if (zend_hash_get_current_data(Z_ARRVAL_PP(entry), (void **)&z_host) == FAILURE) {
                        continue;

                    }
                    /* Check that we have a port */
			        if (zend_hash_move_forward(Z_ARRVAL_PP(entry)) == FAILURE ||
				        zend_hash_get_current_data(Z_ARRVAL_PP(entry), (void **)&z_port) == FAILURE) {
                        continue;

                    }
                    convert_to_string_ex(z_host);
                    convert_to_long_ex(z_port);
                    pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "memcache", Z_STRVAL_PP(z_host), Z_LVAL_PP(z_port), BA_SA);
                }
            }
#else
	    HashPosition	pos;
	    zval *entry;
	    zval *z_host, *z_port;
		zend_string *host;
		zend_long port;
	    ZEND_HASH_FOREACH_VAL (Z_ARRVAL_P(servers), entry) {
		    if (MO_Z_TYPE_P(entry) != IS_ARRAY) {
                continue;
            }
		    entry_size = zend_hash_num_elements(Z_ARRVAL_P(entry));
            if (entry_size > 1) {
			    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(entry), &pos);
            }

			if ((z_host = zend_hash_get_current_data_ex(Z_ARRVAL_P(entry), &pos)) == NULL) {
                continue;
            }

			if (zend_hash_move_forward_ex(Z_ARRVAL_P(entry), &pos) == FAILURE ||
			    (z_port = zend_hash_get_current_data_ex(Z_ARRVAL_P(entry), &pos)) == NULL) {
                continue;
            }
			host = zval_get_string(z_host);
			port = zval_get_long(z_port);
            pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "memcache", ZSTR_VAL(host), port, BA_SA);
            zend_string_release(host);
	    } ZEND_HASH_FOREACH_END();
#endif
        }
    }

    /* db.statement */
    char *value = convert_args_to_string(frame);
    pit->psb->span_add_ba_ex(span,  "db.statement", value, frame->exit_time, pit->pct, BA_NORMAL);
    efree(value);

    /* db.type */
    pit->psb->span_add_ba_ex(span,  "db.type", "memcache", frame->exit_time, pit->pct, BA_NORMAL);

    /* error */
    zval *ret = frame->ori_ret;
    if (ret != NULL && MO_Z_TYPE_P(ret) == IS_FALSE) {
        zval result_ret;
        zval result_function;
        MO_ZVAL_STRING(&result_function, "getResultMessage", 1);
        if (mo_call_user_function(NULL, &object, &result_function, &result_ret, 0, NULL) == SUCCESS) {
            zval *error_msg;
            if (MO_Z_TYPE_P(&result_ret) == IS_STRING) {
                pit->psb->span_add_ba_ex(span, "error", Z_STRVAL(result_ret), frame->exit_time, pit->pct, BA_ERROR);
            } else {
                pit->psb->span_add_ba_ex(span, "error",  "unknown", frame->exit_time, pit->pct, BA_ERROR);
            }
        }
        mo_zval_dtor(&result_function);
        mo_zval_dtor(&result_ret);
    }

    mo_chain_add_span(pit->pct->pcl, span);
}

/*****************************mysqli******************************/
static void mysqli_connect_common_record(mo_interceptor_t *pit, mo_frame_t *frame, int is_procedural)
{
    if (frame->arg_count < 1) {
        return;
    }

    zval *span = build_com_record(pit, frame, 0);
    
    /* build sa */
    GET_FUNC_ARG(host,0);
    if (frame->arg_count >= 1 && MO_Z_TYPE_P(host) == IS_STRING) {
        if (frame->arg_count < 5) {
            pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", Z_STRVAL_P(host), 3306, BA_SA);
        } else {
            GET_FUNC_ARG(port,4);
            if (MO_Z_TYPE_P(port) == IS_LONG) {
                pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", Z_STRVAL_P(host), Z_LVAL_P(port), BA_SA);
            } else if (MO_Z_TYPE_P(port) == IS_STRING) {
                int n = atoi((const char*)Z_STRVAL_P(port));
                pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", Z_STRVAL_P(host), (long)n, BA_SA);
            }

        }
    }

    /* db.instance */
    GET_FUNC_ARG(dbname,3);
    if (frame->arg_count >= 3 && MO_Z_TYPE_P(dbname) == IS_STRING) {
        pit->psb->span_add_ba_ex(span,  "db.instance",Z_STRVAL_P(dbname), frame->exit_time, pit->pct, BA_NORMAL);
    }

    /* error */
    zval func;
    zval ret;
    MO_ZVAL_STRING(&func, "mysqli_connect_errno", 1);
    if (mo_call_user_function(EG(function_table), (zval **)NULL, &func, &ret, 0, NULL) == SUCCESS) {
        if (MO_Z_TYPE_P(&ret) == IS_LONG && Z_LVAL(ret) > 0) {
            zval error_func;
            zval error_ret;
            MO_ZVAL_STRING(&error_func, "mysqli_connect_error", 1);
            if (mo_call_user_function(EG(function_table), (zval **)NULL, &error_func, &error_ret, 0, NULL) == SUCCESS) { 
                    if (MO_Z_TYPE_P(&error_ret) == IS_STRING) {
                        pit->psb->span_add_ba_ex(span,  "error", Z_STRVAL(error_ret), frame->exit_time, pit->pct, BA_ERROR);
                    }
            }
            mo_zval_dtor(&error_ret);
            mo_zval_dtor(&error_func); 
        }
    }
    mo_zval_dtor(&ret);
    mo_zval_dtor(&func); 

    /* add span */
    mo_chain_add_span(pit->pct->pcl, span);
}

static void mysqli_connect_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    mysqli_connect_common_record(pit, frame, 1);
}

static void mysqli_real_connect_record(mo_interceptor_t *pit, mo_frame_t *frame) 
{
    if (frame->arg_count < 2) {
        return;
    }

    zval *span = build_com_record(pit, frame, 0);

    /* build sa */
    if (frame->arg_count >= 2) {
        GET_FUNC_ARG(host, 1);
        if (MO_Z_TYPE_P(host) == IS_STRING) {
            if (frame->arg_count < 6) {
                pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", Z_STRVAL_P(host), 3306, BA_SA);
            } else {
                GET_FUNC_ARG(port,5);
                if (MO_Z_TYPE_P(port) == IS_LONG) {
                    pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", Z_STRVAL_P(host), Z_LVAL_P(port),  BA_SA);
                } else if (MO_Z_TYPE_P(port) == IS_STRING) {
                    int n = atoi((const char*)Z_STRVAL_P(port));
                    pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", Z_STRVAL_P(host), (long)n,  BA_SA);
                }
            }
        }
    }

    /* db.instance */
    if (frame->arg_count >= 5) {
        GET_FUNC_ARG(dbname, 4);
        if (MO_Z_TYPE_P(dbname) == IS_STRING) {
            pit->psb->span_add_ba_ex(span, "db.instance", Z_STRVAL_P(dbname), frame->exit_time, pit->pct, BA_NORMAL);
        }
    }

    /* error */
    zval func;
    zval ret;
    MO_ZVAL_STRING(&func, "mysqli_connect_errno", 1);
    if (mo_call_user_function(EG(function_table), (zval **)NULL, &func, &ret, 0, NULL) == SUCCESS) {
        if (MO_Z_TYPE_P(&ret) == IS_LONG && Z_LVAL(ret) > 0) {
            zval error_func;
            zval error_ret;
            MO_ZVAL_STRING(&error_func, "mysqli_connect_error", 1);
            if (mo_call_user_function(EG(function_table), (zval **)NULL, &error_func, &error_ret, 0, NULL) == SUCCESS) { 
                    if (MO_Z_TYPE_P(&error_ret) == IS_STRING) {
                        pit->psb->span_add_ba_ex(span, "error", Z_STRVAL(error_ret), frame->exit_time, pit->pct, BA_ERROR);
                    }
            }
            mo_zval_dtor(&error_ret);
            mo_zval_dtor(&error_func); 
        }
    }

    mo_zval_dtor(&ret);
    mo_zval_dtor(&func); 

    /* add span */
    mo_chain_add_span(pit->pct->pcl, span);
}

static void mysqli_common_error(mo_frame_t *frame, int is_procedural, zval *span, mo_interceptor_t *pit, char *error_function, char *class_entry_name) 
{
    char *result = NULL;
    if (is_procedural == 1) {
        GET_FUNC_ARG(resource,0);
        if (MO_Z_TYPE_P(resource) != IS_RESOURCE) {
            return;
        }
        zval func;
        zval ret;
        zval *argv[1];
        MO_ZVAL_STRING(&func, error_function, 1);
        argv[0] = resource;
        if (mo_call_user_function(EG(function_table), (zval **)NULL, &func, &ret, 1, argv) == SUCCESS) {
            if (MO_Z_TYPE_P(&ret) == IS_STRING && Z_STRLEN_P(&ret) > 0) {
                result = estrdup(Z_STRVAL(ret));
            }
        } 
        mo_zval_dtor(&ret);
        mo_zval_dtor(&func); 
    } else {
        zend_class_entry *mysqli;
        if (mo_zend_hash_find(CG(class_table), class_entry_name, strlen(class_entry_name) + 1, (void **)&mysqli) == SUCCESS) {

            zval *obj = frame->object;

#if PHP_VERSION_ID < 70000
            zval *message = mo_zend_read_property(mysqli, obj, "error", sizeof("error") - 1, 1);
            if ((IS_STRING == MO_Z_TYPE_P(message)) && (Z_STRLEN_P(message) > 0)) {
                result = estrdup(Z_STRVAL_P(message));
            } 
#else
            zval rv;
            zval *message = zend_read_property(mysqli, obj, "error", sizeof("error") - 1, 1, &rv);
            if ((IS_STRING == MO_Z_TYPE_P(message)) && (Z_STRLEN_P(message) > 0)) {
                result = estrdup(Z_STRVAL_P(message));
            } 
#endif

#if PHP_VERSION_ID < 70000

            if (message != NULL) {
                mo_zval_dtor(message);
                efree(message);
            }
#else
            if (message != NULL) {
                zval_ptr_dtor(message);
            }
#endif
        }
    }

    if (result != NULL) {
        pit->psb->span_add_ba_ex(span, "error", result, frame->exit_time, pit->pct, BA_ERROR);
        efree(result);
    }
}

static void mysqli_error(mo_frame_t *frame, int is_procedural, zval *span, mo_interceptor_t *pit)
{
    mysqli_common_error(frame, is_procedural, span, pit, "mysqli_error", "mysqli");
}

/* for mysqli get sa just for error return by RETURN_NULL;
*/
static void db_query_get_sa(mo_interceptor_t *pit, mo_frame_t *frame, int procedural, zval *span)
{
    /* get sa */
    /* mysql host and port 
     * mysql->mysql->data->host
     * mysql->mysql->data->port
     */
    MY_MYSQL *mysql;
    zval *mysql_link;
    if (procedural == 1) {
        GET_FUNC_ARG_UNDEC(mysql_link,0);
    } else {
       mysql_link = frame->object; 
    }

    if (mysql_link != NULL && MO_Z_TYPE_P(mysql_link) == IS_OBJECT) {
        zval tmp_zval;
        zval *return_value = &tmp_zval;
#if PHP_VERSION_ID < 70000
        MYSQLI_FETCH_RESOURCE_CONN(mysql, &mysql_link, MYSQLI_STATUS_VALID);
#else
        MYSQLI_FETCH_RESOURCE_CONN(mysql, mysql_link, MYSQLI_STATUS_VALID);
#endif
        if (mysql != NULL && mysql->mysql != NULL && mysql->mysql->data != NULL) {
#if PHP_VERSION_ID < 70100
            pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", mysql->mysql->data->host, mysql->mysql->data->port, BA_SA);
            pit->psb->span_add_ba_ex(span, "db.instance", mysql->mysql->data->connect_or_select_db, frame->exit_time, pit->pct, BA_NORMAL);
#else
            pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", mysql->mysql->data->hostname.s, mysql->mysql->data->port, BA_SA);
            pit->psb->span_add_ba_ex(span, "db.instance", mysql->mysql->data->connect_or_select_db.s, frame->exit_time, pit->pct, BA_NORMAL);
#endif
        }
    }
}

static void db_query_record(mo_interceptor_t *pit, mo_frame_t *frame, int procedural, char *component)
{
    if (procedural == 1 && frame->arg_count < 2) {
        return;
    }

    if (procedural == 0 && frame->arg_count < 1) {
        return;
    }

    zval *span = build_com_record(pit, frame, 0);
    zval *sql;
    if (procedural == 1) {
        GET_FUNC_ARG_UNDEC(sql,1);
    } else {
        GET_FUNC_ARG_UNDEC(sql,0);
    }

    if (MO_Z_TYPE_P(sql) == IS_STRING) {
        pit->psb->span_add_ba_ex(span, "db.statement", Z_STRVAL_P(sql), frame->exit_time, pit->pct, BA_NORMAL);
    }

    pit->psb->span_add_ba_ex(span, "db.type", "mysql", frame->exit_time, pit->pct, BA_NORMAL);
    db_query_get_sa(pit, frame, procedural, span);
    mysqli_error(frame, procedural, span, pit);
    mo_chain_add_span(pit->pct->pcl, span);
}

static void mysqli_query_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    db_query_record(pit, frame, 1, "mysqli_query");
}

static void mysqli_common_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    if (smart_strcmp(frame->function, "__construct") == 0) {
        mysqli_connect_common_record(pit, frame, 0);
        return;
    }

    if (smart_strcmp(frame->function, "mysqli") == 0) {
        mysqli_connect_common_record(pit, frame, 0);
        return;
    }

    if (smart_strcmp(frame->function, "real_connect") == 0) {
        mysqli_connect_common_record(pit, frame, 0);
        return;
    }

    if (smart_strcmp(frame->function, "query") == 0) {
        db_query_record(pit, frame, 0, "mysqli::query");
        return;
    }

    if (smart_strcmp(frame->function , "prepare") == 0) {
        db_query_record(pit, frame, 0, "mysqli::prepare");
        return;
    }

    zval *span = build_com_record(pit, frame, 0);
    pit->psb->span_add_ba_ex(span, "db.type", "mysql", frame->exit_time, pit->pct, BA_NORMAL);
    mo_chain_add_span(pit->pct->pcl, span);
}


/*****************mysqli_stmt***********************/
static void mysqli_stmt_error(mo_frame_t *frame, int is_procedural, zval *span, mo_interceptor_t *pit) 
{
    mysqli_common_error(frame, is_procedural, span, pit, "mysqli_stmt_error", "mysqli_stmt");
}

static void mysqli_stmt_get_sa(mo_interceptor_t *pit, mo_frame_t *frame, int procedural, zval *span)
{
    /* get sa */
    /* mysql host and port 
     * stmt->stmt->data->conn->host
     * stmt->stmt->data->conn->host
     */
    MY_STMT *stmt;
    zval *mysql_stmt;
    if (procedural == 1) {
        GET_FUNC_ARG_UNDEC(mysql_stmt,0);
    } else {
       mysql_stmt = frame->object; 
    }

    if (mysql_stmt != NULL && MO_Z_TYPE_P(mysql_stmt) == IS_OBJECT) {
        zval tmp_zval;
        zval *return_value = &tmp_zval;
#if PHP_VERSION_ID < 70000
        MYSQLI_FETCH_RESOURCE_STMT(stmt, &mysql_stmt, MYSQLI_STATUS_INITIALIZED);
#else
        MYSQLI_FETCH_RESOURCE_STMT(stmt, mysql_stmt, MYSQLI_STATUS_INITIALIZED);
#endif
        if (stmt != NULL && stmt->stmt != NULL && stmt->stmt->data != NULL && stmt->stmt->data->conn != NULL) {
#if PHP_VERSION_ID < 70100
            pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", stmt->stmt->data->conn->host, stmt->stmt->data->conn->port, BA_SA);
            pit->psb->span_add_ba_ex(span, "db.instance", stmt->stmt->data->conn->connect_or_select_db, frame->exit_time, pit->pct, BA_NORMAL);
#else
            pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mysql", stmt->stmt->data->conn->hostname.s, stmt->stmt->data->conn->port, BA_SA);
            pit->psb->span_add_ba_ex(span, "db.instance", stmt->stmt->data->conn->connect_or_select_db.s, frame->exit_time, pit->pct, BA_NORMAL);
#endif
        }
    }

}

static void mysqli_stmt_prepare_common_record(mo_interceptor_t *pit, mo_frame_t *frame, int procedural)
{
    if (procedural == 1) {
        if(frame->arg_count < 2) {
            return;
        }
    } else {
        if (frame->arg_count < 1) {
            return;
        }
    }

    zval *span = build_com_record(pit, frame, 0);
    zval *sql;

    if (procedural == 1) {
        GET_FUNC_ARG_UNDEC(sql,1);
    } else {
        GET_FUNC_ARG_UNDEC(sql,0);
    }

    if (MO_Z_TYPE_P(sql) == IS_STRING) {
        pit->psb->span_add_ba_ex(span, "db.statement", Z_STRVAL_P(sql), frame->exit_time, pit->pct, BA_NORMAL);
    }

    pit->psb->span_add_ba_ex(span, "db.type", "mysql", frame->exit_time, pit->pct, BA_NORMAL);
    mysqli_stmt_get_sa(pit, frame, procedural, span);
    mysqli_stmt_error(frame, procedural, span, pit);
    mo_chain_add_span(pit->pct->pcl, span);
}

static void mysqli_stmt_exe_common_record(mo_interceptor_t *pit, mo_frame_t *frame, int procedural)
{
    zval *span = build_com_record(pit, frame, 0);
    pit->psb->span_add_ba_ex(span, "db.type", "mysql", frame->exit_time, pit->pct, BA_NORMAL);
    mysqli_stmt_get_sa(pit, frame, procedural, span);
    mysqli_stmt_error(frame, procedural, span, pit);
    mo_chain_add_span(pit->pct->pcl, span);
}

static void mysqli_stmt_exe_oo_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    if (smart_strcmp(frame->function, "prepare") == 0) {
        mysqli_stmt_prepare_common_record(pit, frame, 0);
    } else {
        mysqli_stmt_exe_common_record(pit, frame, 0);
    }
}

static void mysqli_stmt_prepare_procedural_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    mysqli_stmt_prepare_common_record(pit, frame, 1);
}

static void mysqli_stmt_exe_procedural_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    mysqli_stmt_exe_common_record(pit, frame, 1);
}

/********************predis***********************************/
static void predis_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    zval *span = build_com_record(pit, frame, 1);
    mo_chain_add_span(pit->pct->pcl, span);
}

/******************mongodb***********************/
static void mongodb_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    if (frame->arg_count < 1) {
        return;
    }
    
    zval *span = build_com_record(pit, frame, 0);
    
    /* add db statement */
    char *value = convert_args_to_string(frame);
    pit->psb->span_add_ba_ex(span, "db.statement", value, frame->exit_time, pit->pct, BA_NORMAL);
    efree(value); 
    pit->psb->span_add_ba_ex(span, "db.type", "mongodb", frame->exit_time, pit->pct, BA_NORMAL);
    
    /* add php.db.source */
    /* read url from debug info */
    zval *obj = frame->object;
    int is_temp;
    HashTable *debug_hash = Z_OBJDEBUG_P(obj, is_temp);
    zval *uri = NULL;
    if (mo_zend_hash_zval_find(debug_hash, "uri", sizeof("uri"), (void **)&uri) == SUCCESS) { 
        if (uri != NULL && MO_Z_TYPE_P(uri) == IS_STRING) {
            pit->psb->span_add_ba_ex(span, "php.db.source", Z_STRVAL_P(uri), frame->exit_time, pit->pct, BA_SA_DSN);
        }
    }
    if (is_temp) {
        zend_hash_destroy(debug_hash);
        efree(debug_hash);
    }

    /* check exception */
    SET_DEFAULT_EXCEPTION(frame, pit);
    mo_chain_add_span(pit->pct->pcl, span);
}

static void mongodb_server_record(mo_interceptor_t *pit, mo_frame_t *frame)
{
    if (frame->arg_count < 1) {
        return;
    }
    
    zval *span = build_com_record(pit, frame, 0);
    
    /* add db statement */
    char *value = convert_args_to_string(frame);
    pit->psb->span_add_ba_ex(span, "db.statement", value, frame->exit_time, pit->pct, BA_NORMAL);
    efree(value); 
    pit->psb->span_add_ba_ex(span, "db.type", "mongodb", frame->exit_time, pit->pct, BA_NORMAL);
    
    /* add php.db.source */
    /* read url from debug info */
    zval *obj = frame->object;
    int is_temp;
    HashTable *debug_hash = Z_OBJDEBUG_P(obj, is_temp);
    zval *host = NULL;
    zval *port = NULL;
    if (mo_zend_hash_zval_find(debug_hash, "host", sizeof("host"), (void **)&host) == SUCCESS) { 
        if (host != NULL && MO_Z_TYPE_P(host) == IS_STRING) {
            if (mo_zend_hash_zval_find(debug_hash, "port", sizeof("port"), (void **)&port) == SUCCESS) { 
                if (port != NULL && MO_Z_TYPE_P(port) == IS_LONG) {
                    pit->psb->span_add_ba(span, "sa", "true", frame->exit_time, "mongodb", Z_STRVAL_P(host), Z_LVAL_P(port), BA_SA);
                }
            }
        }
    }

    if (is_temp) {
        zend_hash_destroy(debug_hash);
        efree(debug_hash);
    }

    /* check exception */
    SET_DEFAULT_EXCEPTION(frame, pit);
    mo_chain_add_span(pit->pct->pcl, span);
}


static int extension_loaded(char *extension_name)
{
    char *lcname = zend_str_tolower_dup(extension_name, strlen(extension_name));
    int result = mo_zend_hash_exists(&module_registry, lcname, strlen(extension_name) + 1);
    efree(lcname);
    return result;
}

#define INIT_INTERCEPTOR_ELE(nk, record_f)  do {                                                 \
    mo_interceptor_ele_t *name##_ele = (mo_interceptor_ele_t *) pemalloc(sizeof(mo_interceptor_ele_t), 1);      \
    name##_ele->name = #nk;                                                                                     \
    name##_ele->keyword = #nk;                                                                                  \
    name##_ele->capture = NULL;                                                                            \
    name##_ele->record = record_f;                                                                              \
    name##_ele->pit = pit;                                                                                      \
    ADD_INTERCEPTOR_ELE(pit, name##_ele);                                                                       \
}while(0)                                                                                                       \

#define INIT_INTERCEPTOR_ELE_TAG(nk, record_f)  do {                                                 \
    INIT_INTERCEPTOR_ELE(nk, record_f);              \
    ADD_INTERCEPTOR_TAG(pit, nk);                                   \
}while(0)

void mo_intercept_ctor(mo_interceptor_t *pit, struct mo_chain_st *pct, mo_span_builder *psb)
{
    /* init global vars */
    pit->elements = (HashTable *)pemalloc(sizeof(HashTable), 1);
    zend_hash_init(pit->elements, 128, NULL, hash_destroy_cb, 1);
    pit->tags = (HashTable *)pemalloc(sizeof(HashTable), 1);
    ZEND_INIT_SYMTABLE_EX(pit->tags, 32, 1);
    ZVAL_LONG(&(pit->curl_http_header_const), -1);
    mo_zend_get_constant("CURLOPT_HTTPHEADER", sizeof("CURLOPT_HTTPHEADER") - 1, &pit->curl_http_header_const);
    pit->pct = pct;
    pit->psb = psb;

    
    if (extension_loaded("PDO")) {
        ADD_INTERCEPTOR_TAG(pit, PDO);
        ADD_INTERCEPTOR_TAG(pit, PDOStatement);
        INIT_INTERCEPTOR_ELE(PDOStatement@execute,  &pdo_statement_record);
        INIT_INTERCEPTOR_ELE(PDO@exec,              &pdo_record);
        INIT_INTERCEPTOR_ELE(PDO@query,             &pdo_record);
        INIT_INTERCEPTOR_ELE(PDO@commit,            &pdo_record);
        INIT_INTERCEPTOR_ELE(PDO@prepare,           &pdo_record);
    }

    if (extension_loaded("mysqli")) {
        INIT_INTERCEPTOR_ELE_TAG(mysqli_connect,             &mysqli_connect_record);
        INIT_INTERCEPTOR_ELE_TAG(mysqli_real_connect,        &mysqli_real_connect_record);
        INIT_INTERCEPTOR_ELE_TAG(mysqli_query,               &mysqli_query_record);
        INIT_INTERCEPTOR_ELE_TAG(mysqli_prepare,             &mysqli_query_record);
        INIT_INTERCEPTOR_ELE_TAG(mysqli_stmt_execute,        &mysqli_stmt_exe_procedural_record);
        INIT_INTERCEPTOR_ELE_TAG(mysqli_stmt_prepare,        &mysqli_stmt_prepare_procedural_record);

        ADD_INTERCEPTOR_TAG(pit, mysqli);
        INIT_INTERCEPTOR_ELE(mysqli@__construct,    &mysqli_common_record);
        INIT_INTERCEPTOR_ELE(mysqli@mysqli,         &mysqli_common_record);
        INIT_INTERCEPTOR_ELE(mysqli@real_connect,   &mysqli_common_record);
        INIT_INTERCEPTOR_ELE(mysqli@query,          &mysqli_common_record);
        INIT_INTERCEPTOR_ELE(mysqli@commit,         &mysqli_common_record);
        INIT_INTERCEPTOR_ELE(mysqli@prepare,        &mysqli_common_record);

        ADD_INTERCEPTOR_TAG(pit, mysqli_stmt);
        INIT_INTERCEPTOR_ELE(mysqli_stmt@execute,   &mysqli_stmt_exe_oo_record);
        INIT_INTERCEPTOR_ELE(mysqli_stmt@prepare,   &mysqli_stmt_exe_oo_record);
    }

    /* curl_multi */
    if (extension_loaded("curl")) {
        INIT_INTERCEPTOR_ELE_TAG(curl_multi_add_handle,     &curl_multi_add_handle_record);
        INIT_INTERCEPTOR_ELE_TAG(curl_multi_remove_handle,  &curl_multi_remove_handle_record);
    }

    /* redis */
    if (extension_loaded("redis")) {
        ADD_INTERCEPTOR_TAG(pit, Redis);
#define RIE(k)   INIT_INTERCEPTOR_ELE(k,     &redis_record)
        RIE(Redis@connect);RIE(Redis@open);RIE(Redis@pconnect);RIE(Redis@popen);
        RIE(Redis@auth);RIE(Redis@flushAll);RIE(Redis@flushDb);RIE(Redis@save);
        RIE(Redis@append);RIE(Redis@bitCount);RIE(Redis@bitOp);RIE(Redis@decr);
        RIE(Redis@decrBy);RIE(Redis@get);RIE(Redis@getBit);RIE(Redis@getRange);
        RIE(Redis@getSet);RIE(Redis@incr);RIE(Redis@incrBy);RIE(Redis@incrByFloat);
        RIE(Redis@mGet);RIE(Redis@getMultiple);RIE(Redis@mSet);RIE(Redis@mSetNx);
        RIE(Redis@set);RIE(Redis@setBit);RIE(Redis@setEx);RIE(Redis@pSetEx);
        RIE(Redis@setNx);RIE(Redis@setRange);RIE(Redis@del);RIE(Redis@delete);
        RIE(Redis@dump);RIE(Redis@exists);RIE(Redis@keys);RIE(Redis@getKeys);
        RIE(Redis@scan);RIE(Redis@migrate);RIE(Redis@move);RIE(Redis@persist);RIE(Redis@sort);
    }

    /* add memcache ele */
    if (extension_loaded("memcached")) {
        ADD_INTERCEPTOR_TAG(pit, Memcached);
#define MIE(k)   INIT_INTERCEPTOR_ELE(k,     &memcached_record)
        MIE(Memcached@add);MIE(Memcached@addByKey);MIE(Memcached@addServer);
        MIE(Memcached@addServers);MIE(Memcached@append);MIE(Memcached@appendByKey);
        MIE(Memcached@cas);MIE(Memcached@casByKey);MIE(Memcached@decrement);
        MIE(Memcached@decrementByKey);MIE(Memcached@delete);MIE(Memcached@deleteByKey);
        MIE(Memcached@deleteMulti);MIE(Memcached@deleteMultiByKey);MIE(Memcached@fetch);
        MIE(Memcached@fetchAll);MIE(Memcached@flush);MIE(Memcached@get);
        MIE(Memcached@getAllKeys);MIE(Memcached@getByKey);MIE(Memcached@getDelayed);
        MIE(Memcached@getDelayedByKey);MIE(Memcached@increment);MIE(Memcached@incrementByKey);
        MIE(Memcached@prepend);MIE(Memcached@prependByKey);MIE(Memcached@replace);
        MIE(Memcached@replaceByKey);MIE(Memcached@set);MIE(Memcached@setByKey);
        MIE(Memcached@setMulti);MIE(Memcached@setMultiByKey);MIE(Memcached@touch);
        MIE(Memcached@touchByKey);
    }

   if (extension_loaded("mongodb")) {
        ADD_INTERCEPTOR_TAG(pit, MongoDB\\Driver\\Manager);
        INIT_INTERCEPTOR_ELE(MongoDB\\Driver\\Manager@__construct, &mongodb_record);
        INIT_INTERCEPTOR_ELE(MongoDB\\Driver\\Manager@executeBulkWrite, &mongodb_record);
        INIT_INTERCEPTOR_ELE(MongoDB\\Driver\\Manager@executeCommand, &mongodb_record);
        INIT_INTERCEPTOR_ELE(MongoDB\\Driver\\Manager@executeQuery, &mongodb_record);


        ADD_INTERCEPTOR_TAG(pit, MongoDB\\Driver\\Server);
        INIT_INTERCEPTOR_ELE(MongoDB\\Driver\\Server@executeBulkWrite, &mongodb_server_record);
        INIT_INTERCEPTOR_ELE(MongoDB\\Driver\\Server@executeCommand, &mongodb_server_record);
        INIT_INTERCEPTOR_ELE(MongoDB\\Driver\\Server@executeQuery, &mongodb_server_record);
   }

}

/* intercept rinit */
void mo_intercept_init(mo_interceptor_t *pit)
{
    MO_ALLOC_INIT_ZVAL(pit->curl_header_record);
    array_init(pit->curl_header_record);
    MO_ALLOC_INIT_ZVAL(pit->span_info_cache);
    array_init(pit->span_info_cache);

    /* span count init */
    pit->span_count = 0;
    
    /* curl_header internel call = 0 */
    pit->curl_header_internel_call = HEADER_OUT_CALL;
}

/* intercept rshutdown */
void mo_intercept_uninit(mo_interceptor_t *pit)
{
#if PHP_VERSION_ID < 70000
    zval_ptr_dtor(&pit->curl_header_record);
    MO_FREE_ALLOC_ZVAL(pit->curl_header_record);
    zval_ptr_dtor(&pit->span_info_cache);
    MO_FREE_ALLOC_ZVAL(pit->span_info_cache);
#else 
    zval_ptr_dtor(pit->curl_header_record);
    MO_FREE_ALLOC_ZVAL(pit->curl_header_record);
    zval_ptr_dtor(pit->span_info_cache);
    MO_FREE_ALLOC_ZVAL(pit->span_info_cache);
#endif
}

void mo_intercept_dtor(mo_interceptor_t *pit)
{
    zend_hash_destroy(pit->elements);
    //zend_hash_destroy(pit->tags);
    //FREE_HASHTABLE(pit->elements);
#if PHP_VERSION_ID < 70000
    zval_dtor(&pit->curl_http_header_const);
#else
    zval_ptr_dtor(&pit->curl_http_header_const);
#endif
    pefree(pit->elements, 1);
    pefree(pit->tags, 1);
    pit->elements = NULL;
    pit->tags = NULL;
}
