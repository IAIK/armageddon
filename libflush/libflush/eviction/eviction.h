/* See LICENSE file for license and copyright information */

#ifndef LIBFLUSH_EVICTION_H
#define LIBFLUSH_EVICTION_H

#include "../libflush.h"

bool libflush_eviction_init(libflush_session_t* session, libflush_session_args_t* args);
bool libflush_eviction_terminate(libflush_session_t* session);
void libflush_eviction_evict(libflush_session_t* session, void* address);

void libflush_eviction_prime(libflush_session_t* session, size_t set_index);
void libflush_eviction_probe(libflush_session_t* session, size_t set_index);

size_t libflush_eviction_get_set_index(libflush_session_t* session, void* address);
size_t libflush_eviction_get_number_of_sets(libflush_session_t* session);

#endif // LIBFLUSH_EVICTION_H
