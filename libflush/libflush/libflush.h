/* See LICENSE file for license and copyright information */

#ifndef LIBFLUSH_H
#define LIBFLUSH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * libflush session
 */
typedef struct libflush_session_args_s {
  size_t bind_to_cpu; /**< CPU id to bind dedicated thread timer */
  bool performance_register_div64; /**< Enable 64 divisor (ARM only) */
} libflush_session_args_t;

typedef struct libflush_session_s libflush_session_t;

/**
 * Initializes the libflush session
 *
 * @param[out] session The initialized session
 * @param[in] args Additional arguments for the initialization
 *
 * @return true Initialization was successful
 * @return false Initialization failed
 */
bool libflush_init(libflush_session_t** session, libflush_session_args_t* args);

/**
 * Terminates the libflush session
 *
 * @param[in] session The initialized session
 *
 * @return true Termination was successful
 * @return false Termination failed
 */
bool libflush_terminate(libflush_session_t* session);

/**
 * Get current time measurement
 *
 * @param[in] session The used session
 *
 * @return Current time measurements
 */
uint64_t libflush_get_timing(libflush_session_t* session);

/**
 * Resets the time measurement
 *
 * @param[in] session The used session
 */
void libflush_reset_timing(libflush_session_t* session);

/**
 * Flushes the given address
 *
 * @param[in] session The used session
 * @param[in] address The address to flush
 */
void libflush_flush(libflush_session_t* session, void* address);

/**
 * Measure how long it takes to flush the given address
 *
 * @param[in] session The used session
 * @param[in] address The address to flush
 *
 * @return Timing measurement
 */
uint64_t libflush_flush_time(libflush_session_t* session, void* address);

/**
 * Evicts the given address
 *
 * @param[in] session The used session
 * @param[in] address The address to flush
 */
void libflush_evict(libflush_session_t* session, void* address);

/**
 * Measure how long it takes to evict the given address
 *
 * @param[in] session The used session
 * @param[in] address The address to flush
 *
 * @return Timing measurement
 */
uint64_t libflush_evict_time(libflush_session_t* session, void* address);

/**
 * Accesses the given data pointer
 *
 * @param[in] address Address to access
 */
void libflush_access_memory(void *address);

/**
 * Measures the time it takes to access the given address
 *
 * @param[in] session The used session
 * @param[in] address Address to access
 *
 * @return Timing measurement
 */
uint64_t libflush_reload_address(libflush_session_t* session, void* address);

/**
 * Measures the time it takes to access the given address. Then the address is
 * flushed to memory.
 *
 * @param[in] session The used session
 * @param[in] address Address to access
 *
 * @return Timing measurement
 */
uint64_t libflush_reload_address_and_flush(libflush_session_t* session, void* address);

/**
 * Measures the time it takes to access the given address. Then the address is
 * evicted to memory.
 *
 * @param[in] session The used session
 * @param[in] address Address to access
 *
 * @return Timing measurement
 */
uint64_t libflush_reload_address_and_evict(libflush_session_t* session, void* address);

/**
 * Memory barrier
 */
void libflush_memory_barrier();

/**
 * Primes a given cache set.
 *
 * @param[in] session The used session
 * @param[in] set_index The set index
 */
void libflush_prime(libflush_session_t* session, size_t set_index);

/**
 * Probes a given cache set.
 *
 * @param[in] session The used session
 * @param[in] set_index The set index
 *
 * @return Timing measurement
 */
uint64_t libflush_probe(libflush_session_t* session, size_t set_index);

/**
 * Returns the set index of a given address
 *
 * @param[in] session The used session
 * @param[in] address The target address
 *
 * @return The set index
 */
size_t libflush_get_set_index(libflush_session_t* session, void* address);

/**
 * Returns the number of sets
 *
 * @param[in] session The used session
 *
 * @return The number of sets
 */
size_t libflush_get_number_of_sets(libflush_session_t* session);

/**
 * Prefetches an address.
 *
 * @param[in] session The used session.
 * @param[in] address The target address.
 */
void libflush_prefetch(libflush_session_t* session, void* address);

/**
 * Measures the time it takes to prefetch a given address.
 *
 * @param[in] session The used session.
 * @param[in] address The target address.
 *
 * @return Timing measurement
 */
uint64_t libflush_prefetch_time(libflush_session_t* session, void* address);

/**
 * Returns the physical address of an virtual address.
 *
 * @param[in] session The used session
 * @param[in] virtual_address The virtual address
 *
 * @return The physical address
 */
uintptr_t libflush_get_physical_address(libflush_session_t* session, uintptr_t virtual_address);

/**
 * Returns the raw pagemap entry of an virtual address.
 *
 * @param[in] session The used session
 * @param[in] virtual_address The virtual address
 *
 * @return The raw pagemap entry
 */
uint64_t libflush_get_pagemap_entry(libflush_session_t* session, uint64_t virtual_address);

/**
 * Binds the process to a cpu
 *
 * @param[in] cpu The cpu id
 *
 * @return true Binding to the cpu was successful
 * @return false Binding to the cpu failed
 */
bool libflush_bind_to_cpu(size_t cpu);

#ifdef __cplusplus
}
#endif

#endif /* LIBFLUSH_H */
