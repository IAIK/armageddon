/* See LICENSE file for license and copyright information */

#ifndef X86_MEMORY_H
#define X86_MEMORY_H

extern inline void
x86_access_memory(void* pointer)
{
  asm volatile ("movq (%0), %%rax\n"
      :
      : "c" (pointer)
      : "rax");
}

extern inline void
x86_memory_barrier(void)
{
  asm volatile ("mfence");
}

extern inline void
x86_prefetch(void* pointer)
{
  asm volatile ("prefetchnta (%0)" :: "r" (pointer));
  asm volatile ("prefetcht2 (%0)" :: "r" (pointer));
}

#endif  /*X86_MEMORY_H*/
