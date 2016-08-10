/* See LICENSE file for license and copyright information */

#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

#include "libflush.h"

#define TIME_SOURCE_REGISTER 1
#define TIME_SOURCE_PERF 2
#define TIME_SOURCE_MONOTONIC_CLOCK 3
#define TIME_SOURCE_THREAD_COUNTER  4

#if TIME_SOURCE == TIME_SOURCE_MONOTONIC_CLOCK
uint64_t get_monotonic_time(void);
#endif

#if TIME_SOURCE == TIME_SOURCE_PERF
bool perf_init(libflush_session_t* session, libflush_session_args_t* args);
bool perf_terminate(libflush_session_t* session);
uint64_t perf_get_timing(libflush_session_t* session);
uint64_t perf_get_timing_start(libflush_session_t* session);
uint64_t perf_get_timing_end(libflush_session_t* session);
void perf_reset_timing(libflush_session_t* session);
#endif

#if TIME_SOURCE == TIME_SOURCE_THREAD_COUNTER
bool thread_counter_init(libflush_session_t* session, libflush_session_args_t* args);
uint64_t thread_counter_get_timing(libflush_session_t* session);
bool thread_counter_terminate(libflush_session_t* session);
#endif

#endif // TIMING_H
