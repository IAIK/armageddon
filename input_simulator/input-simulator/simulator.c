/* See LICENSE file for license and copyright information */

#include "simulator.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct simulator_s {
  int fd_tap;
};

static bool send_event(int fd, uint16_t type, uint16_t code, uint32_t value);

bool
simulator_init(simulator_t** simulator)
{
  if (simulator == NULL) {
    return false;
  }

  *simulator = calloc(1, sizeof(simulator_t));
  if (*simulator == NULL) {
    return false;
  }

#if defined(ALTO45)
  (*simulator)->fd_tap = open("/dev/input/event5", O_WRONLY);
#elif defined(MAKO)
  (*simulator)->fd_tap = open("/dev/input/event2", O_WRONLY);
#else
  (*simulator)->fd_tap = open("/dev/input/event1", O_WRONLY);
#endif
  if ((*simulator)->fd_tap < 0) {
    fprintf(stderr, "[simulator] error: Could not open event path\n");
    return false;
  }

  return true;
}

bool
simulator_terminate(simulator_t* simulator)
{
  if (simulator == NULL) {
    return false;
  }

  if (simulator->fd_tap >= 0) {
    close(simulator->fd_tap);
  }

  free(simulator);

  return true;
}

static bool
send_event(int fd, uint16_t type, uint16_t code, uint32_t value)
{
  struct input_event event;
  memset(&event, 0, sizeof(event));

  event.type = type;
  event.code = code;
  event.value = value;

  ssize_t ret = write(fd, &event, sizeof(event));
  if(ret < (ssize_t) sizeof(event)) {
    fprintf(stderr, "Could not write\n");
    return false;
  }

  return true;
}

bool simulator_send_char(simulator_t* simulator, const char* key)
{
  if (simulator == NULL || key == NULL) {
    return false;
  }

  key_mapping_t* key_mapping = NULL;
  for (unsigned int i = 0; key_mappings[i].input != NULL; i++) {
    if (strcmp(key_mappings[i].input, key) == 0) {
      key_mapping = &key_mappings[i];
      break;
    }
  }

  if (key_mapping != NULL) {
    simulator_send_tap(simulator, key_mapping->x, key_mapping->y);
  }

  return true;
}

bool
simulator_get_coordinates(simulator_t* simulator, const char* input, int* x, int* y)
{
  if (simulator == NULL || input == NULL || x == NULL || y == NULL) {
    return false;
  }

  for (unsigned int i = 0; key_mappings[i].input != NULL; i++) {
    if (strcmp(key_mappings[i].input, input) == 0) {
      *x = key_mappings[i].x;
      *y = key_mappings[i].y;
      return true;
    }
  }

  return false;
}

#if defined(ALTO45)
bool
simulator_send_tap(simulator_t* simulator, int x, int y)
{
  int fd = simulator->fd_tap;

  if (send_event(fd, EV_ABS, ABS_MT_SLOT, 0) == false) {
    return false;
  }

  if (send_event(fd, EV_ABS, ABS_MT_TRACKING_ID, 49) == false) {
    return false;
  }
  if (send_event(fd, EV_ABS, ABS_MT_POSITION_X, x) == false) {
    return false;
  }

  if (send_event(fd, EV_ABS, ABS_MT_POSITION_Y, y) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, BTN_TOUCH, 1) == false) {
    return false;
  }

  if (send_event(fd, EV_SYN, SYN_REPORT, 0) == false) {
    return false;
  }

  if (send_event(fd, EV_ABS, ABS_MT_TRACKING_ID, -1) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, BTN_TOUCH, 0) == false) {
    return false;
  }

  if (send_event(fd, EV_SYN, SYN_REPORT, 0) == false) {
    return false;
  }

  sched_yield();

  return true;
}
#elif defined(MAKO)
bool
simulator_send_tap(simulator_t* simulator, int x, int y)
{
  y *= 2;
  x *= 2;

  int fd = simulator->fd_tap;

  if (send_event(fd, EV_ABS, ABS_MT_TRACKING_ID, 49) == false) {
    return false;
  }
  if (send_event(fd, EV_ABS, ABS_MT_POSITION_X, x) == false) {
    return false;
  }

  if (send_event(fd, EV_ABS, ABS_MT_POSITION_Y, y) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, ABS_MT_PRESSURE, 31) == false) {
    return false;
  }

  if (send_event(fd, EV_SYN, SYN_REPORT, 0) == false) {
    return false;
  }

  if (send_event(fd, EV_ABS, ABS_MT_TRACKING_ID, -1) == false) {
    return false;
  }

  if (send_event(fd, EV_SYN, SYN_REPORT, 0) == false) {
    return false;
  }

  sched_yield();

  return true;
}
#elif defined(ZEROFLTE)
bool
simulator_send_tap(simulator_t* simulator, int x, int y)
{
  y *= 1.6;
  x *= 2.8;

  int fd = simulator->fd_tap;

  if (send_event(fd, EV_ABS, ABS_MT_TRACKING_ID, 49) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, BTN_TOUCH, 1) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, 0x14a, 1) == false) { // FINGER
    return false;
  }

  if (send_event(fd, EV_ABS, ABS_MT_POSITION_X, x) == false) {
    return false;
  }

  if (send_event(fd, EV_ABS, ABS_MT_POSITION_Y, y) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, ABS_MT_TOUCH_MAJOR, 4) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, ABS_MT_TOUCH_MINOR, 4) == false) {
    return false;
  }

  if (send_event(fd, EV_SYN, SYN_REPORT, 0) == false) {
    return false;
  }

  if (send_event(fd, EV_ABS, ABS_MT_TRACKING_ID, 255) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, BTN_TOUCH, 0) == false) {
    return false;
  }

  if (send_event(fd, EV_KEY, 0x14a, 0) == false) { // FINGER
    return false;
  }

  if (send_event(fd, EV_SYN, SYN_REPORT, 0) == false) {
    return false;
  }

  sched_yield();

  return true;
}
#endif
