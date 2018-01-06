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
#include "molten_shm.h"

#define SLOT_ELEMENT(mss,offset) ((mo_slot_t *)(mss + offset))

/* {{{ status module ctor */
void mo_shm_ctor(mo_shm_t *msm)
{
    msm->mm        = NULL;
    int slot_num    = MO_MAX_SLOT;
    msm->shm_size   = sizeof(mo_slot_header_t) + slot_num * sizeof(mo_slot_t);
    msm->shm_key    = ftok(SHM_PATH, SHM_PROJECT_ID);
    if (msm->shm_key == -1) {
        MOLTEN_ERROR("[shm] ftok error:%d", errno);
        return;
    }
    
    /* if exist, just map it */
    msm->shm_id = shmget(msm->shm_key, msm->shm_size, IPC_CREAT|0666);
    if (msm->shm_id == -1) {
        MOLTEN_ERROR("[shm] shmget error:%d", errno);
        return;
    }

    /* get shm */
    msm->mm = shmat(msm->shm_id, NULL, 0);
    if (msm->mm == (void *) -1) {
        MOLTEN_ERROR("[shm] shmat error:%d", errno);
        return;
    }
    
    /* lock ctx init */ 
    if (mo_lock_init() == -1) {
        MOLTEN_ERROR("[shm] mo lock init error");
    }
    
    /* map msm to msh */
    msm->msh = (mo_slot_header_t *)msm->mm;
    msm->mss = (mo_slot_t *)(msm->mm + sizeof(mo_slot_header_t));

    /* init lock success */
    /* for all process use one file lock */
    memcpy(msm->init_lock.lock_path, "/tmp/.molten.dddddd", sizeof("/tmp/.molten.dddddd"));
    if (mo_fcntl_locK_init(&(msm->init_lock)) == 0 ) {

        /* check && init slot header */ 
        mo_fcntl_wlock(&msm->init_lock);
        if (msm->msh->magic != MO_SLOT_HEADER_MAGIC || msm->msh->init != 1) {
            msm->msh->magic     = MO_SLOT_HEADER_MAGIC;
            msm->msh->init      = 1;
            msm->msh->slot_num  = slot_num;
        }
        msm->msh->attach_num++;
        mo_fcntl_wunlock(&msm->init_lock);
    } else {
        MOLTEN_ERROR("[shm] fcntl lock error");
    }
}
/* }}} */

/* {{{ create slot */
unsigned char *mo_create_slot(mo_shm_t *msm, int slot, unsigned char *data, int size)
{
    if (size > PAYLOAD_SIZE) {
        return NULL;
    }
    
    /* lock header */ 
    if (SLOT_ELEMENT(msm->mss, slot)->init != 1                             \
        || (SLOT_ELEMENT(msm->mss, slot))->magic != (MO_SLOT_MAGIC + slot)) {
         SLOT_ELEMENT(msm->mss, slot)->magic       = MO_SLOT_MAGIC + slot;
         SLOT_ELEMENT(msm->mss, slot)->init        = 1;
         SLOT_ELEMENT(msm->mss, slot)->lenght      = size;
         memset(SLOT_ELEMENT(msm->mss, slot)->payload, 0x00, PAYLOAD_SIZE);
         memcpy(SLOT_ELEMENT(msm->mss, slot)->payload, data, size);
    }
    
    return SLOT_ELEMENT(msm->mss, slot)->payload; 
}
/* }}} */

/* {{{ mo relase slot */
void mo_realse_slot(mo_shm_t *msm, int slot)
{
    if (!((SLOT_ELEMENT(msm->mss, slot))->init == 1                             \
        && ((SLOT_ELEMENT(msm->mss, slot))->magic == (MO_SLOT_MAGIC + slot)))) {
         SLOT_ELEMENT(msm->mss, slot)->magic    = 0;
         SLOT_ELEMENT(msm->mss, slot)->init        = 0;
         SLOT_ELEMENT(msm->mss, slot)->lenght      = 0;
         memset(SLOT_ELEMENT(msm->mss, slot)->payload, 0x00, PAYLOAD_SIZE);
    }
}
/* }}} */


/* {{{ molten shm dtor */
void mo_shm_dtor(mo_shm_t *msm)
{
    memcpy(msm->init_lock.lock_path, "/tmp/.molten.dddddd", sizeof("/tmp/.molten.dddddd"));

    mo_fcntl_wlock(&msm->init_lock);
    int i = 0;

    /* release slot header and slot */
    /* just build with gcc */
    if (__sync_sub_and_fetch(&msm->msh->attach_num, 1) == 0) {
        for(; i< msm->msh->slot_num; i++) {
             mo_realse_slot(msm, i);
        }
        msm->msh->init = 0;
        msm->msh->slot_num = 0;
        msm->msh->magic = 0;
    }
    
    shmdt(msm->mm);
    /* check nattch, if nattch == 0, delete shm */
    struct shmid_ds buf;
    if (shmctl(msm->shm_id, IPC_STAT, &buf) == 0) {
        if (buf.shm_nattch == 0) {
            if (shmctl(msm->shm_id, IPC_RMID, NULL) == -1) {
                MOLTEN_ERROR("[shm] shmctl IPC_RMID error:%d", errno);
            }
        }
    } else {
        MOLTEN_ERROR("[shm] shmctl IPC_STAT error:%d", errno);
    }

    mo_fcntl_wunlock(&msm->init_lock);

    mo_fcntl_lock_destroy(&msm->init_lock);
}
/* }}} */
