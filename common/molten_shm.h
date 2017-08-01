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

#ifndef MOLTEN_SHM_H
#define MOLTEN_SHM_H

#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <molten_lock.h>

/* max payload one slot */
#define PAYLOAD_SIZE    1024

/* max slot */
#define MO_MAX_SLOT     16          /* shm max slot */

/* slot mean */
#define MO_CTRL_SHM         0           /* shm ctrl module info for every process */
#define MO_RECORD_SHM       1           /* shm detail info for store */
#define MO_SAMPLING_SHM     2           /* shm detail info for store */

/* slot magic */
#define MO_SLOT_MAGIC           0xa78ac1d5
#define MO_SLOT_HEADER_MAGIC    0xb77c81ba

/* shm path */
#define SHM_PATH            "/tmp"
#define SHM_PROJECT_ID      7312

/* molten shm slot */
typedef struct {
    int             magic;
    int             init;
    int             lenght;
    unsigned char   payload[PAYLOAD_SIZE];
} mo_slot_t;

/* shm slot header */
typedef struct {
    int                 init;
    int                 attach_num;     /* the num of php extension attach */
    int                 magic;
    int                 slot_num; 
} mo_slot_header_t;

/* molten status */
typedef struct {
    /* shm */
    int                 shm_size;       /* shm size */
    key_t               shm_key;        /* shm key */
    int                 shm_id;         /* shm id */
    
    /* slot */
    mo_fcntl_lock_t     init_lock;      /* molten status lock for init */ 
    void                *mm;            /* alloc memory */
    mo_slot_header_t    *msh;           /* molten slot header */
    mo_slot_t           *mss;           /* status shm */ 

    /* status uri */
} mo_shm_t;

unsigned char *mo_create_slot(mo_shm_t *mst, int slot, unsigned char *data, int size);
#endif
