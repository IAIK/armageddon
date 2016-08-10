/* See LICENSE file for license and copyright information */

#ifndef X86_TIMING_H
#define X86_TIMING_H

#include <stdint.h>

#include "memory.h"

extern inline uint64_t
x86_get_timing(void)
{
  uint64_t result = 0;
  uint64_t d = 0;

  asm volatile ("rdtsc" : "=a" (result), "=d" (d));
  result = (d << 32) | result;

  return result;
}

extern inline uint64_t
x86_get_timing_start(void)
{
  uint64_t result = 0;
  uint64_t d = 0;

    asm volatile ("mfence\n\t"
    "RDTSCP\n\t"
    "mov %%rdx, %0\n\t"
    "mov %%rax, %1\n\t"
    "xor %%rax, %%rax\n\t"
    "CPUID\n\t"
    : "=r" (d), "=r" (result)
    :
    : "%rax", "%rbx", "%rcx", "%rdx");

  result = (d << 32) | result;

  return result;
}

extern inline  uint64_t
x86_get_timing_end(void)
{
  uint64_t result = 0;
  uint64_t d = 0;

  asm volatile(
    "xor %%rax, %%rax\n\t"
    "CPUID\n\t"
    "RDTSCP\n\t"
    "mov %%rdx, %0\n\t"
    "mov %%rax, %1\n\t"
    "mfence\n\t"
  : "=r" (d), "=r" (result)
  :
  : "%rax", "%rbx", "%rcx", "%rdx");

  result = (d << 32) | result;

  return result;
}

#endif  /*X86_TIMING_H*/
