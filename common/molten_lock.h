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
#ifndef MOLTEN_LOCK_H
#define MOLTEN_LOCK_H

#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/* fcntl lock path */
#define FCNTL_LOCK_PATH     "/tmp/.molten.xxxxxx"
#define LOCK_PATH_SIZE      sizeof(FCNTL_LOCK_PATH)

#define LOCK_SUCCESS 0

/* only support spin lock and rw lock */
/* spin lock and pthread_rw_lock no need to destroy */
#ifndef MO_SPIN_LOCK 
typedef pthread_rwlock_t    mo_lock_t;
#else
typedef unsigned long       mo_lock_t;
#endif

/* molten fcntl lock */
typedef struct {
    char lock_path[LOCK_PATH_SIZE];
    int fd;
} mo_fcntl_lock_t;

int mo_lock_create(mo_lock_t *lock);
void mo_lock_rlock(mo_lock_t *lock);
void mo_lock_wlock(mo_lock_t *lock);
void mo_lock_runlock(mo_lock_t *lock);
void mo_lock_wunlock(mo_lock_t *lock);
int mo_lock_destroy(mo_lock_t *lock);
int mo_lock_init();
void mo_fcntl_wlock(mo_fcntl_lock_t *lock);
void mo_fcntl_wunlock(mo_fcntl_lock_t *lock);
int mo_fcntl_locK_init(mo_fcntl_lock_t *lock);
int mo_fcntl_lock_destroy(mo_fcntl_lock_t *lock);
#endif
