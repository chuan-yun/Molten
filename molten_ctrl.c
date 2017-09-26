
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
#include "molten_ctrl.h"

/* {{{ mo_ctrl_ctor */
int mo_ctrl_ctor(mo_ctrl_t *prt, mo_shm_t *msm, char *notify_uri, char *ip, long sampling_type, long sampling_rate, long sampling_request)
{
    long min = mo_time_m();
    mo_ctrm_t mcm = {0, 1, sampling_type, sampling_rate, sampling_request};
    mo_repi_t mri = {0, 0};
    mo_sr_t   msr = {min, sampling_request};

    /* init repi */
    prt->mcm = (mo_ctrm_t *)mo_create_slot(msm, MO_CTRL_SHM, (unsigned char *)&mcm, sizeof(mo_ctrm_t));
    prt->mri = (mo_repi_t *)mo_create_slot(msm, MO_RECORD_SHM, (unsigned char *)&mri, sizeof(mo_repi_t));
    prt->msr = (mo_sr_t *)mo_create_slot(msm, MO_SAMPLING_SHM, (unsigned char *)&msr, sizeof(mo_sr_t));

    /* notify uri */
    /* one process up, one notify */

#ifdef HAS_CURL
    char *buf;
    spprintf(&buf, 0, "{\"message\":\"molten is up\", \"ip\":\"%s\"}", ip);
    send_data_by_http(notify_uri, buf);
    efree(buf);
#endif
    return 0;
}
/* }}} */

/* {{{ mo_ctrl_dtor */
void mo_ctrl_dtor(mo_ctrl_t *prt)
{
    //array_free_persist(&prt->mri->capture_host);
    //pefree(prt->msr, 1);
}
/* }}} */

/* {{{ serrialize all message */
void mo_ctrl_serialize_msg(mo_ctrl_t *mrt, char **buf)
{
    /* prometheus metrics */
    spprintf(buf, 0, 
        "# HELP molten_request_all Number of all request.\n"
        "# TYPE molten_request_all counter\n"
        "molten_request_all %ld\n"
        "# HELP molten_request_capture Number of request be capture.\n"
        "# TYPE molten_request_capture counter\n"
        "molten_request_capture %ld\n"
        "# HELP molten_sampling_type the type of sampling.\n"
        "# TYPE molten_sampling_type gauge\n"
        "molten_sampling_type %d\n"
        "# HELP molten_sampling_rate the rate of sampling.\n"
        "# TYPE molten_sampling_rate gauge\n"
        "molten_sampling_rate %ld\n"
        "# HELP molten_sampling_request the request be capture one min.\n"
        "# TYPE molten_sampling_request gauge\n"
        "molten_sampling_request %ld\n"
        "# HELP molten_version current molten span version.\n"
        "# TYPE molten_version gauge\n"
        "molten_version %s\n",                                     \
        mrt->mri->request_all, mrt->mri->request_capture,               \
        mrt->mcm->sampling_type, mrt->mcm->sampling_rate,               \
        mrt->mcm->sampling_request,                                     \
        SPAN_VERSION 
    );
}
/* }}} */

/* unpack simple json */
/* 
 *  {"change_time":1121212121, "language":"php", "enable":1, "sampling_rate":11, "sampling_type":1, "sampling_request":100}
 */
int mo_ctrl_update_sampling(char *rec, mo_ctrm_t *mcm)
{
    if (rec == NULL) {
        return -1;
    }

    zval ret;
    php_json_decode_ex(&ret, rec, strlen(rec), PHP_JSON_OBJECT_AS_ARRAY, 256);
    
    if (MO_Z_TYPE_P(&ret) != IS_ARRAY) {
        return -1;
    }

    HashTable *ht = Z_ARRVAL(ret);
    /* retrive date from json */
    zval *tmp;
    
    /* if here is change, we will do after */
    if (mo_zend_hash_zval_find(ht, "enable", sizeof("enable"), (void **)&tmp) == SUCCESS) {
        convert_to_long(tmp);
        if (Z_LVAL_P(tmp) == 0) {
            mcm->enable = 0;
            return 0;
        } else {
            mcm->enable = 1;
        }
    }
    
    /* determine witch sampling type */
    if (mo_zend_hash_zval_find(ht, "samplingType", sizeof("samplingType"), (void **)&tmp) == SUCCESS) {
       convert_to_long(tmp);
       mcm->sampling_type = Z_LVAL_P(tmp);
    }

    if (mcm->sampling_type == SAMPLING_RATE) {
        if (mo_zend_hash_zval_find(ht, "samplingRate", sizeof("samplingRate"), (void **)&tmp) == SUCCESS) {
            convert_to_long(tmp);
            mcm->sampling_rate = Z_LVAL_P(tmp);
        }
    } else {
        if (mo_zend_hash_zval_find(ht, "samplingRequest", sizeof("samplingRequest"), (void **)&tmp) == SUCCESS) {
            convert_to_long(tmp);
            mcm->sampling_request = Z_LVAL_P(tmp);
        }
    }

    zval_dtor(&ret);

    return 0;
}

/* {{{ record message */
void mo_ctrl_record(mo_ctrl_t *mrt, int is_sampled)
{
    /* every request will do this */
    __sync_add_and_fetch(&mrt->mri->request_all, 1);
    
    /* todo something error 
    zval *http_host;
    if (find_server_var("HTTP_HOST", sizeof("HTTP_HOST"), (void **)&http_host) == SUCCESS) {
        add_assoc_bool(&prt->mri->capture_host, (Z_STRVAL_P(http_host)), 1);
    }
    */
    
    if (is_sampled == 1) {
        __sync_add_and_fetch(&mrt->mri->request_capture, 1);
    }
}
/* }}} */

/* {{{ control sampling module,  must after is_cli judege */
void mo_ctrl_sampling(mo_ctrl_t *prt, mo_chain_t *pct)
{
    /* determine sample or not */
    /* if not sampled this function will return direct */
    zval *sampled = NULL;
    int sampled_ret = FAILURE;
    if (pct->is_cli != 1) {
        sampled_ret = find_server_var(MOLTEN_REC_SAMPLED, sizeof(MOLTEN_REC_SAMPLED), (void **)&sampled);
    }

    /* default is 0 */
    pct->pch.is_sampled = 0;
     
    if ((sampled_ret == SUCCESS) && (sampled != NULL) && (Z_TYPE_P(sampled) == IS_STRING) ) {
        if (strncmp(Z_STRVAL_P(sampled), "1", 1) == 0) {
            pct->pch.is_sampled = 1;
        } else {
            pct->pch.is_sampled = 0;
        }
    } else {

        /* sampling rate */
        if (prt->mcm->sampling_type == SAMPLING_RATE) {
            if (check_hit_ratio(prt->mcm->sampling_rate,100)) {
                pct->pch.is_sampled = 1;
            } else {
                pct->pch.is_sampled = 0;
            }
        } else {

            /* sampling by r/m */
            long min = mo_time_m();
            /* todo will not sync if not lock, here we can add a spin lock */
            if (min == prt->msr->last_min) {
                __sync_add_and_fetch(&prt->msr->request_num, 1);
            } else {
                prt->msr->request_num = 0;
                prt->msr->last_min = min;
            }

            if (prt->msr->request_num < prt->mcm->sampling_request) {
                pct->pch.is_sampled = 1;
            } else {
                pct->pch.is_sampled = 0;
            }
        }  
    }
}
/* }}} */
