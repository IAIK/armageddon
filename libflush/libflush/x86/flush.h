/* See LICENSE file for license and copyright information */

#ifndef X86_FLUSH_H
#define X86_FLUSH_H

extern inline void x86_flush(void* address)
{
  asm volatile ("clflush 0(%0)"
    :
    : "r" (address)
    : "rax"
  );
}

#endif /* X86_FLUSH_H */
