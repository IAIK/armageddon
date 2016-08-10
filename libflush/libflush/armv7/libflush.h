/* See LICENSE file for license and copyright information */

#ifndef ARM_V7_LIBFLUSH_H
#define ARM_V7_LIBFLUSH_H

#include "../libflush.h"

#include "timing.h"
#include "memory.h"

void arm_v7_init(libflush_session_t* session, libflush_session_args_t* args);
void arm_v7_terminate(libflush_session_t* session);

#endif /* ARM_V7_LIBFLUSH_H */
