/* See LICENSE file for license and copyright information */

#ifndef ARM_V8_LIBFLUSH_H
#define ARM_V8_LIBFLUSH_H

#include "../libflush.h"

#include "flush.h"
#include "timing.h"
#include "memory.h"

static inline void
arm_v8_init(libflush_session_t* session, libflush_session_args_t* args)
{
  (void) session;
  (void) args;

  // Enable user space performance counter
#if TIME_SOURCE == TIME_SOURCE_REGISTER
  arm_v8_timing_init();
#endif
}

static inline void
arm_v8_terminate(libflush_session_t* session)
{
  (void) session;

  // Disable user space performance counter
#if TIME_SOURCE == TIME_SOURCE_REGISTER
  arm_v8_timing_terminate();
#endif
}

#endif /* ARM_V8_LIBFLUSH_H */
