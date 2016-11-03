 /* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <sched.h>

#include <libflush/libflush.h>

#define NUMBER_OF_RUNS (1ull*1000ull*1000ull)
#define BATCH_SIZE 5000
#define BIND_TO_CPU 1

#define MIN(a, b) ((a) > (b)) ? (b) : (a)

static void
print_help(char* argv[]) {
  fprintf(stdout, "Usage: %s [OPTIONS]\n", argv[0]);
  fprintf(stdout, "\t-c, -cpu <value>\t Bind to cpu\n");
  fprintf(stdout, "\t-t, -thread-cpu <value>\t Bind thread to cpu (only for thread counter)\n");
  fprintf(stdout, "\t-n, -number-of-measurements <value>\t Number of measurements\n");
  fprintf(stdout, "\t-b, -batch-size <value>\t Batch size\n");
  fprintf(stdout, "\t-h, -help\t Help page\n");
}

int
main(int argc, char* argv[])
{
  /* Define parameters */
  size_t cpu = BIND_TO_CPU;
  size_t thread_cpu = BIND_TO_CPU + 1;
  FILE* logfile = NULL;
  uint64_t number_of_runs = NUMBER_OF_RUNS;
  uint64_t batch_size = BATCH_SIZE;

  /* Parse arguments */
  static const char* short_options = "c:t:n:b:h";
  static struct option long_options[] = {
    {"cpu",             required_argument, NULL, 'c'},
    {"thread-cpu",      required_argument, NULL, 't'},
    {"number-of-runs",  required_argument, NULL, 'n'},
    {"batch-size",      required_argument, NULL, 'b'},
    {"help",            no_argument, NULL, 'h'},
    { NULL,             0, NULL, 0}
  };

  size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);

  int c;
  while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    switch (c) {
      case 'c':
        cpu = atoi(optarg);
        if (cpu >= number_of_cpus) {
          fprintf(stderr, "Error: CPU %zu is not available.\n", cpu);
          return -1;
        }
        break;
      case 't':
        thread_cpu = atoi(optarg);
        if (thread_cpu >= number_of_cpus) {
          fprintf(stderr, "Error: CPU %zu is not available.\n", thread_cpu);
          return -1;
        }
        break;
      case 'n':
        number_of_runs = atoi(optarg);
        break;
      case 'b':
        batch_size = atoi(optarg);
        break;
      case 'h':
        print_help(argv);
        return 0;
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
    fprintf(stderr, "Error: No logfile passed\n");
    return 0;
  }

  logfile = fopen(argv[optind], "w+");
  if (logfile == NULL) {
    fprintf(stderr, "Error: Could not open logfile '%s'\n", argv[optind]);
    return -1;
  }

  /* Bind to CPU */
  cpu = cpu % number_of_cpus;
  thread_cpu = thread_cpu % number_of_cpus;

  if (libflush_bind_to_cpu(cpu) == false) {
    fprintf(stderr, "Could not bind to CPU: %zu\n", cpu);
  }

  char buffer[4096] = {0};
  void* address = (void*) ((size_t) &buffer[1024] & ~(0x3F));

  /* Initialize libflush */
  libflush_session_args_t args;
  args.bind_to_cpu = thread_cpu;
  libflush_session_t* libflush_session;
  if (libflush_init(&libflush_session, &args) == false) {
    fprintf(stderr, "Error: Could not initialize libflush.\n");
    return -1;
  }

  // Initialize results
  uint64_t* miss_measurements = calloc(number_of_runs, sizeof(uint64_t));
  if (miss_measurements == NULL) {
    fprintf(stderr, "Error: Out of memory\n");
    return -1;
  }

  uint64_t* execution_measurements = calloc(number_of_runs, sizeof(uint64_t));
  if (execution_measurements == NULL) {
    fprintf(stderr, "Error: Out of memory\n");
    return -1;
  }

  uint64_t number_of_batches = ceil(number_of_runs / batch_size);

  uint64_t* execution_batch_measurements = calloc(number_of_batches, sizeof(uint64_t));
  if (execution_batch_measurements == NULL) {
    fprintf(stderr, "Error: Out of memory\n");
    return -1;
  }

  libflush_flush(libflush_session, address);

  // Measure time it takes to access something from the memory
  for (unsigned int i = 0; i < number_of_runs; i++) {
      uint64_t time = libflush_reload_address_and_flush(libflush_session, address);
      miss_measurements[i] = time;
      sched_yield();
  }

  // Miss time
  libflush_reset_timing(libflush_session);
  for (unsigned int i = 0; i < number_of_runs; i++) {
      uint64_t begin = libflush_get_timing(libflush_session);
      libflush_flush(libflush_session, address);
      libflush_access_memory(address);
      uint64_t end = libflush_get_timing(libflush_session);
      execution_measurements[i] = end - begin;
      sched_yield();
  }

  for (unsigned int b = 0; b < number_of_batches; b++) {
    libflush_reset_timing(libflush_session);

    uint64_t begin = libflush_get_timing(libflush_session);
    for (unsigned int i = 0; i < batch_size; i++) {
      libflush_access_memory(address);
      libflush_flush(libflush_session, address);
    }
    uint64_t end = libflush_get_timing(libflush_session);
    execution_batch_measurements[b] = end - begin;
    sched_yield();
  }

  fprintf(logfile, "Miss,Runtime,RuntimeBatch\n");
  for (unsigned int i = 0; i < number_of_runs; i++) {
    fprintf(logfile, "%" PRIu64 ",%" PRIu64 ",",
        miss_measurements[i], execution_measurements[i]);

    if (i < number_of_batches) {
      fprintf(logfile, "%" PRIu64 "", execution_batch_measurements[i]);
    }

    fprintf(logfile, "\n");
  }

  fflush(logfile);
  fclose(logfile);

  free(miss_measurements);
  free(execution_measurements);

  /* Terminate libflush */
  libflush_terminate(libflush_session);

  return 0;
}
