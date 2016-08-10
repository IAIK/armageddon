/* See LICENSE file for license and copyright information */

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <sched.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libflush.h"
#include "timing.h"
#include "internal.h"
#include "eviction/eviction.h"

#include <pthread.h>

#if defined(__ARM_ARCH_7A__)
#include "armv7/libflush.h"
#elif defined(__ARM_ARCH_8A__)
#include "armv8/libflush.h"
#elif defined(__i386__) || defined(__x86_64__)
#include "x86/libflush.h"
#endif

static uint64_t libflush_get_timing_start(libflush_session_t* session);
static uint64_t libflush_get_timing_end(libflush_session_t* session);

#if HAVE_PAGEMAP_ACCESS == 1
static size_t get_frame_number_from_pagemap(size_t value);
#endif

bool
libflush_init(libflush_session_t** session, libflush_session_args_t* args)
{
  (void) session;
  (void) args;

  if (session == NULL) {
    return false;
  }

  if ((*session = calloc(1, sizeof(libflush_session_t))) == NULL) {
    return false;
  }

  if (args != NULL) {
    (*session)->performance_register_div64 = args->performance_register_div64;
  }

#if HAVE_PAGEMAP_ACCESS == 1
  (*session)->memory.pagemap = open("/proc/self/pagemap", O_RDONLY);
  if ((*session)->memory.pagemap == -1) {
    free(*session);
    return false;
  }
#endif

  /* Initialize timer */
#if TIME_SOURCE == TIME_SOURCE_PERF
  perf_init(*session, args);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
  thread_counter_init(*session, args);
#endif

  /* Initialize eviction */
  libflush_eviction_init(*session, args);

  /* Initialize architecture */
#if defined(__ARM_ARCH_7A__)
  arm_v7_init(*session, args);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_init(*session, args);
#endif

  return true;
}

bool
libflush_terminate(libflush_session_t* session)
{
  (void) session;

  if (session == NULL) {
    return false;
  }

  /* Pagemap access */
#if HAVE_PAGEMAP_ACCESS == 1
  if (session->memory.pagemap >= 0) {
    close(session->memory.pagemap);
  }
  session->memory.pagemap = -1;
#endif

  /* Terminate timer */
#if TIME_SOURCE == TIME_SOURCE_PERF
  perf_terminate(session);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
  thread_counter_terminate(session);
#endif

  /* Terminate eviction */
  libflush_eviction_terminate(session);

  /* Terminate architecture */
#if defined(__ARM_ARCH_7A__)
  arm_v7_terminate(session);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_terminate(session);
#endif

  /* Clean-up */
  free(session);

  return true;
}

void
libflush_flush(libflush_session_t* session, void* address)
{
  (void) session;

#if USE_EVICTION == 1
  libflush_eviction_evict(session, address);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_flush(address);
#elif defined(__i386__) || defined(__x86_64__)
  x86_flush(address);
#else
#error No flush/eviction method available on this platform
#endif
}

uint64_t
libflush_flush_time(libflush_session_t* session, void* address)
{
  (void) session;

  uint64_t start = libflush_get_timing(session);

#if USE_EVICTION == 1
  libflush_eviction_evict(session, address);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_flush(address);
#elif defined(__i386__) || defined(__x86_64__)
  x86_flush(address);
#else
#error No flush/eviction method available on this platform
#endif

  return libflush_get_timing(session) - start;
}

void
libflush_evict(libflush_session_t* session, void* address)
{
  libflush_eviction_evict(session, address);
}

uint64_t
libflush_evict_time(libflush_session_t* session, void* address)
{
  uint64_t start = libflush_get_timing(session);
  libflush_eviction_evict(session, address);
  return libflush_get_timing(session) - start;
}

void
libflush_prefetch(libflush_session_t* session, void* address)
{
  (void) session;

#if defined(__ARM_ARCH_7A__)
  arm_v7_prefetch(address);
  libflush_memory_barrier();
#elif defined(__ARM_ARCH_8A__)
  arm_v8_prefetch(address);
  libflush_memory_barrier();
#elif defined(__i386__) || defined(__x86_64__)
  x86_prefetch(address);
#else
#error No prefetch method available on this platform
#endif
}

uint64_t
libflush_prefetch_time(libflush_session_t* session, void* address)
{
  (void) session;

  uint64_t start = libflush_get_timing_start(session);

#if defined(__ARM_ARCH_7A__)
  arm_v7_prefetch(address);
  libflush_memory_barrier();
#elif defined(__ARM_ARCH_8A__)
  arm_v8_prefetch(address);
  libflush_memory_barrier();
#elif defined(__i386__) || defined(__x86_64__)
  x86_prefetch(address);
#else
#error No prefetch method available on this platform
#endif

  return libflush_get_timing_end(session) - start;
}

uint64_t
libflush_get_timing(libflush_session_t* session)
{
  (void) session;

  uint64_t result = 0;

  libflush_memory_barrier();

#if TIME_SOURCE == TIME_SOURCE_MONOTONIC_CLOCK
  result = get_monotonic_time();
#elif TIME_SOURCE == TIME_SOURCE_PERF
  result = perf_get_timing(session);
#elif TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
  result = thread_counter_get_timing(session);
#elif TIME_SOURCE == TIME_SOURCE_REGISTER
#if defined(__ARM_ARCH_7A__)
  result = arm_v7_get_timing();
#elif defined(__ARM_ARCH_8A__)
  result = arm_v8_get_timing();
#elif defined(__i386__) || defined(__x86_64__)
  result = x86_get_timing();
#endif
#endif

  libflush_memory_barrier();

  return result;
}

static uint64_t
libflush_get_timing_start(libflush_session_t* session)
{
  (void) session;

  uint64_t result = 0;

#if defined(__i386__) || defined(__x86_64__)
  result = x86_get_timing_start();
#else
  result = libflush_get_timing(session);
#endif

  return result;
}

static uint64_t
libflush_get_timing_end(libflush_session_t* session)
{
  (void) session;

  uint64_t result = 0;

#if defined(__i386__) || defined(__x86_64__)
  result = x86_get_timing_end();
#else
  result = libflush_get_timing(session);
#endif

  return result;
}

void
libflush_reset_timing(libflush_session_t* session)
{
  (void) session;

#if TIME_SOURCE == TIME_SOURCE_PERF
  perf_reset_timing(session);
#elif TIME_SOURCE == TIME_SOURCE_REGISTER
#if defined(__ARM_ARCH_7A__)
  arm_v7_reset_timing(session->performance_register_div64);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_reset_timing();
#endif
#endif

  libflush_memory_barrier();
}

inline void
libflush_access_memory(void *address) {
#if defined(__ARM_ARCH_7A__)
  arm_v7_access_memory(address);
#elif defined(__ARM_ARCH_8A__)
  arm_v8_access_memory(address);
#elif defined(__i386__) || defined(__x86_64__)
  x86_access_memory(address);
#endif
}

uint64_t
libflush_reload_address(libflush_session_t* session, void* address)
{
  uint64_t time = libflush_get_timing(session);
  libflush_access_memory(address);

  return libflush_get_timing(session) - time;
}

uint64_t
libflush_reload_address_and_flush(libflush_session_t* session, void* address)
{
  uint64_t time = libflush_get_timing_start(session);
  libflush_access_memory(address);
  uint64_t delta =  libflush_get_timing_end(session) - time;
  libflush_flush(session, address);

  return delta;
}

uint64_t
libflush_reload_address_and_evict(libflush_session_t* session, void* address)
{
  uint64_t time = libflush_get_timing_start(session);
  libflush_access_memory(address);
  uint64_t delta =  libflush_get_timing_end(session) - time;
  libflush_evict(session, address);

  return delta;
}

inline void
libflush_memory_barrier()
{
#if defined(__ARM_ARCH_7A__)
  arm_v7_memory_barrier();
#elif defined(__ARM_ARCH_8A__)
  arm_v8_memory_barrier();
#elif defined(__i386__) || defined(__x86_64__)
  x86_memory_barrier();
#endif
}

void
libflush_prime(libflush_session_t* session, size_t set_index)
{
  libflush_eviction_prime(session, set_index);
}

size_t
libflush_get_set_index(libflush_session_t* session, void* address)
{
  return libflush_eviction_get_set_index(session, address);
}

size_t
libflush_get_number_of_sets(libflush_session_t* session)
{
  return libflush_eviction_get_number_of_sets(session);
}

uint64_t
libflush_probe(libflush_session_t* session, size_t set_index)
{
  uint64_t time = libflush_get_timing_start(session);
  libflush_eviction_probe(session, set_index);
  uint64_t delta =  libflush_get_timing_end(session) - time;

  return delta;
}

uintptr_t
libflush_get_physical_address(libflush_session_t* session, uintptr_t virtual_address)
{
  (void) session;
  (void) virtual_address;

#if HAVE_PAGEMAP_ACCESS == 1
  // Access memory
  libflush_access_memory((void *) virtual_address);

  uint64_t value;
  off_t offset = (virtual_address / 4096) * sizeof(value);
  int got = pread(session->memory.pagemap, &value, sizeof(value), offset);
  assert(got == 8);

  // Check the "page present" flag.
  assert(value & (1ULL << 63));

  uint64_t frame_num = get_frame_number_from_pagemap(value);
  return (frame_num * 4096) | (virtual_address & (4095));
#else
  return 0;
#endif
}

uint64_t
libflush_get_pagemap_entry(libflush_session_t* session, uint64_t virtual_address)
{
  (void) session;
  (void) virtual_address;

#if HAVE_PAGEMAP_ACCESS == 1
  // Access memory
  uint64_t value;
  off_t offset = (virtual_address / 4096) * sizeof(value);
  int got = pread(session->memory.pagemap, &value, sizeof(value), offset);
  assert(got == 8);

  return value;
#else
  return 0;
#endif
}

#if HAVE_PAGEMAP_ACCESS == 1
static size_t
get_frame_number_from_pagemap(size_t value)
{
  return value & ((1ULL << 55) - 1);
}
#endif
