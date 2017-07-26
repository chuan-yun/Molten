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

#ifndef MOLTEN_SPAN_H
#define MOLTEN_SPAN_H

#include "stdint.h"
#include <netinet/in.h>

#include "php.h"

#include "php7_wrapper.h"
#include "molten_struct.h"
#include "molten_util.h"

/* log format */
#define ZIPKIN      1
#define OPENTRACING 1<<1

/* annotation type */
#define AN_SERVER      0
#define AN_CLIENT      1

/* ba type */
enum ba_type {BA_NORMAL, BA_SA, BA_SA_HOST, BA_SA_IP, BA_SA_DSN, BA_ERROR, BA_PATH};

/* span context */
typedef struct {
    char *trace_id;             /* this used global trace id */
    char *parent_span_id;       /* use father node id */
    char *span_id;              /* use self node id */
    char *sampled;              /* this used global sampled tag */
    uint32_t span_count;        /* current span count */
} span_context;

/* stack */
//#define MO_STACK_BLOCK_SIZE     16;
//#define MO_STACK_ELE(stack, n)  ((void *)((char *) (stack)->elements + (stack)->size * (n)))
//typedef void(*dtor_func)(void *ele);
//typedef struct {
//    int size,top,max;
//    void *elements; 
//    dtor_func dtor;
//}mo_stack;
//
///* stack func */
//void mo_stack_init(mo_stack *stack, dtor_funct dtor, int size)
//{
//    stack->size = size;
//    stack->top = 0;
//    stack->max = 0;
//    stack->elements = NULL;
//    stack->dtor = dtor;
//}
//
//void mo_stack_push(mo_stack *stack, const void *ele)
//{
//    if (stack->top  >= stack->max) {
//        stack->max += MO_STACK_BLOCK_SIZE;
//        stack->elements = safe_erealloc(stack->elements, stack->size, stack->max, 0);
//    }
//    memcpy(MO_STACK_ELE(stack, stack->top), ele, stack->size);
//    stack->top++;
//}
//
//void mo_stack_pop(mo_stack *stack)
//{
//}

/* span id builder */
typedef void (*build_span_id_func)(char **span_id, char *parent_span_id, int span_count);

/* span_builder */
typedef void (*start_span_func)(zval **span, char *service_name, char *trace_id, char *span_id, char *parent_id, long start_time, long finish_time, struct mo_chain_st *pct, uint8_t an_type);
typedef void (*start_span_ex_func)(zval **span, char *service_name, struct mo_chain_st *pct, mo_frame_t *frame, uint8_t an_type);
typedef void (*span_add_ba_func)(zval *span, const char *key, const char *value, long timestamp, char *service_name, char *ipv4, long port, uint8_t ba_type);
typedef void (*span_add_ba_ex_func)(zval *span, const char *key, const char *value, long timestamp, struct mo_chain_st *pct, uint8_t ba_type);

typedef struct {
    uint8_t type;    
    start_span_func         start_span;
    start_span_ex_func      start_span_ex;
    span_add_ba_func        span_add_ba;
    span_add_ba_ex_func     span_add_ba_ex;
    build_span_id_func      build_span_id;
}mo_span_builder;

/* {{{ span builder for two format */
void zn_start_span_builder(zval **span, char *service_name, char *trace_id, char *span_id, char *parent_id, long start_time, long finish_time, struct mo_chain_st *pct, uint8_t an_type);
void zn_start_span_ex_builder(zval **span, char *service_name, struct mo_chain_st *pct, mo_frame_t *frame, uint8_t an_type);
void zn_span_add_ba_builder(zval *span, const char *key, const char *value, long timestamp, char *service_name, char *ipv4, long port, uint8_t ba_type);
void zn_span_add_ba_ex_builder(zval *span, const char *key, const char *value, long timestamp, struct mo_chain_st *pct, uint8_t ba_type);

void ot_start_span_builder(zval **span, char *service_name, char *trace_id, char *span_id, char *parent_id, long start_time, long finish_time, struct mo_chain_st *pct, uint8_t an_type);
void ot_start_span_ex_builder(zval **span, char *service_name, struct mo_chain_st *pct, mo_frame_t *frame, uint8_t an_type);
void ot_span_add_ba_builder(zval *span, const char *key, const char *value, long timestamp, char *service_name, char *ipv4, long port, uint8_t ba_type);
void ot_span_add_ba_ex_builder(zval *span, const char *key, const char *value, long timestamp, struct mo_chain_st *pct, uint8_t ba_type);
/* }}} */

/* {{{ pt span ctor */
static void inline mo_span_ctor(mo_span_builder *psb, char *span_format, char *span_id_format)
{
    if(strncmp(span_format, "zipkin", sizeof("zipkin") - 1) == 0) {
        psb->type = ZIPKIN;
        psb->start_span         = &zn_start_span_builder;
        psb->start_span_ex      = &zn_start_span_ex_builder;
        psb->span_add_ba        = &zn_span_add_ba_builder;
        psb->span_add_ba_ex     = &zn_span_add_ba_ex_builder;
    } else {
        psb->type = OPENTRACING;
        psb->start_span         = &ot_start_span_builder;
        psb->start_span_ex      = &ot_start_span_ex_builder;
        psb->span_add_ba        = &ot_span_add_ba_builder;
        psb->span_add_ba_ex     = &ot_span_add_ba_ex_builder;
    }

    // level or random 
    if(strncmp(span_id_format, "level", sizeof("level") - 1) == 0) {
        psb->build_span_id = &build_span_id_level;
    } else {
        psb->build_span_id = &build_span_id_random;
    }
}
/* }}} */

/* {{{ record fucntion for zipkin format, it is the default set */
void zn_sart_span(zval **span, char *trace_id, char *service_name, char *span_id, char *parent_id, long timestamp, long duration);
void zn_add_span_annotation(zval *span, const char *value, long timestamp, char *service_name, char *ipv4, long port);
void zn_add_span_annotation_ex(zval *span, const char *value, long timestamp, struct mo_chain_st *pct);
void zn_add_span_bannotation(zval *span, const char *key, const char *value, char *service_name, char *ipv4, long port);
void zn_add_span_bannotation_ex(zval *span, const char *key, const char *value, struct mo_chain_st *pct);
/* }}} */

#endif
