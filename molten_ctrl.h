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

#ifndef MOLTEN_CTRL_H
#define MOLTEN_CTRL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>

#include "php.h"

#include "molten_util.h"
#include "molten_struct.h"
#include "molten_shm.h"
#include "ext/json/php_json.h"
#include "php7_wrapper.h"
#include "SAPI.h"

#define SAMPLING_RATE   1
#define SAMPLING_REQ    1<<1

#define REC_DATA_SIZE   1024

#define CTRL_ENABLE(prt) ((prt)->pcm->enable == 1)


/* molten control detail */
typedef struct {
    long        change_time;
    uint8_t     enable;
    uint8_t     sampling_type;
    long        sampling_rate;
    long        sampling_request;       /* sampling request/minute for ervery server */
} mo_ctrm_t;

/* molten report info */
/* this record info will be clear after every send */
typedef struct {
   unsigned long request_all;                /* request all (long)       */
   unsigned long request_capture;            /* request capture (long)   */
   //zval capture_host;                      /* capture host (array)     */
} mo_repi_t;

/* sampling for request limit */
typedef struct {
    long last_min;                  /* last min */
    long request_num;               /* request num */
} mo_sr_t;

/* molten control module */
typedef struct{
    /*
    int         sock;  
    char        domain_path[4096];
    */
    long        last_req_time;             /* last request time microtime */
    long        req_interval;                /* request interval microtime */
    mo_ctrm_t   *mcm;                      /* ctrol module */
    mo_repi_t   *mri;
    mo_sr_t     *msr;
} mo_ctrl_t;            

int mo_ctrl_ctor(mo_ctrl_t *prt, mo_shm_t *mst, char *notify_uri, char *ip, long sampling_type, long sampling_rate, long sampling_request);
void mo_ctrl_dtor(mo_ctrl_t *prt);
void mo_ctrl_serialize_msg(mo_ctrl_t *mrt, char **buf);
void mo_ctrl_record(mo_ctrl_t *mrt, int is_sampled);
int mo_ctrl_update_sampling(char *rec, mo_ctrm_t *mcm);
void mo_ctrl_sampling(mo_ctrl_t *prt, mo_chain_t *pct);
#endif
