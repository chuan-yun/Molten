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

#ifndef MOLTEN_CHAIN_H
#define MOLTEN_CHAIN_H

#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "php7_wrapper.h"
#include "php.h"
#include "SAPI.h"

#include "molten_log.h"
#include "molten_struct.h"
#include "molten_span.h"
#include "molten_util.h"

#define MO_MAX_KEY_LEN             256
#define MO_MAX_VAL_LEN             256 
#define MO_DEFAULT_ID              "0"

#define zend_true                   1
#define zend_flase                  0

#define ADD_HASH_MOLTEN_KEY(ht, pck) mo_zend_hash_update(ht, pck->name, (strlen(pck->name) + 1), \
        (void *)&pck, sizeof(mo_chain_key_t *), NULL)

void mo_chain_ctor(mo_chain_t *pct, mo_chain_log_t *pcl, char *service_name, char *ip);
void mo_chain_dtor(mo_chain_t *pct, mo_span_builder *psb);
char *mo_rebuild_url(mo_chain_t *pct, char *ori_url);
void build_http_header(mo_chain_t *pct, zval *header, char *span_id);
void mo_build_chain_header(mo_chain_t *pct, char *ip);
void mo_obtain_local_ip(char *ip);
#endif
