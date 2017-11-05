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
#include "molten_util.h"

#if RAND_MAX/256 >= 0xFFFFFFFFFFFFFF
  #define LOOP_COUNT 1
#elif RAND_MAX/256 >= 0xFFFFFF
  #define LOOP_COUNT 2
#elif RAND_MAX/256 >= 0x3FFFF
  #define LOOP_COUNT 3
#elif RAND_MAX/256 >= 0x1FF
  #define LOOP_COUNT 4
#else
  #define LOOP_COUNT 5
#endif

#define MAX_SPANS       65534
#define MAX_SPANS_LEN   5


/* rand uini64 */
uint64_t rand_uint64(void) 
{
    uint64_t r = 0;
    int i = 0;
    struct timeval tv;
    int seed = gettimeofday(&tv, NULL) == 0 ? tv.tv_usec * getpid() : getpid();
    srandom(seed);
    for (i = LOOP_COUNT; i > 0; i--) {
      r = r*(RAND_MAX + (uint64_t)1) + random();
    }
    return r;
}

/* check is hit or not */
int check_hit_ratio(long base)
{
    struct timeval tv;
    int seed = gettimeofday(&tv, NULL) == 0 ? tv.tv_usec * getpid() : getpid();
    srandom(seed);
    int num = random();
    
    /* the remainder 0 is choose by self */
    if (num % base == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* bin to hex */
void b2hex(char **output, const unsigned char *input, int input_len)
{
    static const char hexits[17] = "0123456789abcdef";
    int i;
    *output = (char *)emalloc(input_len * 2 + 1); 
    for (i = 0; i < input_len; i++) {
        *(*output + i*2) = hexits[*(input+i) >> 4];
        *(*output + i*2 + 1) = hexits[*(input+i) & 0x0F];
    }
    *(*output + input_len *2) = '\0';
}

/* bin2hex64 */
void bin2hex64(char **output, const uint64_t *input) 
{
    b2hex(output, (const unsigned char *)input, 8); 
}

/* random 64 and change to hex */
void rand64hex(char **output) 
{
    uint64_t num = rand_uint64();
    return bin2hex64(output, &num);
}

/* build span id random */
void build_span_id_random(char **span_id, char *parent_span_id, int span_count)
{
    rand64hex(span_id);
}

/* span id change to 1.1.x.x 
 * if the parent span id 1.1.1, the span id is 1.1.1.1, the next span id is 1.1.1.2
 * */
/* so we will build our span id contact the parent_span_id */
void build_span_id_level(char **span_id, char *parent_span_id, int span_count)
{
    if (parent_span_id != NULL) {
        /* len = . + \0 + max_spans_len + parent_len */
        int len = 2 + MAX_SPANS_LEN + strlen(parent_span_id); 

        *span_id  = emalloc(len);
        memset(*span_id, 0x00, len);
        sprintf(*span_id, "%s.%d", parent_span_id, span_count);
        (*span_id)[len-1] = '\0';
    } else {
        *span_id = estrdup("1");
    }
}

/* {{{ retrive zval  */
smart_string repr_zval(zval *zv, int limit TSRMLS_DC)
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
/* {{{ change string param */
/* only used by internal function */
static zval* parse_arg(int num) {
	zend_execute_data *ex; 
	ex = EG(current_execute_data);

#if PHP_VERSION_ID < 70000
	void **p = ex->function_state.arguments;
	int arg_count = (int)(zend_uintptr_t) *p;
    zval *arg = *((zval **) (p-(arg_count-(num -1))));
	if (arg_count >= num) {
		return arg;
	}
#else
    int arg_count = ZEND_CALL_NUM_ARGS(ex);
    if (arg_count && arg_count >= num) {
        zval *p = ZEND_CALL_ARG(ex, 1);
        if (ex->func->type == ZEND_USER_FUNCTION) {
            uint32_t first_extra_arg = ex->func->op_array.num_args;

            if (first_extra_arg && arg_count > first_extra_arg) {
              	p = ZEND_CALL_VAR_NUM(ex, ex->func->op_array.last_var + ex->func->op_array.T);
        	}
		}
		return p+(num-1);
    } 
#endif	
	return NULL;
}

/* replace string param */
void change_string_param(int num, char *string) {
	zval *arg = parse_arg(num);
	if (arg != NULL) {
#if PHP_VERSION_ID < 70000
		efree(Z_STRVAL_P(arg));
		ZVAL_STRING(arg, string, 1);
#else 
		zend_string_delref(Z_STR_P(arg));	
		zend_string *p = zend_string_init(string, strlen(string), 0);
		Z_STR_P(arg) = p;
#endif
	}
}

/* replace long param */
void change_long_param(int num, long length) {
	zval *arg = parse_arg(num);
	if (arg != NULL) {
		ZVAL_LONG(arg, length);
	}
}
