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

#include "molten_report.h"

/* {{{ pt error struct dtor */
void rep_error_dtor(void *data)
{
    mo_rep_error_t *e = (mo_rep_error_t *)data;
    pefree(e->uri, 1);
    pefree(e->error, 1);
    //pefree(e, 1);
}
/* }}} */

/* {{{ mo_rep_ctor */
void mo_rep_ctor(mo_report_t *pre, long rep_interval, long report_limit)
{
    pre->last_rep_time = mo_time_sec();
    pre->rep_interval = rep_interval;
    pre->report_limit = report_limit;

    zend_llist_init(&pre->error_list, sizeof(mo_rep_error_t), rep_error_dtor, 1);

    /* init repi */
    /*
    ZVAL_LONG(&pre->request_all, 0);
    ZVAL_LONG(&pre->request_capture, 0);
    array_init_persist(&pre->url_count_map ZEND_FILE_LINE_CC);
    array_init_persist(&pre->url_error_map ZEND_FILE_LINE_CC);
    */
}
/* }}} */

/* {{{ mo_ctrl_dtor */
void mo_rep_dtor(mo_report_t *pre)
{
    /*
    array_free_persist(&pre->url_count_map);
    array_free_persist(&pre->url_error_map);
    */
   zend_llist_destroy(&pre->error_list); 
}
/* }}} */


/* {{{ check req interval already */
static int inline check_interval(mo_report_t *pre, long usec)
{
    long sec = usec/1000000;
    if ((sec - pre->last_rep_time) > pre->rep_interval) {
        pre->last_rep_time = sec;
        return 2;
    } else {
        int count = zend_llist_count(&pre->error_list);
        if (count >= pre->report_limit) {
            pre->last_rep_time = sec;
            return 1; 
        }
        return 0;
    }
}
/* }}} */

/* {{{ report record error */
void mo_rep_record_error(mo_report_t *pre, smart_string *uri, char *error_str, long usec)
{
    if (uri != NULL && smart_string_str((*uri)) != NULL) {
        //mo_rep_error_t *e = pemalloc(sizeof(mo_rep_error_t), 1); 
        mo_rep_error_t e = {0};
        e.uri = pestrdup(smart_string_str((*uri)), 1);
        e.error = pestrdup(error_str, 1);
        e.timestamp = usec;
        zend_llist_add_element(&pre->error_list, &e);
    }
}
/* }}} */

/* {{{ list apply func */
static void llist_apply_func(void *data, void *arg)
{
    mo_rep_error_t *e = (mo_rep_error_t *)data;
    zval *arr       = (zval *)arg;
    zval *pack;
    MO_ALLOC_INIT_ZVAL(pack);
    array_init(pack);
    mo_add_assoc_string(pack, "uri", e->uri, 1);
    mo_add_assoc_string(pack, "error", e->error, 1);
    add_assoc_long(pack, "timestamp", e->timestamp);

    add_next_index_zval(arr, pack); 
    MO_FREE_ALLOC_ZVAL(pack);
}
/* }}} */

/* {{{ format error list to array */
void format_llist_to_array(smart_string *s, zend_llist *error_list)
{
    /* build array */
    zval *pack;
    zval *out;
    MO_ALLOC_INIT_ZVAL(pack);
    MO_ALLOC_INIT_ZVAL(out);
    array_init(pack);
    array_init(out);
    zend_llist_apply_with_argument(error_list, llist_apply_func, pack);
    
    add_assoc_zval(out, "error", pack);

    /* swap to string */
    mo_php_json_encode(s, out, 0);
    smart_string_appendl(s, "\n", sizeof("\n") - 1);

#if PHP_VERSION_ID < 70000
    zval_ptr_dtor(&pack);
    zval_ptr_dtor(&out);
#else
    zval_ptr_dtor(pack);
    zval_ptr_dtor(out);
#endif

    MO_FREE_ALLOC_ZVAL(pack);
    MO_FREE_ALLOC_ZVAL(out);
}
/* }}} */

/* {{{ pt req record data */
void mo_rep_record_data(mo_report_t *pre, mo_repi_t *pri, mo_chain_log_t *pcl, smart_string *uri, int is_sampled, long usec) 
{
    /*
    ZVAL_LONG_PLUS(&pre->request_all);
    if (1 == is_sampled) {
        ZVAL_LONG_PLUS(&pre->request_capture);
    }

    if (uri != NULL) {
        zval *tmp;
        int count = mo_zend_array_count(Z_ARRVAL(pre->url_count_map));
        if (count < pre->report_limit) {
            if (mo_zend_hash_zval_find(Z_ARRVAL(pre->url_count_map), smart_string_str((*uri)), (smart_string_len((*uri)) + 1), (void **)&tmp) == SUCCESS) {
                add_assoc_long(&pre->url_count_map, smart_string_str((*uri)), Z_LVAL_P(tmp) + 1);
            } else {
                add_assoc_long(&pre->url_count_map, smart_string_str((*uri)), 1);
            }
        }
    }
    */

//    if (check_interval(pre, usec)) {

        /*
        smart_string_appendl(&send, REPORT_FLAG, sizeof(REPORT_FLAG) - 1);
        zval *pack;
        MO_ALLOC_INIT_ZVAL(pack);
        array_init(pack);
        add_assoc_zval(pack,        "requestAll",           &pre->request_all);
        add_assoc_zval(pack,        "requestCapture",       &pre->request_capture);
        add_assoc_zval(pack,        "urlCount",             &pre->url_count_map);
        add_assoc_zval(pack,        "errorList",            &pre->url_error_map);
    
        php_json_encode(&send, pack, 0);
        smart_string_appendl(&send, "\n", sizeof("\n") - 1);

#if PHP_VERSION_ID < 70000
        zval_ptr_dtor(&pack);
#else
        zval_ptr_dtor(pack);
#endif
        MO_FREE_ALLOC_ZVAL(pack);
        */

        if (zend_llist_count(&pre->error_list) > 0) {
            smart_string send = {0};
            format_llist_to_array(&send, &pre->error_list);

            /* write log */
            mo_log_write(pcl, smart_string_str(send), smart_string_len(send));
            smart_string_free(&send);

            /* clear and init */
            mo_rep_dtor(pre);

            /*
            ZVAL_LONG(&pre->request_all, 0);
            ZVAL_LONG(&pre->request_capture, 0);
            array_init_persist(&pre->url_count_map ZEND_FILE_LINE_CC);
            array_init_persist(&pre->url_error_map ZEND_FILE_LINE_CC);
            */
            zend_llist_init(&pre->error_list, sizeof(mo_rep_error_t), rep_error_dtor, 1);   
        }
//    }
}
/* ]}} */
