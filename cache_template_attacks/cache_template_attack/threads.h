 /* See LICENSE file for license and copyright information */

#ifndef THREADS_H
#define THREADS_H

#include <libflush/libflush.h>

typedef enum thread_type_e {
  THREAD_FLUSH_AND_RELOAD,
  THREAD_FLUSH
} thread_type_t;

typedef struct thread_data_s {
  thread_type_t type;
  uint8_t* m;
  size_t range;
  uint64_t threshold;
  size_t offset;
  size_t cpu_id;
  bool spy;
  size_t number_of_tests;
  useconds_t offset_update_time;
  bool show_timing;
  FILE* logfile;
  libflush_session_t* libflush_session;
} thread_data_t;

#endif  /*THREADS_H*/
