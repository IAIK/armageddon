 /* See LICENSE file for license and copyright information */

#include <inttypes.h>
#include <sched.h>
#include <libflush/libflush.h>

#include "calibrate.h"

#define MIN(a, b) ((a) > (b)) ? (b) : (a)

uint64_t calibrate(libflush_session_t* libflush_session)
{
  char buffer[4096] = {0};
  void* address = &buffer[1024];

  // Measure time it takes to access something from the cache
  size_t hit_histogram[CALIBRATION_HISTOGRAM_SIZE] = {0};
  libflush_access_memory(address);

  for (unsigned int i = 0; i < CALIBRATION_HISTOGRAM_ENTRIES; i++) {
      uint64_t time = libflush_reload_address(libflush_session, address);
      hit_histogram[MIN(CALIBRATION_HISTOGRAM_SIZE - 1, time / CALIBRATION_HISTOGRAM_SCALE)]++;
      sched_yield();
  }

  // Measure time it takes to access something from memory
  size_t miss_histogram[CALIBRATION_HISTOGRAM_SIZE] = {0};
  for (unsigned int i = 0; i < CALIBRATION_HISTOGRAM_ENTRIES; i++) {
      uint64_t time = libflush_reload_address_and_flush(libflush_session, address);
      miss_histogram[MIN(CALIBRATION_HISTOGRAM_SIZE - 1, time / CALIBRATION_HISTOGRAM_SCALE)]++;
      sched_yield();
  }

  // Get the maximum value of a cache hit and the minimum value of a cache miss
  size_t hit_maximum_index = 0;
  size_t hit_maximum = 0;

  size_t miss_minimum_index = 0;
  size_t miss_maximum = 0;
  size_t miss_maximum_index = 0;

  for (int i = 0; i < CALIBRATION_HISTOGRAM_SIZE; i++) {
      if (hit_maximum < hit_histogram[i]) {
          hit_maximum = hit_histogram[i];
          hit_maximum_index = i;
      }

      if (miss_maximum < miss_histogram[i]) {
          miss_maximum = miss_histogram[i];
          miss_maximum_index = i;
      }

      if (miss_histogram[i] > CALIBRATION_HISTOGRAM_THRESHOLD && miss_minimum_index == 0) {
          miss_minimum_index = i;
      }
  }

  uint64_t cache = hit_maximum_index * CALIBRATION_HISTOGRAM_SCALE;
  uint64_t mem = miss_maximum_index * CALIBRATION_HISTOGRAM_SCALE;
  uint64_t threshold = mem - (mem - cache) / 2;

  return threshold;
}
