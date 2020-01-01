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

#include "molten_span.h"

/* use span context */
/* {{{ build span context */
void span_context_dtor(void *element)
{
#ifdef USE_LEVEL_ID
    mo_span_context *context = (mo_span_context *)element;
    efree(context->span_id);
    context->span_count = 0;
#else
    efree(*(char **)element);
#endif
}
/* }}} */

/* {{{ init span context */
void init_span_context(mo_stack *stack)
{
#ifdef USE_LEVEL_ID
    mo_stack_init(stack, sizeof(mo_span_context), &span_context_dtor);
#else
    mo_stack_init(stack, sizeof(char *), &span_context_dtor);
#endif
}
/* }}} */

/* {{{ push span context */
void push_span_context(mo_stack *stack)
{
#ifdef USE_LEVEL_ID
    char *span_id; 
    mo_span_context context;
    if (!mo_stack_empty(stack)) {
        mo_span_context *parent_node = (mo_span_context *)mo_stack_top(stack);
        build_span_id_level(&span_id, parent_node->span_id, parent_node->span_count);
        parent_node->span_count++;
    } else {
        build_span_id_level(&span_id, NULL, 0);
    }
    
    context.span_id = span_id;
    context.span_count = 1;
    mo_stack_push(stack, &context);
#else
    char *span_id = NULL;
    build_span_id_random(&span_id, NULL, 0);
    mo_stack_push(stack, &span_id);
#endif
}
/* }}} */

/* {{{ push span context with id */
void push_span_context_with_id(mo_stack *stack, char *span_id)
{
#ifdef USE_LEVEL_ID
    mo_span_context context;
    if (!mo_stack_empty(stack)) {
        mo_span_context *parent_node = (mo_span_context *)mo_stack_top(stack);
        parent_node->span_count++;
    }
    context.span_id = span_id;
    context.span_count = 1;
    mo_stack_push(stack, &context);
#else
    char *tmp_span_id = estrdup(span_id);
    mo_stack_push(stack, &tmp_span_id);
#endif
}
/* }}} */

/* {{{ pop span context */
void pop_span_context(mo_stack *stack)
{
    mo_stack_pop(stack, NULL);
}
/* }}} */

/* {{{ retrieve span id */
void retrieve_span_id(mo_stack *stack, char **span_id)
{
#ifdef USE_LEVEL_ID
    mo_span_context *context = (mo_span_context *) mo_stack_top(stack);
    if (context == NULL) {
        *span_id = NULL;
    } else {
        *span_id = context->span_id;
    }
#else
    char **sid = (char **)mo_stack_top(stack);
    if (sid == NULL) {
        *span_id = NULL;
    } else {
        *span_id = *sid;
    }
#endif
}
/* }}} */

/* {{{ retrieve parent span id */
void retrieve_parent_span_id(mo_stack *stack, char **parent_span_id)
{
#ifdef USE_LEVEL_ID
   mo_span_context *context = (mo_span_context *) mo_stack_sec_element(stack);
   if (context == NULL) {
        *parent_span_id = NULL;
   } else {
        *parent_span_id = context->span_id;
   }
#else
    char **psid = (char **)mo_stack_sec_element(stack);
    if (psid == NULL) {
        *parent_span_id = NULL;
    } else {
        *parent_span_id = *psid;
    }
#endif
}
/* }}} */

/* {{{ destroy all span context */
void destroy_span_context(mo_stack *stack)
{
   mo_stack_destroy(stack);
}
/* }}} */

void retrieve_span_id_4_frame(mo_frame_t *frame, char **span_id)
{
   retrieve_span_id(frame->span_stack, span_id);
}

void retrieve_parent_span_id_4_frame(mo_frame_t *frame, char **parent_span_id)
{
   retrieve_parent_span_id(frame->span_stack, parent_span_id);
}

/* {{{ build zipkin v2 format main span */
void zn_v2_start_span(zval **span, char *service_name, char *trace_id, char *span_id, char *parent_id, char *kind, long timestamp, long duration) 
{
    MO_ALLOC_INIT_ZVAL(*span);
    array_init(*span);
    mo_add_assoc_string(*span, "traceId", trace_id, 1);
    mo_add_assoc_string(*span, "name", service_name, 1);
    mo_add_assoc_string(*span, "version", SPAN_VERSION, 1);
    mo_add_assoc_string(*span, "kind", kind, 1);
    mo_add_assoc_string(*span, "id", span_id, 1);
    if (parent_id != NULL) {
        mo_add_assoc_string(*span, "parentId", parent_id, 1);
    }
    add_assoc_long(*span, "timestamp", timestamp);
    add_assoc_long(*span, "duration", duration);
    
    /* add tags */
    zval *tags;
    MO_ALLOC_INIT_ZVAL(tags);
    array_init(tags);
    add_assoc_zval(*span, "tags", tags);
	
    /* free */
    MO_FREE_ALLOC_ZVAL(tags);
}
/* }}} */

/* {{{ add span annotation */
void zn_v2_add_endpoint(zval *span, bool is_local, char *service_name, char *ipv4, long port) 
{
    if (span == NULL || service_name == NULL || ipv4 == NULL) {
        return;
    }

    zval *endpoint;
    if (is_local) {
    	if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "localEndpoint", sizeof("localEndpoint"), (void **)&endpoint) == FAILURE) {
            /* add local Endpoint */
            zval *local_endpoint;
            MO_ALLOC_INIT_ZVAL(local_endpoint);
            array_init(local_endpoint);
            add_assoc_zval(span, "localEndpoint", local_endpoint);
	        endpoint=local_endpoint;
    	}
    } else {
	    if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "remoteEndpoint", sizeof("remoteEndpoint"), (void **)&endpoint) == FAILURE) {
            /* add remote endpoint */
            zval *remote_endpoint;
            MO_ALLOC_INIT_ZVAL(remote_endpoint);
            array_init(remote_endpoint);
            add_assoc_zval(span, "remoteEndpoint", remote_endpoint);
            endpoint=remote_endpoint;
    	}
    }

    mo_add_assoc_string(endpoint, "serviceName", service_name, 1); 
    mo_add_assoc_string(endpoint, "ipv4", ipv4, 1);
    if (port != 0) {
        add_assoc_long(endpoint, "port", port);
    }
}
/* }}} */

/* {{{ build zipkin v2 format span */
void zn_v2_start_span_builder(zval **span, char *service_name, char *trace_id, char *span_id, char *parent_id, long start_time, long finish_time, struct mo_chain_st *pct, uint8_t an_type)
{
    if (an_type == AN_SERVER) {
    	zn_v2_start_span(span, service_name, trace_id, span_id, parent_id, "SERVER", start_time, finish_time - start_time);
    } else {
    	zn_v2_start_span(span, service_name, trace_id, span_id, parent_id, "CLIENT", start_time, finish_time - start_time);
    }
    zn_v2_add_endpoint(*span, true, service_name, pct->pch.ip, pct->pch.port);
}
/* }}} */

/* {{{ build zipkin v2 format span */
void zn_v2_start_span_ex_builder(zval **span, char *service_name, struct mo_chain_st *pct, mo_frame_t *frame, uint8_t an_type)
{
    char *span_id;
    char *parent_span_id;

    retrieve_span_id_4_frame(frame, &span_id);
    retrieve_parent_span_id_4_frame(frame, &parent_span_id);

    zn_v2_start_span_builder(span, service_name, pct->pch.trace_id->val, span_id, parent_span_id, frame->entry_time, frame->exit_time, pct, an_type);
}
/* }}} */

/* {{{ zipkin add tag */
void zn_v2_add_tag(zval *span, const char *key, const char *val){
    if (span == NULL || key == NULL || val == NULL ) {
        return;
    }
    zval *tags;
    if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "tags", sizeof("tags"), (void **)&tags) == FAILURE) {
        return;
    }
    mo_add_assoc_string(tags, key, (char *)val, 1);
}
/* }}} */

/* {{{ build zn v2 ba builder */
void zn_v2_span_add_ba_builder(zval *span, const char *key, const char *value, long timestamp, char *service_name, char *ipv4, long port, uint8_t ba_type) {
    switch (ba_type) {
        case BA_NORMAL:
        case BA_PATH: 
            zn_v2_add_tag(span, key, value);
            break;
        case BA_SA:
        case BA_SA_HOST:
        case BA_SA_IP:
            zn_v2_add_tag(span, key, value);
            zn_v2_add_endpoint(span, false, service_name, ipv4, port);
            break;
        default:
            break;
    }

}
/* }}} */

/* {{{ zipkin v2 span add ba builder */
void zn_v2_span_add_ba_ex_builder(zval *span, const char *key, const char *value, long timestamp, struct mo_chain_st *pct, uint8_t ba_type) {
    zn_v2_span_add_ba_builder(span, key, value, timestamp, pct->service_name, pct->pch.ip, pct->pch.port, ba_type);
}
/* }}} */

/* {{{ build zipkin format main span */
void zn_start_span(zval **span, char *trace_id, char *server_name, char *span_id, char *parent_id, long timestamp, long duration) 
{
    MO_ALLOC_INIT_ZVAL(*span);
    array_init(*span);
    mo_add_assoc_string(*span, "traceId", trace_id, 1);
    mo_add_assoc_string(*span, "name", server_name, 1);
    mo_add_assoc_string(*span, "version", SPAN_VERSION, 1);
    mo_add_assoc_string(*span, "id", span_id, 1);
    if (parent_id != NULL) {
        mo_add_assoc_string(*span, "parentId", parent_id, 1);
    }
    add_assoc_long(*span, "timestamp", timestamp);
    add_assoc_long(*span, "duration", duration);
    
    /* add annotions */
    zval *annotations;
    MO_ALLOC_INIT_ZVAL(annotations);
    array_init(annotations);
    add_assoc_zval(*span, "annotations", annotations);

    /* add binaryAnnotationss */
    zval *bannotations;
    MO_ALLOC_INIT_ZVAL(bannotations);
    array_init(bannotations);
    add_assoc_zval(*span, "binaryAnnotations", bannotations);

    MO_FREE_ALLOC_ZVAL(annotations);
    MO_FREE_ALLOC_ZVAL(bannotations);
}
/* }}} */

/* {{{ build zipkin service name */
char *zn_build_service_name(struct mo_chain_st *pct, char *service_name)
{
    char *g_service_name = pct->service_name; 
    int service_len = strlen(service_name) + strlen(g_service_name) + 3;
    char *full_service_name = emalloc(service_len);
    memset(full_service_name, 0x00, service_len);
    snprintf(full_service_name, service_len, "%s-%s", g_service_name, service_name);
    full_service_name[service_len-1] = '\0';
    return full_service_name;
}
/* }}} */

/* {{{ add endpoint */
void zn_add_endpoint(zval *annotation, char *service_name, char *ipv4, long port) 
{
    zval *endpoint; 
    MO_ALLOC_INIT_ZVAL(endpoint);
    array_init(endpoint);
    mo_add_assoc_string(endpoint, "serviceName", service_name, 1); 
    mo_add_assoc_string(endpoint, "ipv4", ipv4, 1);
    if (port != 0) {
        add_assoc_long(endpoint, "port", port);
    }
    add_assoc_zval(annotation, "endpoint", endpoint);
    MO_FREE_ALLOC_ZVAL(endpoint);
}
/* }}} */

/* {{{ add span annotation */
void zn_add_span_annotation(zval *span, const char *value, long timestamp, char *service_name, char *ipv4, long port) 
{
    if (span == NULL || value == NULL || service_name == NULL || ipv4 == NULL) {
        return;
    }

    zval *annotations;
    if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "annotations", sizeof("annotations"), (void **)&annotations) == FAILURE) {
        return;
    }

    zval *annotation;
    MO_ALLOC_INIT_ZVAL(annotation);
    array_init(annotation);
    mo_add_assoc_string(annotation, "value", (char *)value, 1);
    add_assoc_long(annotation, "timestamp", timestamp);
    zn_add_endpoint(annotation, service_name, ipv4, port);
    add_next_index_zval(annotations, annotation);
    MO_FREE_ALLOC_ZVAL(annotation);

}
/* }}} */

/* {{{ add span annotation ex */
void zn_add_span_annotation_ex(zval *span, const char *value, long timestamp, struct mo_chain_st *pct)
{
    zn_add_span_annotation(span, value, timestamp, pct->service_name, pct->pch.ip, pct->pch.port);
}
/* }}} */

/* {{{ add span binnary annotation */
void zn_add_span_bannotation(zval *span, const char *key, const char *value, char *service_name, char *ipv4, long port)
{
    if (span == NULL || key == NULL || value == NULL || service_name == NULL || ipv4 == NULL) {
        return;
    }

    int init = 0;
    zval *bannotations;
    if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "binaryAnnotations", sizeof("binaryAnnotations"), (void **)&bannotations) == FAILURE) {
        /* add binaryAnnotationss */
        MO_ALLOC_INIT_ZVAL(bannotations);
        array_init(bannotations);
        add_assoc_zval(span, "binaryAnnotations", bannotations);
        init = 1;
    }

    zval *bannotation;
    MO_ALLOC_INIT_ZVAL(bannotation);
    array_init(bannotation);
    mo_add_assoc_string(bannotation, "key", (char *)key, 1);
    mo_add_assoc_string(bannotation, "value", (char *)value, 1);
    zn_add_endpoint(bannotation, service_name, ipv4, port);
    add_next_index_zval(bannotations, bannotation);
    MO_FREE_ALLOC_ZVAL(bannotation);

    if (init == 1) {
        MO_FREE_ALLOC_ZVAL(bannotations);
    }
}
/* }}} */

/* {{{ add span binnary annotation ex */
void zn_add_span_bannotation_ex(zval *span, const char *key, const char *value, struct mo_chain_st *pct) 
{
    zn_add_span_bannotation(span, key, value, pct->service_name, pct->pch.ip, pct->pch.port);
}

/* {{{ build opentracing format span                            */
/* --------------------span format----------------------------- */
/* ------------------------------------------------------------ */
/* |operationName(string)|startTime(long)|finishTime(long)  |   */
/* |spanContext(map){traceID, spanID, parentSpanID}         |   */
/* |tags(map)|logs(list)|references(not used)               |   */
/* ------------------------------------------------------------ */
void ot_start_span(zval **span, char *op_name, char *trace_id, char *span_id, char *parent_id, int sampled, long start_time, long finish_time)
{
    MO_ALLOC_INIT_ZVAL(*span);
    array_init(*span);

    mo_add_assoc_string(*span, "operationName", op_name, 1);
    add_assoc_long(*span, "startTime", start_time);
    add_assoc_long(*span, "finishTime", finish_time);

    /* add spanContext */
    zval *spanContext;
    MO_ALLOC_INIT_ZVAL(spanContext);
    array_init(spanContext);
    mo_add_assoc_string(spanContext, "traceID", trace_id, 1);
    mo_add_assoc_string(spanContext, "spanID", span_id, 1);
    if (parent_id != NULL) {
        mo_add_assoc_string(spanContext, "parentSpanID", parent_id, 1);
    }
    add_assoc_zval(*span, "spanContext", spanContext);

    /* add tags */
    zval *tags;
    MO_ALLOC_INIT_ZVAL(tags);
    array_init(tags);
    add_assoc_zval(*span, "tags", tags);

    /* add logs */
    zval *logs;
    MO_ALLOC_INIT_ZVAL(logs);
    array_init(logs);
    add_assoc_zval(*span, "logs", logs);

    /* free map */
    MO_FREE_ALLOC_ZVAL(logs);
    MO_FREE_ALLOC_ZVAL(tags);
    MO_FREE_ALLOC_ZVAL(spanContext);
}
/* }}} */

/* {{{ opentracing add tag */
/* the tag list @see https://github.com/opentracing-contrib/opentracing-specification-zh/blob/master/semantic_conventions.md */
void ot_add_tag(zval *span, const char *key, const char *val) 
{
    if (span == NULL || key == NULL || val == NULL ) {
        return;
    }
    zval *tags;
    if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "tags", sizeof("tags"), (void **)&tags) == FAILURE) {
        return;
    }
    mo_add_assoc_string(tags, key, (char *)val, 1);
}
/* }}} */

void ot_add_tag_long(zval *span, const char *key, long val)
{
    if (span == NULL || key == NULL) {
        return;
    }

    zval *tags;
    if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "tags", sizeof("tags"), (void **)&tags) == FAILURE) {
        return;
    }
    add_assoc_long(tags, key, val);
}

void ot_add_tag_bool(zval *span, const char *key, uint8_t val)
{
    if (span == NULL || key == NULL) {
        return;
    }

    zval *tags;
    if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "tags", sizeof("tags"), (void **)&tags) == FAILURE) {
        return;
    }
    add_assoc_bool(tags, key, val);
}

/* {{{ opentracing add log */
void ot_add_log(zval *span, long timestamp, int8_t field_num, ...)
{
    if (span == NULL) {
        return;
    }

    zval *logs;
    if (mo_zend_hash_zval_find(Z_ARRVAL_P(span), "logs", sizeof("logs"), (void **)&logs) == FAILURE) {
        return;
    }

    /* build fields */
    zval *fields;
    MO_ALLOC_INIT_ZVAL(fields);
    array_init(fields);
         
    va_list arg_ptr;
    int i = 0;
    char *key, *val = NULL;
    va_start(arg_ptr, field_num);

    /* fetch very key and val */
    for (;i < field_num; i++) {
        key = va_arg(arg_ptr, char*);
        val = va_arg(arg_ptr, char*);
        mo_add_assoc_string(fields, key, val, 1);
    }
    
    /* build log */
    zval *log;
    MO_ALLOC_INIT_ZVAL(log);
    array_init(log);
    add_assoc_long(log, "timestamp", timestamp);
    add_assoc_zval(log, "fields", fields);
    
    /* add log */
    add_next_index_zval(logs, log);

    /* free map */
    MO_FREE_ALLOC_ZVAL(log);
    MO_FREE_ALLOC_ZVAL(fields);
}

/* span function wrapper for zipkin */
void zn_start_span_builder(zval **span, char *service_name, char *trace_id, char *span_id, char *parent_id, long start_time, long finish_time, struct mo_chain_st *pct, uint8_t an_type)
{
    zn_start_span(span, trace_id, service_name, span_id, parent_id, start_time, finish_time - start_time);
    if (an_type == AN_SERVER) {
        zn_add_span_annotation_ex(*span, "sr", start_time, pct);
        zn_add_span_annotation_ex(*span, "ss", finish_time, pct);
    } else {
        zn_add_span_annotation_ex(*span, "cs", start_time, pct);
        zn_add_span_annotation_ex(*span, "cr", finish_time, pct);
    }
}

void zn_start_span_ex_builder(zval **span, char *service_name, struct mo_chain_st *pct, mo_frame_t *frame, uint8_t an_type)
{
    char *span_id;
    char *parent_span_id;

    retrieve_span_id_4_frame(frame, &span_id);
    retrieve_parent_span_id_4_frame(frame, &parent_span_id);

    zn_start_span_builder(span, service_name, pct->pch.trace_id->val, span_id, parent_span_id, frame->entry_time, frame->exit_time, pct, an_type);
}

void zn_span_add_ba_builder(zval *span, const char *key, const char *value, long timestamp, char *service_name, char *ipv4, long port, uint8_t ba_type)
{
    zn_add_span_bannotation(span, key, value, service_name, ipv4, port);
}

void zn_span_add_ba_ex_builder(zval *span, const char *key, const char *value, long timestamp, struct mo_chain_st *pct, uint8_t ba_type) 
{
    zn_span_add_ba_builder(span, key, value, timestamp, pct->service_name, pct->pch.ip, pct->pch.port, ba_type);
}

/** span function wrapper for opentracing */
void ot_start_span_builder(zval **span, char *service_name, char *trace_id, char *span_id, char *parent_id, long start_time, long finish_time, struct mo_chain_st *pct, uint8_t an_type)
{
    ot_start_span(span, service_name, trace_id, span_id, parent_id, 1, start_time, finish_time);
    if (an_type == AN_SERVER) {
        ot_add_tag(*span, "span.kind", "server");
    } else {
        ot_add_tag(*span, "span.kind", "client");
    }
}

void ot_start_span_ex_builder(zval **span, char *service_name, struct mo_chain_st *pct, mo_frame_t *frame, uint8_t an_type)
{
    char *span_id;
    char *parent_span_id;

    retrieve_span_id_4_frame(frame, &span_id);
    retrieve_parent_span_id_4_frame(frame, &parent_span_id);

    ot_start_span_builder(span, service_name, pct->pch.trace_id->val, span_id, parent_span_id, frame->entry_time, frame->exit_time, pct, an_type);
}

void ot_span_add_ba_builder(zval *span, const char *key, const char *value, long timestamp, char *service_name, char *ipv4, long port, uint8_t ba_type)
{
    switch (ba_type) {
        case BA_NORMAL:
            ot_add_tag(span, key, value);
            break;
        case BA_SA:
            ot_add_tag(span, "peer.ipv4", ipv4);
            ot_add_tag_long(span, "peer.port", port);
            ot_add_tag(span, "peer.service", service_name);
            break;
        case BA_SA_HOST:
            ot_add_tag(span, "peer.hostname", ipv4);
            ot_add_tag_long(span, "peer.port", port);
            ot_add_tag(span, "peer.service", service_name);
            break;
        case BA_SA_IP:
            ot_add_tag(span, "peer.ipv4", ipv4);
            ot_add_tag_long(span, "peer.port", port);
            ot_add_tag(span, "peer.service", service_name);
            break;
        case BA_SA_DSN:
            ot_add_tag(span, "peer.address", value);
            break;
        case BA_PATH:
            /* not use for opentracing */
            break;
        case BA_ERROR:
            ot_add_tag_bool(span, "error", 1);
            ot_add_log(span, timestamp, 3, "event", "error", "error.kind", "Exception", "message", value);
        default:
            break;
    }

}
void ot_span_add_ba_ex_builder(zval *span, const char *key, const char *value, long timestamp, struct mo_chain_st *pct, uint8_t ba_type)
{
    ot_span_add_ba_builder(span, key, value, timestamp, pct->service_name, pct->pch.ip, pct->pch.port, ba_type);
}
