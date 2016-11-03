 /* See LICENSE file for license and copyright information */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdlib.h>
#include <stdbool.h>

#include "key-mapping.h"

typedef struct simulator_s simulator_t;

bool simulator_init(simulator_t** simulator);
bool simulator_terminate(simulator_t* simulator);

bool simulator_send_tap(simulator_t* simulator, int x, int y);

bool simulator_send_char(simulator_t* simulator, const char* key);

bool simulator_get_coordinates(simulator_t* simulator, const char* input, int* x, int* y);

#endif  /*SIMULATOR_H*/
