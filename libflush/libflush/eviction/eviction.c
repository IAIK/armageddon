/* See LICENSE file for license and copyright information */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sched.h>
#include <sys/sysinfo.h>

#ifdef PTHREAD_ENABLE
#include <pthread.h>
#endif

#include "../libflush.h"
#include "../internal.h"
#include "eviction.h"

#define __include_strategy(x) #x
#define _include_strategy(x) __include_strategy(x)
#define include_strategy(x) _include_strategy(x)

#include include_strategy(DEVICE_CONFIGURATION)

#include "configuration.h"

#define ADDRESS_COUNT ((ES_EVICTION_COUNTER) + (ES_DIFFERENT_ADDRESSES_IN_LOOP) - 1)

typedef struct virtual_address_cache_entry_s {
#ifdef PTHREAD_ENABLE
  pthread_mutex_t lock;
#endif
  bool used;
  void* virtual_address;
  size_t index;
} virtual_address_cache_entry_t;

typedef struct congruent_address_cache_entry_s {
#ifdef PTHREAD_ENABLE
  pthread_mutex_t lock;
#endif
  bool used;
  void* congruent_virtual_addresses[ADDRESS_COUNT];
} congruent_address_cache_entry_t;

typedef struct memory_s {
#ifdef PTHREAD_ENABLE
  pthread_mutex_t lock;
#endif
  size_t mapping_size;
  void* mapping;
  int pagemap;
} memory_t;

typedef struct libflush_eviction_s {
  congruent_address_cache_entry_t congruent_address_cache[NUMBER_OF_SETS];
  virtual_address_cache_entry_t virtual_address_cache[ADDRESS_CACHE_SIZE];
#ifdef PTHREAD_ENABLE
    pthread_mutex_t virtual_address_cache_lock;
#endif
  memory_t memory;
} libflush_eviction_t;

#if USE_FIXED_MEMORY_SIZE == 0
static size_t get_physical_memory_size(void);
#endif

void evict(congruent_address_cache_entry_t* address_cache_entry);
void find_congruent_addresses(libflush_session_t* session, libflush_eviction_t*
    eviction, size_t index, uintptr_t physical_address);

void
libflush_eviction_evict(libflush_session_t* session, void* address)
{
  libflush_eviction_t* eviction = (libflush_eviction_t*) session->data;

#ifdef PTHREAD_ENABLE
  pthread_mutex_lock(&(eviction->virtual_address_cache_lock));
#endif
  // Check if address is cached and run eviction
  for (unsigned int i = 0; i < ADDRESS_CACHE_SIZE; i++) {
    if (eviction->virtual_address_cache[i].virtual_address == address) {
      evict(&(eviction->congruent_address_cache[eviction->virtual_address_cache[i].index]));
#ifdef PTHREAD_ENABLE
  pthread_mutex_unlock(&(eviction->virtual_address_cache_lock));
#endif
      return;
    }
  }

  // Find free cache entry
  virtual_address_cache_entry_t* virtual_address_cache_entry = NULL;
  for (unsigned int i = 0; i < ADDRESS_CACHE_SIZE; i++) {
    if (eviction->virtual_address_cache[i].used == false) {
      virtual_address_cache_entry = &(eviction->virtual_address_cache[i]);
      break;
    }
  }

  // If there is no free cache entry, we chose a random one
  if (virtual_address_cache_entry == NULL) {
#if defined(__ARM_ARCH_7A__)
    virtual_address_cache_entry = &(eviction->virtual_address_cache[lrand48() % ADDRESS_CACHE_SIZE]);
#else
    virtual_address_cache_entry = &(eviction->virtual_address_cache[rand() % ADDRESS_CACHE_SIZE]);
#endif
  }

  uintptr_t physical_address = libflush_get_physical_address(session, (size_t) address);
  uint64_t index = (physical_address >> LINE_LENGTH_LOG2) % NUMBER_OF_SETS;

  // Set cache entry information
  virtual_address_cache_entry->virtual_address  = address;
  virtual_address_cache_entry->used = true;
  virtual_address_cache_entry->index = index;

#ifdef PTHREAD_ENABLE
    pthread_mutex_lock(&(eviction->congruent_address_cache[index].lock));
#endif

  if (eviction->congruent_address_cache[index].used == false) {
    find_congruent_addresses(session, eviction, index, physical_address);
  }

#ifdef PTHREAD_ENABLE
    pthread_mutex_unlock(&(eviction->congruent_address_cache[index].lock));
#endif

#ifdef PTHREAD_ENABLE
  pthread_mutex_unlock(&(eviction->virtual_address_cache_lock));
#endif

  // Run eviction
  evict(&(eviction->congruent_address_cache[virtual_address_cache_entry->index]));
}

bool
libflush_eviction_init(libflush_session_t* session, libflush_session_args_t* args)
{
  if (session == NULL) {
    return false;
  }

  (void) args;

  libflush_eviction_t* eviction = calloc(1, sizeof(libflush_eviction_t));
  if (eviction == NULL) {
    return false;
  }

  session->data = eviction;

#ifdef PTHREAD_ENABLE
  assert(pthread_mutex_init(&(eviction->memory.lock), NULL) == 0);
  assert(pthread_mutex_init(&(eviction->virtual_address_cache_lock), NULL) == 0);
  pthread_mutex_lock(&(eviction->memory.lock));
#endif

  // Calculate mapping size
#if USE_FIXED_MEMORY_SIZE == 1
  eviction->memory.mapping_size = PHYSICAL_MEMORY_MAPPED_SIZE;
#else
  eviction->memory.mapping_size = get_physical_memory_size() * FRACTION_OF_PHYSICAL_MEMORY;
#endif

  // Map memory
  eviction->memory.mapping = mmap(NULL, eviction->memory.mapping_size, PROT_READ | PROT_WRITE,
                       MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  assert(eviction->memory.mapping != (void *) -1);

  // Initialize the mapping so that the pages are non-empty.
  for (uint64_t index = 0; index < eviction->memory.mapping_size; index += 0x400) {
      uint64_t* temporary = (uint64_t*) (((uint8_t*) eviction->memory.mapping) + index);
      temporary[0] = index;
  }

  // Clean address cache
  memset(eviction->virtual_address_cache, 0, ADDRESS_CACHE_SIZE * sizeof(virtual_address_cache_entry_t));
  memset(eviction->congruent_address_cache, 0, NUMBER_OF_SETS * sizeof(congruent_address_cache_entry_t));

#ifdef PTHREAD_ENABLE
  for (unsigned int i = 0; i < ADDRESS_CACHE_SIZE; i++) {
    assert(pthread_mutex_init(&(eviction->virtual_address_cache[i].lock), NULL) == 0);
  }

  for (unsigned int i = 0; i < ADDRESS_CACHE_SIZE; i++) {
    assert(pthread_mutex_init(&(eviction->congruent_address_cache[i].lock), NULL) == 0);
  }

  pthread_mutex_unlock(&(eviction->memory.lock));
#endif

  return true;
}

bool
libflush_eviction_terminate(libflush_session_t* session)
{
  if (session == NULL) {
    return false;
  }

  libflush_eviction_t* eviction = (libflush_eviction_t*) session->data;

  if (eviction == NULL) {
    return true;
  }

#ifdef PTHREAD_ENABLE
  pthread_mutex_lock(&(eviction->memory.lock));
#endif

  if (eviction->memory.mapping != NULL) {
    munmap(eviction->memory.mapping, eviction->memory.mapping_size);
  }

  eviction->memory.mapping_size = 0;
  eviction->memory.mapping = NULL;

#ifdef PTHREAD_ENABLE
  for (unsigned int i = 0; i < ADDRESS_CACHE_SIZE; i++) {
    pthread_mutex_destroy(&(eviction->virtual_address_cache[i].lock));
  }

  for (unsigned int i = 0; i < ADDRESS_CACHE_SIZE; i++) {
    pthread_mutex_destroy(&(eviction->congruent_address_cache[i].lock));
  }

  pthread_mutex_unlock(&(eviction->memory.lock));
  pthread_mutex_destroy(&(eviction->memory.lock));
#endif

  /* Free eviction data */
  free(eviction);
  session->data = NULL;

  return true;
}

void
libflush_eviction_prime(libflush_session_t* session, size_t set_index)
{
  libflush_eviction_t* eviction = (libflush_eviction_t*) session->data;

#ifdef PTHREAD_ENABLE
  pthread_mutex_lock(&(eviction->virtual_address_cache_lock));
  pthread_mutex_lock(&(eviction->congruent_address_cache[set_index].lock));
#endif

  if (eviction->congruent_address_cache[set_index].used == false) {
    find_congruent_addresses(session, eviction, set_index, (uintptr_t) NULL);
  }

#ifdef PTHREAD_ENABLE
  pthread_mutex_unlock(&(eviction->congruent_address_cache[set_index].lock));
  pthread_mutex_unlock(&(eviction->virtual_address_cache_lock));
#endif

  evict(&(eviction->congruent_address_cache[set_index]));
}

void libflush_eviction_probe(libflush_session_t* session, size_t set_index)
{
  libflush_eviction_t* eviction = (libflush_eviction_t*) session->data;

#ifdef PTHREAD_ENABLE
  pthread_mutex_lock(&(eviction->congruent_address_cache[set_index].lock));
#endif

  congruent_address_cache_entry_t congruent_address_cache_entry =
    eviction->congruent_address_cache[set_index];

  if (congruent_address_cache_entry.used == false) {
    find_congruent_addresses(session, eviction, set_index, (uintptr_t) NULL);
  }

  for (int i = ADDRESS_COUNT - 1; i >= 0; i -= 1) {
    libflush_access_memory(congruent_address_cache_entry.congruent_virtual_addresses[i]);
  }

#ifdef PTHREAD_ENABLE
  pthread_mutex_unlock(&(eviction->congruent_address_cache[set_index].lock));
#endif
}

size_t
libflush_eviction_get_number_of_sets(libflush_session_t* session)
{
  (void) session;

  return NUMBER_OF_SETS;
}

size_t
libflush_eviction_get_set_index(libflush_session_t* session, void* address)
{
  uintptr_t physical_address = libflush_get_physical_address(session, (uintptr_t) address);
  return (physical_address >> LINE_LENGTH_LOG2) % NUMBER_OF_SETS;
}

#if USE_FIXED_MEMORY_SIZE == 0
static size_t
get_physical_memory_size(void)
{
  struct sysinfo info;
  sysinfo(&info);

  return info.totalram * info.mem_unit;
}
#endif

inline void
evict(congruent_address_cache_entry_t* congruent_address_cache_entry)
{
#ifdef PTHREAD_ENABLE
  pthread_mutex_lock(&(congruent_address_cache_entry->lock));
  if (congruent_address_cache_entry->used == false) {
    pthread_mutex_unlock(&(congruent_address_cache_entry->lock));
    return;
  }
#endif
  for (unsigned int i = 0; i < ES_EVICTION_COUNTER; i += 1) {
    for (unsigned int j = 0; j < ES_NUMBER_OF_ACCESSES_IN_LOOP; j++) {
      for (unsigned int k = 0; k < ES_DIFFERENT_ADDRESSES_IN_LOOP; k++) {
        libflush_access_memory(congruent_address_cache_entry->congruent_virtual_addresses[i+k]);
      }
    }
  }
#ifdef PTHREAD_ENABLE
  pthread_mutex_unlock(&(congruent_address_cache_entry->lock));
#endif
}

inline void
find_congruent_addresses(libflush_session_t* session, libflush_eviction_t*
    eviction, size_t index, uintptr_t physical_address)
{
  // Find congruent addresses
  unsigned int found = 0;
  for (unsigned int i = 0; i < eviction->memory.mapping_size; i += LINE_LENGTH) {
#ifdef PTHREAD_ENABLE
    pthread_mutex_lock(&(eviction->memory.lock));
#endif
    uint8_t* virtual_address_2 = (uint8_t*) eviction->memory.mapping + i;
#ifdef PTHREAD_ENABLE
    pthread_mutex_unlock(&(eviction->memory.lock));
#endif
    uintptr_t physical_address_2 = libflush_get_physical_address(session, (uintptr_t) virtual_address_2);
    uint64_t index_2 = (physical_address_2 >> LINE_LENGTH_LOG2) % NUMBER_OF_SETS;

    if (index == index_2 && physical_address != physical_address_2) {
      eviction->congruent_address_cache[index].congruent_virtual_addresses[found++] =
        virtual_address_2;
    }

    if (found == ADDRESS_COUNT) {
      break;
    }
  }

  // Abort if we were not able to find enough addresses
  assert(found == ADDRESS_COUNT);

  eviction->congruent_address_cache[index].used = true;
}
