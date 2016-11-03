/* See LICENSE file for license and copyright information */

#ifndef KEY_MAPPING_H
#define KEY_MAPPING_H

typedef struct key_mapping_s {
  char* input;
  int x;
  int y;
} key_mapping_t;

extern key_mapping_t key_mappings[];

#endif  /*KEY_MAPPING_H*/
