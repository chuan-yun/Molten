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

#ifndef MOLTEN_INTERCEMO_H
#define MOLTEN_INTERCEMO_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "stdint.h"

#include "php.h"

#ifdef HAS_PDO
#include "ext/pdo/php_pdo_driver.h"
#endif

#ifdef HAS_MYSQLND
#include "ext/mysqli/php_mysqli_structs.h"
#include "ext/mysqlnd/mysqlnd_structs.h"
#endif

#include "ext/standard/url.h"
#include "ext/pcre/php_pcre.h"
#include "Zend/zend_exceptions.h"

#include "php7_wrapper.h"
#include "molten_struct.h"
#include "molten_span.h"
#include "molten_log.h"
#include "molten_util.h"
#include "molten_chain.h"


#define HEADER_INTERNAL_CALL        1
#define HEADER_OUT_CALL             0

#define ADD_INTERCEPTOR_ELE(interceptor, ele) do {   \
    mo_zend_hash_update_ptr(interceptor->elements, ele->keyword, strlen(ele->keyword) + 1, (void *)(&ele), sizeof(mo_interceptor_ele_t *), NULL);  \
}while(0)

#define ADD_INTERCEPTOR_TAG(interceptor, tag) do {                                                                               \
    mo_zend_hash_update_ptr(interceptor->tags, #tag, sizeof(#tag), (void *)(&(#tag)), sizeof(char *), NULL);             \
}while(0)

#define ADD_ELE_TAG(interceptor, ele) do {          \
    ADD_INTERCEPTOR_ELE(interceptor, ele);          \
    ADD_INTERCEPTOR_TAG(interceptor, ele->keyword);              \
}while(0)

/* interceptor */
typedef struct {
    HashTable   *elements;
    HashTable   *tags;
    zval        curl_http_header_const;            /* curl add header const */

    /* curl request info*/
    zval *curl_header_record;               /* record curl handler set header */

    /* cache info for span */
    zval *span_info_cache;                  /* span info cache */

    /* one request the span count */
    int span_count;
    
    /* curl set header internel */
    int curl_header_internel_call;
   
    /* relation struct */
    struct mo_chain_st *pct;
    mo_span_builder    *psb; 
}mo_interceptor_t;


/* for user defined function or class, the args will be dtor at some time, 
 * if we get args after execute, maybe cano not resolve arg,
 * so we set two phases to merge info.
 */
typedef void (*capture_func)(mo_interceptor_t *pit, mo_frame_t *frame);
typedef void (*record_func)(mo_interceptor_t *pit, mo_frame_t *frame);

/* interceptor element */ 
typedef struct {
    char *keyword;
    capture_func capture;
    record_func record;

    mo_interceptor_t *pit;
}mo_interceptor_ele_t;

/* function */
void mo_intercept_ctor(mo_interceptor_t *pit, struct mo_chain_st *pct, mo_span_builder *psb);
void mo_intercept_dtor(mo_interceptor_t *pit);
zend_bool mo_intercept_hit(mo_interceptor_t *pit, mo_interceptor_ele_t **ele, char *class_name, char *function_name);
void add_chain_header(mo_interceptor_t *pit, zval *curl_resource, char *span_id);
void build_curl_bannotation(zval *span, long timestamp, mo_interceptor_t *pit, zval *handle, char *method, zend_bool check_error);
void mo_intercept_uninit(mo_interceptor_t *pit);
void mo_intercept_init(mo_interceptor_t *pit);
#endif

