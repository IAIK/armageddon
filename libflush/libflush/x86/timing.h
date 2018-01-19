/* See LICENSE file for license and copyright information */

#ifndef X86_TIMING_H
#define X86_TIMING_H

#include <stdint.h>

#include "memory.h"

extern inline uint64_t
x86_get_timing(void)
{
    uint32_t lo = 0;
    uint32_t hi = 0;

  asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));

    return  (((int64_t)hi) << 32) | lo;
}

extern inline uint64_t
x86_get_timing_start(void)
{
    uint32_t lo = 0;
    uint32_t hi = 0;

    asm volatile ("mfence\n\t"
    "RDTSCP\n\t"
    "mov %%edx, %0\n\t"
    "mov %%eax, %1\n\t"
    "xor %%eax, %%eax\n\t"
    "CPUID\n\t"
    : "=r" (hi), "=r" (lo)
    :
    : "%eax", "%ebx", "%ecx", "%edx");

  return  (((int64_t)hi) << 32) | lo;
}

extern inline  uint64_t
x86_get_timing_end(void)
{
  uint32_t lo = 0;
  uint32_t hi = 0;

  asm volatile(
    "xor %%eax, %%eax\n\t"
    "CPUID\n\t"
    "RDTSCP\n\t"
    "mov %%edx, %0\n\t"
    "mov %%eax, %1\n\t"
    "mfence\n\t"
  : "=r" (hi), "=r" (lo)
  :
  : "%eax", "%ebx", "%ecx", "%edx");

    return  (((int64_t)hi) << 32) | lo;
}

#endif  /*X86_TIMING_H*/
