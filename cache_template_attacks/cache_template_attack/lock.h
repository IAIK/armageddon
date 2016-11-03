 /* See LICENSE file for license and copyright information */

#ifndef LOCK_H
#define LOCK_H

#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

#include "configuration.h"

#ifdef WITH_POSIX_THREAD_PROCESS_SHARED
#ifndef _POSIX_THREAD_PROCESS_SHARED
#error This system does not support process shared mutex
#endif
#endif

typedef struct lock_attr_s {
#if LOCK_ROUND_ROBIN == 1
  unsigned int number_of_forks;
  unsigned int fork_idx;
#else
  void* x;
#endif
} lock_attr_t;

typedef struct lock_s {
#ifdef WITH_POSIX_THREAD_PROCESS_SHARED
  pthread_mutex_t mtx;
  pthread_mutexattr_t mtx_attr;
#else
#if LOCK_ROUND_ROBIN == 1
  atomic_uint current_idx;
#else
  volatile atomic_flag cat;
#endif

  lock_attr_t attr;
#endif
} lock_t;

void tal_init(lock_t* lock, lock_attr_t* attr);
void tal_lock(lock_t* lock);
void tal_unlock(lock_t* lock);

#endif  /*LOCK_H*/
