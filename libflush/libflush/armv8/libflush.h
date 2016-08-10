/* See LICENSE file for license and copyright information */

#ifndef ARM_V8_LIBFLUSH_H
#define ARM_V8_LIBFLUSH_H

#include "../libflush.h"

#include "flush.h"
#include "timing.h"
#include "memory.h"

void arm_v8_init(libflush_session_t* session, libflush_session_args_t* args);
void arm_v8_terminate(libflush_session_t* session);

#endif /* ARM_V8_LIBFLUSH_H */
