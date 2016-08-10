/* See LICENSE file for license and copyright information */

#if ANDROID_PLATFORM >= 21
#define _GNU_SOURCE
#include <sched.h>
#endif

#include "libflush.h"

#if ANDROID_PLATFORM < 21
#include <sys/syscall.h>
#include <string.h>

#define CPU_SETSIZE 1024
#define __NCPUBITS  (8 * sizeof (unsigned long))

typedef struct cpu_set_s {
  unsigned long __bits[CPU_SETSIZE / __NCPUBITS];
} cpu_set_t;

#define CPU_SET(cpu, cpusetp) \
    ((cpusetp)->__bits[(cpu)/__NCPUBITS] |= (1UL << ((cpu) % __NCPUBITS)))

#define CPU_ZERO(cpusetp) \
    memset((cpusetp), 0, sizeof(cpu_set_t))
#else
#endif

bool
libflush_bind_to_cpu(size_t cpu)
{
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);

#if ANDROID_PLATFORM < 21
  if (syscall(__NR_sched_setaffinity, 0, sizeof(mask), &mask) == -1) {
#else
  if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
#endif
    return false;
  } else {
    return true;
  }
}
