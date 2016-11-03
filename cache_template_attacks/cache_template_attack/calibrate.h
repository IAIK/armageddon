 /* See LICENSE file for license and copyright information */

#ifndef CALIBRATE_H
#define CALIBRATE_H

#include <libflush/libflush.h>

#define CALIBRATION_HISTOGRAM_SIZE 200
#define CALIBRATION_HISTOGRAM_ENTRIES 100000
#define CALIBRATION_HISTOGRAM_SCALE 5
#define CALIBRATION_HISTOGRAM_THRESHOLD 100

uint64_t calibrate(libflush_session_t* libflush_session);

#endif  /*CALIBRATE_H*/
