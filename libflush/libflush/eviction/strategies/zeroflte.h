/* See LICENSE file for license and copyright information */

/* Samsung S6 (A53) */
#if 1
#define NUMBER_OF_SETS 512
#define LINE_LENGTH_LOG2 6
#define LINE_LENGTH 64
#define ES_EVICTION_COUNTER 21
#define ES_NUMBER_OF_ACCESSES_IN_LOOP 2
#define ES_DIFFERENT_ADDRESSES_IN_LOOP 5
#else
/* Samsung S6 (A57) */
#define NUMBER_OF_SETS 2048
#define LINE_LENGTH_LOG2 6
#define LINE_LENGTH 64
#define ES_EVICTION_COUNTER 25
#define ES_NUMBER_OF_ACCESSES_IN_LOOP 10
#define ES_DIFFERENT_ADDRESSES_IN_LOOP 10
#endif
