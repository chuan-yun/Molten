
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

/* clear sock */
static inline void clear_sock(mo_ctrl_t *prt)
{
    if (prt->sock > 0) {
        close(prt->sock);
    }
    prt->sock = -1;
}

/* connect sock */
static inline int connect_sock(mo_ctrl_t *prt)
{
    /* init unix sock */  
    struct sockaddr_un server;
    struct timeval timeout;
    int len;
    prt->sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (prt->sock < 0) {
        clear_sock(prt);
        return -1;
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, prt->domain_path);
    len = strlen(server.sun_path) + sizeof(server.sun_family);

    // set read and write timeout 
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;    /* set to 10 ms */
    if (setsockopt(prt->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        clear_sock(prt);
        return -1;
    }

    if (setsockopt(prt->sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        clear_sock(prt);
        return -1;
    }
    
    if (connect(prt->sock, (struct sockaddr *)&server, len) < 0) {
        clear_sock(prt);
        return -1;
    }

    return 0;
}

/* dispose network error */
static inline void dispose_error(mo_ctrl_t *prt)
{
    /* for after error , havn`t to reconnect */
    /* if timeout errno = EAGAIN or EWOULDBLOCK */
    //if (errno != EAGAIN || errno != EWOULDBLOCK || errno != EINTR) {
    if (errno != EINTR) {
        clear_sock(prt);
        connect_sock(prt);
    }
}

/* {{{ mo_ctrl_ctor */
/* use tcp(stream) to keep data stable */
/* if use udp server need use funciton recvfrom and sendto */
/* current use tcp , so we use read and write to set timeout (: */
int mo_ctrl_ctor(mo_ctrl_t *prt, char *domain_path, int req_interval, int sampling_type, long sampling_rate, long sampling_request)
{
    memset(prt->domain_path, 0x00, sizeof(prt->domain_path));
    strcpy(prt->domain_path, domain_path);

    prt->last_req_time = 0;
    prt->req_interval = req_interval;
    prt->pcm = pemalloc(sizeof(mo_ctrm_t), 1);
    prt->pri = pemalloc(sizeof(mo_repi_t), 1);
    prt->psr = pemalloc(sizeof(mo_sr_t), 1);

    /* init ctrm */
    prt->pcm->change_time       = 0;
    prt->pcm->enable            = 1;
    prt->pcm->sampling_type     = sampling_type;
    prt->pcm->sampling_rate     = sampling_rate;    
    prt->pcm->sampling_request  = sampling_request;

    /* init repi */
    ZVAL_LONG(&prt->pri->request_all, 0);
    ZVAL_LONG(&prt->pri->request_capture, 0);
    array_init_persist(&prt->pri->capture_host ZEND_FILE_LINE_CC);
    
    if (connect_sock(prt) == -1) {
        /* record log */
        MOLTEN_ERROR("molten ctrl ctor connect sock errno:[%d], str:[%s]", errno, strerror(errno)); 
        return -1;
    }

    return 0;
}
/* }}} */

/* {{{ mo_ctrl_dtor */
void mo_ctrl_dtor(mo_ctrl_t *prt)
{
    pefree(prt->pcm, 1);
    array_free_persist(&prt->pri->capture_host);
    pefree(prt->pri, 1);
    pefree(prt->psr, 1);
    close(prt->sock);
}
/* }}} */

/* {{{ check req interval already */
static int inline check_interval(mo_ctrl_t *prt)
{
    long sec = mo_time_sec();
    if ((sec - prt->last_req_time) > prt->req_interval) {
        prt->last_req_time = sec;
        return 1;
    } else {
        return 0;
    }
}
/* }}} */

/* pack simple json */
/* {"request_all":1121, "request_capture":1212, "capture_host":["xxx.com", "xxx.info"]} */
static void pack_message(smart_string *send, mo_repi_t *pri)
{
    zval *pack;
    MO_ALLOC_INIT_ZVAL(pack);
    array_init(pack);
    mo_add_assoc_string(pack,   "language",             "php", 1);
    add_assoc_zval(pack,        "requestAll",           &pri->request_all);
    add_assoc_zval(pack,        "requestCapture",       &pri->request_capture);
    add_assoc_zval(pack,        "captureHost",          &pri->capture_host);
    mo_add_assoc_string(pack,   "version",              MOLTEN_EXT_VERSION, 1);
    
    mo_php_json_encode(send, pack, 0);

#if PHP_VERSION_ID < 70000
    zval_ptr_dtor(&pack);
#else
    zval_ptr_dtor(pack);
#endif
    MO_FREE_ALLOC_ZVAL(pack);
}

/* unpack simple json */
/* 
 *  {"change_time":1121212121, "language":"php", "enable":1, "sampling_rate":11, "sampling_type":1, "sampling_request":100}
 */
static void unpack_message(smart_string *r, mo_ctrm_t *pcm)
{
    if (r == NULL) {
        return;
    }

    char *rec = smart_string_str((*r));
    zval ret;
    php_json_decode_ex(&ret, rec, strlen(rec), PHP_JSON_OBJECT_AS_ARRAY, 256);
    
    if (MO_Z_TYPE_P(&ret) != IS_ARRAY) {
        return;
    }

    HashTable *ht = Z_ARRVAL(ret);
    /* retrive date from json */
    zval *tmp;
    if (mo_zend_hash_zval_find(ht, "changeTime", sizeof("changeTime"), (void **)&tmp) == SUCCESS) {
        /* equeal to change time if not different , do nothting */ 
        convert_to_long(tmp);
        if(pcm->change_time >= Z_LVAL_P(tmp)) {
            return;
        } else {
            pcm->change_time = Z_LVAL_P(tmp);
        }
    }
    
    /* if here is change, we will do after */
    if (mo_zend_hash_zval_find(ht, "enable", sizeof("enable"), (void **)&tmp) == SUCCESS) {
        convert_to_long(tmp);
        if (Z_LVAL_P(tmp) == 0) {
            pcm->enable = 0;
            return;
        } else {
            pcm->enable = 1;
        }
    }
    
    /* determine witch sampling type */
    if (mo_zend_hash_zval_find(ht, "samplingType", sizeof("samplingType"), (void **)&tmp) == SUCCESS) {
       convert_to_long(tmp);
       pcm->sampling_type = Z_LVAL_P(tmp);
    }

    if (pcm->sampling_type == SAMPLING_RATE) {
        if (mo_zend_hash_zval_find(ht, "samplingRate", sizeof("samplingRate"), (void **)&tmp) == SUCCESS) {
            convert_to_long(tmp);
            pcm->sampling_rate = Z_LVAL_P(tmp);
        }
    } else {
        if (mo_zend_hash_zval_find(ht, "samplingRequest", sizeof("samplingRequest"), (void **)&tmp) == SUCCESS) {
            convert_to_long(tmp);
            pcm->sampling_request = Z_LVAL_P(tmp);
        }
    }

    zval_dtor(&ret);
}

/* {{{ record message */
void mo_ctrl_record(mo_ctrl_t *prt, int is_sampled)
{
    /* every request will do this */
    ZVAL_LONG_PLUS(&prt->pri->request_all);
    
    /* todo something error 
    zval *http_host;
    if (find_server_var("HTTP_HOST", sizeof("HTTP_HOST"), (void **)&http_host) == SUCCESS) {
        add_assoc_bool(&prt->pri->capture_host, (Z_STRVAL_P(http_host)), 1);
    }
    */
    

    if (is_sampled == 1) {
        ZVAL_LONG_PLUS(&prt->pri->request_capture);
    }
}
/* }}} */

/* {{{ report and recive date */
/* define protocol linke http , clent send report info, server send control info */
/* we must avoid dead lock */
void mo_ctrl_sr_data(mo_ctrl_t *prt)
{
    /* check interval */
    if (check_interval(prt)) {

        int             ret;
        smart_string    rec = {0};
        smart_string    s = {0};
        char            buf[REC_DATA_SIZE] = {0};

        if (prt->sock < 0) {
            if (connect_sock(prt) < 0) {
                return;
            }
        }
        
        /* pack message */
        pack_message(&s, prt->pri);

        /* send msg set to block */
        ret = send(prt->sock, smart_string_str(s), smart_string_len(s), 0);
        
        smart_string_free(&s);

        /* if peer down, reconnet too */
        if (ret <= 0) {
            dispose_error(prt);
            MOLTEN_ERROR("molten report data error, errno:[%d], str:[%s]", errno, strerror(errno)); 
            return;
        } else {
            /* send success clear report info */
            ZVAL_LONG(&prt->pri->request_capture,   0);
            ZVAL_LONG(&prt->pri->request_all,       0);
        }

        /* recv msg set to block */
        do{
            ret = recv(prt->sock, buf, REC_DATA_SIZE, 0);
            if (ret > 0) {
                smart_string_appendl(&rec, buf, ret);
                if (ret < REC_DATA_SIZE) {
                    smart_string_0(&rec);
                    break;
                }
            }
        }while(ret > 0);


        if (ret <= 0) {
            dispose_error(prt);
            MOLTEN_ERROR("molten read data error, errno:[%d], str:[%s]", errno, strerror(errno)); 
            return;
        }

        unpack_message(&rec, prt->pcm);
        smart_string_free(&rec);
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
        if (prt->pcm->sampling_type == SAMPLING_RATE) {
            if (check_hit_ratio(prt->pcm->sampling_rate)) {
                pct->pch.is_sampled = 1;
            } else {
                pct->pch.is_sampled = 0;
            }
        } else {

            /* sampling by r/m */
            /* todo here can use share memory for actural data, but use memory lock will effect performance */
            long min = mo_time_m();
            if (min == prt->psr->last_min) {
                prt->psr->request_num++; 
            } else {
                prt->psr->request_num = 0;
                prt->psr->last_min = min;
            }

            if (prt->psr->request_num < prt->pcm->sampling_request) {
                pct->pch.is_sampled = 1;
            } else {
                pct->pch.is_sampled = 0;
            }
        }  
    }
}
/* }}} */
