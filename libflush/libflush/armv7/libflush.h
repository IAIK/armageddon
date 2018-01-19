/* See LICENSE file for license and copyright information */

#ifndef ARM_V7_LIBFLUSH_H
#define ARM_V7_LIBFLUSH_H

#include "../libflush.h"

#include "timing.h"
#include "memory.h"
#include "configuration.h"

static inline void
arm_v7_init(libflush_session_t* session, libflush_session_args_t* args)
{
  (void) session;
  (void) args;

#if TIME_SOURCE == TIME_SOURCE_REGISTER
  bool div64 = CYCLE_COUNTER_DIV_64;
  if (args != NULL && args->performance_register_div64 == true) {
    div64 = true;
  }

  // Enable user space performance counter
  arm_v7_timing_init(div64);
#endif
}

static inline void
arm_v7_terminate(libflush_session_t* session)
{
  (void) session;

  // Disable user space performance counter
#if TIME_SOURCE == TIME_SOURCE_REGISTER
  arm_v7_timing_terminate();
#endif
}

#endif /* ARM_V7_LIBFLUSH_H */
