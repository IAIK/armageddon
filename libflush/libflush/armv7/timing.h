/* See LICENSE file for license and copyright information */

#ifndef ARM_V7_TIMING_H
#define ARM_V7_TIMING_H

#include <stdint.h>
#include "memory.h"
#include "timing.h"
#include "configuration.h"

#define ARMV7_PMCR_E       (1 << 0) /* Enable all counters */
#define ARMV7_PMCR_P       (1 << 1) /* Reset all counters */
#define ARMV7_PMCR_C       (1 << 2) /* Cycle counter reset */
#define ARMV7_PMCR_D       (1 << 3) /* Cycle counts every 64th cpu cycle */
#define ARMV7_PMCR_X       (1 << 4) /* Export to ETM */

#define ARMV7_PMCNTENSET_C (1 << 31) /* Enable cycle counter */

#define ARMV7_PMOVSR_C     (1 << 31) /* Overflow bit */

inline uint64_t
arm_v7_get_timing(void)
{
  uint32_t result = 0;

  asm volatile ("MRC p15, 0, %0, c9, c13, 0" : "=r" (result));

  return result;
}

inline void
arm_v7_reset_timing(bool div64)
{
  uint32_t value = 0;

  value |= ARMV7_PMCR_E; // Enable all counters
  value |= ARMV7_PMCR_P; // Reset all counters
  value |= ARMV7_PMCR_C; // Reset cycle counter to zero
  value |= ARMV7_PMCR_X; // Enable export of events

  if (div64 == true) {
    value |= ARMV7_PMCR_D; // Enable cycle count divider
  }

  // Performance Monitor Control Register
  asm volatile ("MCR p15, 0, %0, c9, c12, 0" :: "r" (value));
}

inline void
arm_v7_timing_init(bool div64)
{
  uint32_t value = 0;

  value |= ARMV7_PMCR_E; // Enable all counters
  value |= ARMV7_PMCR_P; // Reset all counters
  value |= ARMV7_PMCR_C; // Reset cycle counter to zero
  value |= ARMV7_PMCR_X; // Enable export of events

  if (div64 == true) {
    value |= ARMV7_PMCR_D; // Enable cycle count divider
  }

  // Performance Monitor Control Register
  asm volatile ("MCR p15, 0, %0, c9, c12, 0" :: "r" (value));

  // Count Enable Set Register
  value = 0;
  value |= ARMV7_PMCNTENSET_C;

  for (unsigned int x = 0; x < 4; x++) {
    value |= (1 << x); // Enable the PMx event counter
  }

  asm volatile ("MCR p15, 0, %0, c9, c12, 1" :: "r" (value));

  // Overflow Flag Status register
  value = 0;
  value |= ARMV7_PMOVSR_C;

  for (unsigned int x = 0; x < 4; x++) {
    value |= (1 << x); // Enable the PMx event counter
  }
  asm volatile ("MCR p15, 0, %0, c9, c12, 3" :: "r" (value));
}

inline void
arm_v7_timing_terminate(void)
{
  uint32_t value = 0;
  uint32_t mask = 0;

  // Performance Monitor Control Register
  asm volatile ("MRC p15, 0, %0, c9, c12, 0" :: "r" (value));

  mask = 0;
  mask |= ARMV7_PMCR_E; /* Enable */
  mask |= ARMV7_PMCR_C; /* Cycle counter reset */
  mask |= ARMV7_PMCR_P; /* Reset all counters */
  mask |= ARMV7_PMCR_X; /* Export */

  asm volatile ("MCR p15, 0, %0, c9, c12, 0" :: "r" (value & ~mask));
}

#endif  /*ARM_V7_TIMING_H*/
