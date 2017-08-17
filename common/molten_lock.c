
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
#include "molten_lock.h"

#ifndef MO_SPIN_LOCK 
pthread_rwlockattr_t    mo_lock_attr;
#endif

/* {{{ lock create */
int mo_lock_create(mo_lock_t *lock)
{
#ifndef MO_SPIN_LOCK 
    if (pthread_rwlock_init(lock, &mo_lock_attr) != LOCK_SUCCESS) {
        return -1;
    }
#else
    *lock = 0;
#endif
    return 0;
}
/* }}} */

/* {{{ mo read lock */
void mo_lock_rlock(mo_lock_t *lock)
{
#ifndef MO_SPIN_LOCK 
    pthread_rwlock_rdlock(lock);
#else
    /* todo spin lock */
#endif
}
/* }}} */

/* {{{ mo write lock */
void mo_lock_wlock(mo_lock_t *lock)
{
#ifndef MO_SPIN_LOCK 
    pthread_rwlock_wrlock(lock);
#else
#endif
}
/* }}} */

/* {{{ mo read unlock */
void mo_lock_runlock(mo_lock_t *lock)
{
#ifndef MO_SPIN_LOCK 
    pthread_rwlock_unlock(lock);
#else
#endif
}
/* }}} */

/* {{{ mo write unlock */
void mo_lock_wunlock(mo_lock_t *lock)
{
#ifndef MO_SPIN_LOCK 
    pthread_rwlock_unlock(lock);
#else
#endif
}
/* }}} */



/* {{{ molten lock destroy */
int mo_lock_destroy(mo_lock_t *lock)
{
#ifndef MO_SPIN_LOCK 
    if (pthread_rwlock_destroy(lock) != LOCK_SUCCESS) {
        return -1;
    }
#endif
    return 0; 
}
/* }}} */

/* {{{ lock init */
int mo_lock_init()
{
#ifndef MO_SPIN_LOCK 
    if (pthread_rwlockattr_init(&mo_lock_attr) == LOCK_SUCCESS) {
        if (pthread_rwlockattr_setpshared(&mo_lock_attr, PTHREAD_PROCESS_SHARED) == LOCK_SUCCESS) {
            return 0;
        }
    }
#else
    return 0;
#endif
    return -1;
}
/* }}} */

/* {{{ fcntl lock call */
static int mo_fcntl_call(int fd, int cmd, int type, off_t offset, int whence, off_t len) 
{
    int ret;
    struct flock lock;
    
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
    lock.l_pid = 0;
    
    do {
        ret = fcntl(fd, cmd, &lock) ;
    } while(ret < 0 && errno == EINTR);
    
    return(ret);
}
/* }}} */

/* {{{ fcntl file lock */
void mo_fcntl_wlock(mo_fcntl_lock_t *lock)
{
    mo_fcntl_call(lock->fd, F_SETLKW, F_WRLCK, 0, SEEK_SET, 0);
}
/* }}} */

/* {{{ molten fcntl wunlock */
void mo_fcntl_wunlock(mo_fcntl_lock_t *lock)
{
    mo_fcntl_call(lock->fd, F_SETLKW, F_UNLCK, 0, SEEK_SET, 0);
}
/* }}} */

/* {{{ fcntl lock init */
int mo_fcntl_locK_init(mo_fcntl_lock_t *lock)
{
    if (lock->lock_path == NULL) {
        memcpy(lock->lock_path, FCNTL_LOCK_PATH, LOCK_PATH_SIZE); 
        mktemp(lock->lock_path);       
    }
    lock->fd = open(lock->lock_path, O_RDWR|O_CREAT, 0666);
    if (lock->fd > 0) {
        unlink(lock->lock_path); 
        return 0;
    } else {
        return -1;
    }
}
/* }}} */

/* {{{ destry fcntl lock */
void mo_fcntl_lock_destroy(mo_fcntl_lock_t *lock)
{
    close(lock->fd);
}
/* }}} */
