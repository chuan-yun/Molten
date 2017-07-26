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

#ifndef MOLTEN_REPORT_H
#define MOLTEN_REPORT_H

#include <stdint.h>

#include "php.h"
#include "Zend/zend_llist.h"
#include "php7_wrapper.h"

#include "molten_util.h"
#include "molten_ctrl.h"
#include "molten_log.h"

#define REPORT_FLAG     "[report]"

typedef struct {
    char *uri;                  /* error uri */
    char *error;                /* error message (truncate) */
    long timestamp;             /* error occour timestamp */
} mo_rep_error_t;

/* molten report data */
typedef struct{
    long            last_rep_time;             /* last report time sec */
    long            rep_interval;              /* rep interval */
    long            report_limit;              /* report count limit */
    
    zend_llist      error_list;                 /* zend error */
    /* report data */
    //zval            request_all;               /* request all (long) */
    //zval            request_capture;           /* capture request count (long) */
    //zval            url_error_map;             /* url and error map (array) */
    //zval            url_count_map;             /* url and count map (array) */
} mo_report_t;            

void mo_rep_ctor(mo_report_t *pre, long rep_interval, long report_limit);
void mo_rep_record_data(mo_report_t *pre, mo_repi_t *pri, mo_chain_log_t *pcl, smart_string *uri, int is_sampled, long usec);
void mo_rep_record_error(mo_report_t *pre, smart_string *uri, char *error_str, long usec);
void mo_rep_dtor(mo_report_t *pre);
#endif
