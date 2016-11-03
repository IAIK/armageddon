/* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include "configuration.h"
#include "lock.h"

#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>

#if LOCK_ROUND_ROBIN == 1
static unsigned int fork_idx = 0;
#endif

inline void tal_init(lock_t* lock, lock_attr_t* attr)
{
  /* Set attributes */
  if (attr != NULL) {
    memcpy(&(lock->attr), attr, sizeof(lock_attr_t));
  }

#ifdef WITH_POSIX_THREAD_PROCESS_SHARED
  if (pthread_mutexattr_init(&(lock->mtx_attr)) != 0) {
    fprintf(stderr, "Error: Could not init mutexattr\n");
  }

  if (pthread_mutexattr_setpshared(&(lock->mtx_attr), PTHREAD_PROCESS_SHARED) != 0) {
    fprintf(stderr, "Error: Could not set pshared\n");
  }

  if (pthread_mutex_init(&(lock->mtx), &(lock->mtx_attr)) != 0) {
    fprintf(stderr, "Error: Could not initalize mutex\n");
  }
#else
#if LOCK_ROUND_ROBIN == 1
  fork_idx = attr->fork_idx;
  lock->current_idx = fork_idx;
#else
  atomic_flag_clear(&(lock->cat));
#endif
#endif
}

inline void tal_lock(lock_t* lock)
{
#ifdef WITH_POSIX_THREAD_PROCESS_SHARED
  if (pthread_mutex_lock(&(lock->mtx)) != 0) {
    fprintf(stderr, "Error: Could not lock\n");
  }
#else
#if LOCK_ROUND_ROBIN == 1
  while (lock->current_idx != fork_idx) {
    sched_yield();
  }
#else
  while (atomic_flag_test_and_set(&lock->cat)) {
    sched_yield();
  };
#endif
#endif
}

inline void tal_unlock(lock_t* lock)
{
#ifdef WITH_POSIX_THREAD_PROCESS_SHARED
  if (pthread_mutex_unlock(&(lock->mtx)) != 0) {
    fprintf(stderr, "Error: Could not unlock\n");
  }
#else
#if LOCK_ROUND_ROBIN == 1
  lock->current_idx = (fork_idx + 1) % lock->attr.number_of_forks;
  sched_yield();
#else
  atomic_flag_clear(&(lock->cat));
  sched_yield();
#endif
#endif
}
