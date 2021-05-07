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

#ifndef MOLTEN_PHP7_WRAPPER_H
#define MOLTEN_PHP7_WRAPPER_H

#include "php.h"
#include "ext/json/php_json.h"
#include "ext/standard/php_array.h"


#if PHP_VERSION_ID < 70000

/* smart string */
#include "ext/standard/php_smart_str.h"
#define smart_string                smart_str
#define smart_string_free           smart_str_free
#define smart_string_appends        smart_str_appends
#define smart_string_appendl        smart_str_appendl
#define smart_string_0              smart_str_0

/* php_json_encode */
#define mo_php_json_encode          php_json_encode

#define mo_zend_hash_update         zend_hash_update
#define mo_zend_hash_update_ptr     zend_hash_update
#define mo_zend_hash_index_update   zend_hash_index_update
#define mo_zend_hash_add            zend_hash_add
#define mo_zend_hash_str_add_zval   zend_hash_add
#define MO_EX_OBJ(ex)               ex->object
#define MO_EX_OBJ_ZVAL(ex)          ex->object
#define MO_EX_OBJCE(ex)             Z_OBJCE_P(ex->object)
#define MO_EX_OPARR(ex)             ex->op_array
#define MO_STR(v)                   v
#define P7_STR_LEN(v)               strlen(v)
#define MO_ZVAL_STRING              ZVAL_STRING
#define MO_ZVAL_STRINGL             ZVAL_STRINGL
#define IS_TRUE                     1
#define IS_FALSE                    -1 
#define MO_MAKE_STD_ZVAL           MAKE_STD_ZVAL
#define MO_ALLOC_INIT_ZVAL         ALLOC_INIT_ZVAL
#define MO_INIT_ZVAL               INIT_ZVAL
//#define MO_ARRAY_INIT(p)           MO_ALLOC_INIT_ZVAL(p);array_init(p)

#define MO_COPY_ZVAL_TO_STRING(z, p) do {               \
    ALLOC_INIT_ZVAL(z);                                 \
    ZVAL_ZVAL(z,p,1,0);                                 \
    convert_to_string(z);                               \
}while(0)
#define MO_FREE_COPY_STRING(z)      zval_dtor(z);
#define MO_FREE_ALLOC_ZVAL(p)

#define mo_add_assoc_string         add_assoc_string
#define mo_add_next_index_string    add_next_index_string

#define mo_zend_get_constant    zend_get_constant
#define mo_zval_ptr_dtor        zval_ptr_dtor
#define mo_zval_dtor            zval_dtor
#define mo_call_user_function   call_user_function
#define mo_zend_read_property   zend_read_property
#define mo_zend_is_auto_global  zend_is_auto_global
#define mo_zend_array_count     zend_hash_num_elements

static inline void mo_array_merge(zval *dest, zval *src TSRMLS_DC) 
{
    php_array_merge(Z_ARRVAL_P(dest), Z_ARRVAL_P(src), 1);  
}

static inline void array_init_persist(zval *arg ZEND_FILE_LINE_DC)
{
    Z_ARRVAL_P(arg) = (HashTable *)pemalloc(sizeof(HashTable), 1);
#if PHP_VERSION_ID < 50600
    _zend_hash_init(Z_ARRVAL_P(arg), 0, NULL, ZVAL_PTR_DTOR, 1 ZEND_FILE_LINE_RELAY_CC);
#else
    _zend_hash_init(Z_ARRVAL_P(arg), 0, ZVAL_PTR_DTOR, 1 ZEND_FILE_LINE_RELAY_CC);
#endif
    Z_TYPE_P(arg) = IS_ARRAY;
}

static inline void array_free_persist(zval *arg)
{
    zend_hash_destroy(Z_ARRVAL_P(arg));
    pefree(Z_ARRVAL_P(arg), 1);
}

static inline int MO_Z_TYPE_P(zval *z)
{
    if (Z_TYPE_P(z) == IS_BOOL) {
        if ((uint8_t)Z_LVAL_P(z) == 1) {
            return IS_TRUE;     
        } else {
            return IS_FALSE;
        }
    } else {
        return Z_TYPE_P(z);
    }
}
#define MO_Z_TYPE_PP(z)     MO_Z_TYPE_P(*z)

static inline int mo_zend_hash_find(HashTable *ht, char *k, int len, void **v)
{
    zval **tmp = NULL; 
    if (zend_hash_find(ht, k, len, (void **)&tmp) == SUCCESS) {
        *v = *tmp;
        return SUCCESS;
    } else {
        *v = NULL;
        return FAILURE;
    }
}

#define mo_zend_hash_zval_find mo_zend_hash_find

static inline int mo_zend_hash_index_find(HashTable *ht, ulong h, void **v)
{
    zval **tmp = NULL;
    if (zend_hash_index_find(ht, h, (void **)&tmp) == SUCCESS) {
        *v = *tmp;
        return SUCCESS;
    } else {
        *v = NULL;
        return FAILURE;
    }
}

#define mo_zend_hash_index_zval_find mo_zend_hash_index_find

static inline int mo_zend_hash_get_current_data(HashTable *ht, void **v)
{
    zval **tmp = NULL;
    if (zend_hash_get_current_data(ht, (void **)&tmp) == SUCCESS) {
        *v = *tmp;
        return SUCCESS;
    } else {
        *v = NULL;
        return FAILURE;
    }
}

#define mo_zend_hash_exists     zend_hash_exists
#define mo_zend_hash_index_del  zend_hash_index_del

#else 

/* smart string */
#include "ext/standard/php_smart_string.h"
#include "Zend/zend_smart_str.h"

/* php_json_encode */
static void inline mo_php_json_encode(smart_string *s, zval *z, int options)
{
    smart_str tmp = {0}; 
    php_json_encode(&tmp, z, options);      
    smart_string_appendl(s, ZSTR_VAL(tmp.s), ZSTR_LEN(tmp.s));
    smart_str_free(&tmp);
}

#if PHP_VERSION_ID < 70100
/* object fetching in PHP 7.0
 * object = call ? Z_OBJ(call->This) : NULL; */
#define MO_EX_OBJ(ex)   Z_OBJ(ex->This)
#else
/* object fetching in PHP 7.1
 * object = (call && (Z_TYPE(call->This) == IS_OBJECT)) ? Z_OBJ(call->This) : NULL; */
#define MO_EX_OBJ(ex)   (Z_TYPE(ex->This) == IS_OBJECT ? Z_OBJ(ex->This) : NULL)
#endif
#define MO_EX_OBJ_ZVAL(ex)          &(ex->This)
#define MO_EX_OBJCE(ex)             Z_OBJCE(ex->This)
#define MO_EX_OPARR(ex)             (&(ex->func->op_array))
#define MO_STR(v)                   ZSTR_VAL(v)
#define MO_STR_LEN(v)               ZSTR_LEN(v)
#define MO_ZVAL_STRING(z,s,dup)     ZVAL_STRING(z,s)
#define MO_ZVAL_STRINGL(z,s,l,dup)  ZVAL_STRINGL(z,s,l)
#define Z_RESVAL(z)                 Z_RES_HANDLE(z)
#define Z_RESVAL_P(z)               Z_RES_HANDLE_P(z)

#define MO_MAKE_STD_ZVAL(p)                     zval _stack_zval_##p; p = &(_stack_zval_##p)
#define MO_ALLOC_INIT_ZVAL(p)                   do{p = emalloc(sizeof(zval)); bzero(p, sizeof(zval));}while(0)
#define MO_INIT_ZVAL(z)                         bzero(z, sizeof(zval))
#define mo_add_next_index_string(z,key,dup)     add_next_index_string(z,key)
#define MO_COPY_ZVAL_TO_STRING(z, p) do {               \
    MO_ALLOC_INIT_ZVAL(z);                              \
    ZVAL_DUP(z,p);                                      \
    convert_to_string(z);                               \
}while(0)
#define MO_FREE_COPY_STRING(z)      zval_dtor(z);
//#define MO_ARRAY_INIT(p)                array_init(p)
#define MO_FREE_ALLOC_ZVAL(p)       efree(p)

#define mo_zval_ptr_dtor(p)     zval_ptr_dtor(*p)
#define mo_zval_dtor(p)         zval_ptr_dtor(p)
#define mo_add_assoc_string(array, key, value, dup) add_assoc_string(array, key, value)
#define MO_Z_TYPE_P        Z_TYPE_P
#define MO_Z_TYPE_PP(z)    Z_TYPE_P(*z)

#define MO_PHP_MAX_PARAMS_NUM   20

#define mo_zend_is_auto_global zend_is_auto_global_str
#define mo_zend_array_count    zend_array_count

static inline int mo_call_user_function(HashTable *ht, zval **obj, zval *function_name, zval *retval_ptr, uint32_t param_count, zval **params) 
{
    zval pass_params[MO_PHP_MAX_PARAMS_NUM];
    int i = 0;
    for(;i < param_count; i++){
        pass_params[i] = *params[i];
    }
    zval *pass_obj = obj ? *obj : NULL;
    return call_user_function(ht, pass_obj, function_name, retval_ptr, param_count, pass_params);
}

static inline zval *mo_zend_read_property(zend_class_entry *class_ptr, zval *obj, char *s, int len, int silent)
{
    zval rv;
    return zend_read_property(class_ptr, obj, s, len, silent, &rv);
}

static inline int mo_zend_get_constant(char *key, int len, zval *z)
{
    zend_string *key_str = zend_string_init(key, len, 0);
    zval *c = zend_get_constant(key_str); 
    zend_string_free(key_str);
    if (c != NULL) {
        ZVAL_COPY(z,c);
        return 1;
    } else {
        return 0;
    }
}

static inline void mo_array_merge(zval *dest, zval *src TSRMLS_DC) 
{
    php_array_merge_recursive(Z_ARRVAL_P(dest), Z_ARRVAL_P(src));  
}

static inline void array_init_persist(zval *arg ZEND_FILE_LINE_DC)
{
    ZVAL_NEW_PERSISTENT_ARR(arg);
#if PHP_VERSION_ID < 70300
    _zend_hash_init(Z_ARRVAL_P(arg), 0, ZVAL_PTR_DTOR, 1 ZEND_FILE_LINE_RELAY_CC);
#else
    _zend_hash_init(Z_ARRVAL_P(arg), 0, ZVAL_PTR_DTOR, 1);
#endif
}

static inline void array_free_persist(zval *arg)
{
    zend_hash_destroy(Z_ARRVAL_P(arg));
    pefree(Z_ARRVAL_P(arg), 1);
}

/***********************hash********************/
static inline int mo_zend_hash_find(HashTable *ht, char *k, int len, void **v)
{
    void *value = (void *)zend_hash_str_find_ptr(ht, k, len - 1);
    if (value == NULL) {
        return FAILURE;
    } else {
        *v = value;
        return SUCCESS;
    }
}

static inline int mo_zend_hash_zval_find(HashTable *ht, char *k, int len, void **v)
{
    zval *value = zend_hash_str_find(ht, k, len - 1);
    if (value == NULL) {
        return FAILURE;
    } else {
        *v = value;
        return SUCCESS;
    }
}


static inline int mo_zend_hash_index_find(HashTable *ht, ulong h, void **v)
{
    void **value = (void **)zend_hash_index_find_ptr(ht, h);
    if (value == NULL) {
        return FAILURE;
    } else {
        *v = *value;
        return SUCCESS;
    }
}

static inline int mo_zend_hash_index_zval_find(HashTable *ht, ulong h, void **v)
{
    zval *value = zend_hash_index_find(ht, h);
    if (value == NULL) {
        return FAILURE;
    } else {
        *v = value;
        return SUCCESS;
    }
}

static inline int mo_zend_hash_get_current_data(HashTable *ht, void **v)
{
    zval *value = zend_hash_get_current_data(ht);
    if (value == NULL) {
        return FAILURE;
    } else {
        *v = Z_PTR_P(value);
        return SUCCESS;
    }
}
static inline int mo_zend_hash_get_current_data_zval(HashTable *ht, void **v)
{
    zval *value = zend_hash_get_current_data(ht);
    if (value == NULL) {
        return FAILURE;
    } else {
        *v = value;
        return SUCCESS;
    }
}

static inline int mo_zend_hash_update_ptr(HashTable *ht, char *k, int len, void *val, int nDataSize, void **pDest)
{
    void **v = (void **)val;
    return zend_hash_str_update_ptr(ht, k, len - 1, *v) ? SUCCESS : FAILURE;
}

static inline int mo_zend_hash_update(HashTable *ht, char *k, int len, void *val, int nDataSize, void **pDest)
{
    //return zend_hash_str_update_ptr(ht, k, len, val) ? SUCCESS : FAILURE;
    void **v = (void **)val;
    return zend_hash_str_update_ptr(ht, k, len - 1, *v) ? SUCCESS : FAILURE;
}

static inline int mo_zend_hash_index_update(HashTable *ht, ulong h, void *pData, uint nDataSize, void **pDest)
{
    void **v = (void **)pData;
    return zend_hash_index_update_ptr(ht, h, *v) ? SUCCESS : FAILURE;
}

static inline void mo_zend_hash_str_add_zval(HashTable *ht, char *k, int len, zval *val, int datasize, void **pDest)
{
    zend_hash_str_add(ht, k, len - 1, val);
}

static inline int mo_zend_hash_add(HashTable *ht, char *k, int len, void *val, int datasize, void **pDest)
{
    void **v = (void **)val;
    return zend_hash_str_add_ptr(ht, k, len - 1, v) ? SUCCESS : FAILURE;
}

static inline int mo_zend_hash_exists(HashTable *ht, char *k, int len)
{
    zend_string *k_str = zend_string_init(k, len - 1, 0);    
    int result = zend_hash_exists(ht, k_str);
    zend_string_free(k_str);
    return result;
}          

static inline int mo_zend_hash_index_del(HashTable *ht, ulong h)
{
    zend_ulong uh = (zend_ulong)h;
    return zend_hash_index_del(ht, uh);
}
#endif

/* smart string wrapper */
#define smart_string_len(s)    s.len
#define smart_string_str(s)    s.c
#define smart_strcmp(s,s1)  strcmp(s.c, s1)
#define smart_strncmp(s,s1,size)  strncmp(s.c, s1, size)

#endif
