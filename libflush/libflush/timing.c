/* See LICENSE file for license and copyright information */

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <sched.h>

#include "libflush.h"
#include "timing.h"
#include "internal.h"

#if TIME_SOURCE == TIME_SOURCE_MONOTONIC_CLOCK
#include <time.h>

uint64_t
get_monotonic_time(void)
{
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  return t1.tv_sec * 1000*1000*1000ULL + t1.tv_nsec;
}
#endif

#if TIME_SOURCE == TIME_SOURCE_PERF
#include <assert.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>

inline bool
perf_init(libflush_session_t* session, libflush_session_args_t* args)
{
  (void) session;
  (void) args;

  static struct perf_event_attr attr;
  attr.type = PERF_TYPE_HARDWARE;
  attr.config = PERF_COUNT_HW_CPU_CYCLES;
  attr.size = sizeof(attr);
  attr.exclude_kernel = 1;
  attr.exclude_hv = 1;
  attr.exclude_callchain_kernel = 1;

  session->perf.fd = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
  assert(session->perf.fd >= 0 && "if this assertion fails you have no perf event interface available for the userspace. install a different kernel/rom."); // if this assertion fails you have no perf event interface available for the userspace. install a different kernel/rom.

  return true;
}

inline bool
perf_terminate(libflush_session_t* session)
{
  close(session->perf.fd);

  return true;
}

inline uint64_t
perf_get_timing(libflush_session_t* session)
{
  long long result = 0;

  if (read(session->perf.fd, &result, sizeof(result)) < (ssize_t) sizeof(result)) {
    return 0;
  }

  return result;
}

inline void
perf_reset_timing(libflush_session_t* session)
{
  ioctl(session->perf.fd, PERF_EVENT_IOC_RESET, 0);
}
#endif

#if TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
static void* thread_counter_func(void*);
#include <pthread.h>

inline bool
thread_counter_init(libflush_session_t* session, libflush_session_args_t* args)
{
  if (session == NULL) {
    return false;
  }

  session->thread_counter.data.cpu = (args != NULL) ? (ssize_t) args->bind_to_cpu : -1;
  session->thread_counter.data.session = session;

  if (pthread_create(&(session->thread_counter.thread), NULL,
        thread_counter_func, &(session->thread_counter.data)) != 0) {
    return false;
  }

  return true;
}

inline bool
thread_counter_terminate(libflush_session_t* session)
{
  if (session == NULL) {
    return false;
  }

#if __BIONIC__
  pthread_kill(session->thread_counter.thread, SIGUSR1);
#else
  pthread_cancel(session->thread_counter.thread);
#endif
  pthread_join(session->thread_counter.thread, NULL);

  return true;
}

inline uint64_t
thread_counter_get_timing(libflush_session_t* session)
{
  libflush_memory_barrier(session);

  uint64_t time = session->thread_counter.value;

  libflush_memory_barrier(session);

  return time;
}

#if __BIONIC__
static void
thread_exit_handler(int sig)
{
  (void) sig;

  pthread_exit(NULL);
}
#endif

static void*
thread_counter_func(void* data) {
#ifndef __BIONIC__
  /* Set cancel able */
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

  /* Unpack data */
  thread_data_t* thread_data = (thread_data_t*) data;
  libflush_session_t* session = thread_data->session;
  ssize_t cpu = thread_data->cpu;

  /* Bind to CPU */
  if (cpu > 0) {
    if (libflush_bind_to_cpu(cpu) == false) {
      fprintf(stderr, "Could not bind to CPU: %zu\n", cpu);
    } else {
      fprintf(stderr, "Bind thread to CPU: %zu\n", cpu);
    }
  }

#if __BIONIC__
  /* Setup cancel signal */
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  memset(&action.sa_mask, 0, sizeof(sigset_t));
  action.sa_flags = 0;
#if defined(__ARM_ARCH_7A__)
  action._u._sa_handler = thread_exit_handler;
#else
  action.sa_handler = thread_exit_handler;
#endif
  sigaction(SIGUSR1, &action, NULL);
#endif

  while (true) {
    session->thread_counter.value++;
  }

  pthread_exit(NULL);
}
#endif
