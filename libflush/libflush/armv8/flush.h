/* See LICENSE file for license and copyright information */

#ifndef ARM_V8_FLUSH_H
#define ARM_V8_FLUSH_H

inline void arm_v8_flush(void* address)
{
  asm volatile ("DC CIVAC, %0" :: "r"(address));
  asm volatile ("DSB ISH");
  asm volatile ("ISB");
}

#endif /* ARM_V8_FLUSH_H */
