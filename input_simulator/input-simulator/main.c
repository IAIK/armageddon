 /* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "simulator.h"

static void
print_help(char* argv[]) {
  fprintf(stdout, "Usage: %s [OPTIONS] <character>\n", argv[0]);
  fprintf(stdout, "Application Options:\n");
  fprintf(stdout, "\t-r, -repititions <range>\t Number of repititions to execute the event\n");
  fprintf(stdout, "\t-d, -delay <delay>\t Delay in seconds\n");
  fprintf(stdout, "\t-h, -help\t Help page\n");
}

int main(int argc, char* argv[])
{
  /* Define parameters */
  ssize_t repititions  = 1;
  useconds_t delay = 50000;

  /* Parse arguments */
  static const char* short_options = "r:d:h";
  static struct option long_options[] = {
    {"repitition", required_argument, NULL, 'r'},
    {"delay",      required_argument, NULL, 'd'},
    {"help",       no_argument, NULL, 'h'},
    { NULL,        0, NULL, 0}
  };

  opterr = 0;

  int c;
  while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    switch (c) {
      case 'r':
        {
          if (!sscanf(optarg,"%zd",&repititions)) {
            fprintf(stderr, "Could not parse repititions parameter: %s\n", optarg);
            return -1;
          }
        }
        break;
      case 'd':
        {
          float delay_seconds;
          if (!sscanf(optarg,"%f", &delay_seconds)) {
            fprintf(stderr, "Could not parse delay parameter: %s\n", optarg);
            return -1;
          }

          delay = delay_seconds * 1000 * 1000;
        }
        break;
      case 'h':
        print_help(argv);
        break;
      case ':':
        fprintf(stderr, "Error: option `-%c' requires an argument\n", optopt);
        break;
      case '?':
      default:
        fprintf(stderr, "Error: Invalid option '-%c'\n", optopt);
        return -1;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Error: No character passed\n");
    return 0;
  }

  char* input = argv[optind];

  /* Init simulator */
  simulator_t* simulator;
  if (simulator_init(&simulator) == false) {
    fprintf(stderr, "Could not initialize simulator\n");
    return -1;
  }

  /* Find key mapping */
  int x = 0;
  int y = 0;
  if (simulator_get_coordinates(simulator, input, &x, &y) == false) {
    fprintf(stderr, "Could not find mapping for input '%s'\n", input);
  }

  /* Simulate taps */
  for (ssize_t i = 0; i < repititions || repititions < 0; i++) {
    simulator_send_tap(simulator, x, y);
    usleep(delay);
  }

  /* Terminate simulator */
  if (simulator_terminate(simulator) == false) {
    fprintf(stderr, "Could not terminate simulator\n");
    return -1;
  }

  return 0;
}

