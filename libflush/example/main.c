 /* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <sched.h>
#include <sys/mman.h>

#include <libflush/libflush.h>

#define HISTOGRAM_SIZE 300
#define HISTOGRAM_ENTRIES 50000
#define HISTOGRAM_SCALE 5
#define HISTOGRAM_THRESHOLD 100

#define BIND_TO_CPU 0
#define BIND_THREAD_TO_CPU 1

#define MIN(a, b) ((a) > (b)) ? (b) : (a)
#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#define _STR(x) #x
#define STR(x) _STR(x)

static void
print_help(char* argv[]) {
  fprintf(stdout, "Usage: %s [OPTIONS]\n", argv[0]);
  fprintf(stdout, "\t-f, -function <value>\t Function (default: flush_reload)\n");
  fprintf(stdout, "\t-c, -cpu <value>\t Bind to cpu (default: " STR(BIND_TO_CPU) ")\n");
  fprintf(stdout, "\t-t, -thread-cpu <value>\t Bind thread to cpu (only for thread counter) (default: " STR(BIND_THREAD_TO_CPU) ")\n");
  fprintf(stdout, "\t-l, -logfile <value>\t Logfile in csv format\n");
  fprintf(stdout, "\t-s, -size <value>\t Histogram size (default: " STR(HISTOGRAM_SIZE) ")\n");
  fprintf(stdout, "\t-n, -entries <value>\t Number of histogram entries (default: " STR(HISTOGRAM_ENTRIES) ")\n");
  fprintf(stdout, "\t-x, -scale <value>\t Histogram scale (default: " STR(HISTOGRAM_SCALE) ")\n");
  fprintf(stdout, "\t-z, -threshold <value>\t Histogram threshold (default: " STR(HISTOGRAM_THRESHOLD) ")\n");
  fprintf(stdout, "\t-h, -help\t\t Help page\n");
}

typedef void (*hit_function_t)(libflush_session_t* libflush_session, void*
    address,  size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);

typedef void (*miss_function_t)(libflush_session_t* libflush_session, void*
    address, size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);

typedef struct function_mapping_s {
  const char* name;
  hit_function_t hit_function;
  miss_function_t miss_function;
} function_mapping_t;

static void flush_reload_hit(libflush_session_t* libflush_session, void*
    address, size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void flush_reload_miss(libflush_session_t* libflush_session, void*
    address, size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void prime_probe_hit(libflush_session_t* libflush_session, void*
    address, size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void prime_probe_miss(libflush_session_t* libflush_session, void*
    address, size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void evict_reload_hit(libflush_session_t* libflush_session, void*
    address, size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void evict_reload_miss(libflush_session_t* libflush_session, void*
    address, size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void flush_flush_hit(libflush_session_t* libflush_session, void* address,
    size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void flush_flush_miss(libflush_session_t* libflush_session, void*
    address, size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void prefetch_hit(libflush_session_t* libflush_session, void* address,
    size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);
static void prefetch_miss(libflush_session_t* libflush_session, void* address,
    size_t runs, size_t* histogram, size_t histogram_size, size_t
    histogram_scale);

function_mapping_t function_mapping[] = {
  { "flush_reload", flush_reload_hit, flush_reload_miss },
  { "prime_probe",  prime_probe_hit,  prime_probe_miss },
  { "evict_reload", evict_reload_hit, evict_reload_miss },
  { "flush_flush",  flush_flush_hit,  flush_flush_miss },
  { "prefetch",     prefetch_hit,     prefetch_miss },
};

int
main(int argc, char* argv[])
{
  /* Define parameters */
  size_t cpu = BIND_TO_CPU;
  size_t thread_cpu = BIND_THREAD_TO_CPU;
  FILE* logfile = NULL;
  hit_function_t hit_function = flush_reload_hit;
  miss_function_t miss_function = flush_reload_miss;
  size_t histogram_size = HISTOGRAM_SIZE;
  size_t histogram_scale = HISTOGRAM_SCALE;
  size_t histogram_entries = HISTOGRAM_ENTRIES;
  size_t histogram_threshold = HISTOGRAM_THRESHOLD;

  /* Parse arguments */
  static const char* short_options = "f:c:t:l:s:n:x:z:h";
  static struct option long_options[] = {
    {"function",        required_argument, NULL, 'f'},
    {"cpu",             required_argument, NULL, 'c'},
    {"thread-cpu",      required_argument, NULL, 't'},
    {"logfile",         required_argument, NULL, 'l'},
    {"size",            required_argument, NULL, 's'},
    {"entries",         required_argument, NULL, 'n'},
    {"scale",           required_argument, NULL, 'x'},
    {"threshold",       required_argument, NULL, 'z'},
    {"help",            no_argument,       NULL, 'h'},
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
      case 'l':
        logfile = fopen(optarg, "w+");
        if (logfile == NULL) {
          fprintf(stderr, "Error: Could not open logfile '%s'\n", optarg);
          return -1;
        }
        fprintf(logfile, "Time,Hit,Miss\n");
        break;
      case 'f':
        {
          bool found = false;
          for (size_t i = 0; i < LENGTH(function_mapping); i++) {
            if (strcmp(optarg, function_mapping[i].name) == 0) {
              hit_function = function_mapping[i].hit_function;
              miss_function = function_mapping[i].miss_function;
              found = true;
              break;
            }
          }

          if (found == false) {
            fprintf(stderr, "Error: Invalid function mapping '%s'\n", optarg);
            return -1;
          }
        }
        break;
      case 's':
        histogram_size = atoi(optarg);
        break;
      case 'n':
        histogram_entries = atoi(optarg);
        break;
      case 'x':
        histogram_scale = atoi(optarg);
        break;
      case 'z':
        histogram_threshold = atoi(optarg);
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

  /* Bind to CPU */
  cpu = cpu % number_of_cpus;
  thread_cpu = thread_cpu % number_of_cpus;

  if (libflush_bind_to_cpu(cpu) == false) {
    fprintf(stderr, "Warning: Could not bind to CPU: %zu\n", cpu);
  }

  /* Initialize libflush */
  libflush_session_args_t args = { 0 };
  args.bind_to_cpu = thread_cpu;
  libflush_session_t* libflush_session;
  if (libflush_init(&libflush_session, &args) == false) {
    fprintf(stderr, "Error: Could not initialize libflush\n");
    return -1;
  }

  /* Allocate histograms */
  size_t* hit_histogram = calloc(histogram_size, sizeof(size_t));
  if (hit_histogram == NULL) {
    fprintf(stderr, "Error: Could not allocate memory for histogram.\n");
    return -1;
  }

  size_t* miss_histogram = calloc(histogram_size, sizeof(size_t));
  if (miss_histogram == NULL) {
    fprintf(stderr, "Error: Could not allocate memory for histogram.\n");
    return -1;
  }

  /* Map memory region */
#define MAP_SIZE 4096
  void* array = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (array == MAP_FAILED) {
    fprintf(stderr, "Error: Could not map memory.\n");

    free(hit_histogram);
    free(miss_histogram);

    return -1;
  }

  /* Chose target address */
  void* address = (void*) ((char*) array + MAP_SIZE / 2);
  libflush_access_memory(address);

  /* Measure hit function */
  hit_function(libflush_session, address, histogram_entries,
      hit_histogram, histogram_size, histogram_scale);

  /* Measure miss function */
  libflush_flush(libflush_session, address);

  miss_function(libflush_session, address, histogram_entries,
      miss_histogram, histogram_size, histogram_scale);

  // Get the maximum value of a cache hit and the minimum value of a cache miss
  size_t hit_maximum_index = 0;
  size_t hit_maximum = 0;

  size_t miss_minimum_index = 0;
  size_t miss_maximum = 0;
  size_t miss_maximum_index = 0;

  for (size_t i = 0; i < histogram_size; i++) {
      if (hit_maximum < hit_histogram[i]) {
          hit_maximum = hit_histogram[i];
          hit_maximum_index = i;
      }

      if (miss_maximum < miss_histogram[i]) {
          miss_maximum = miss_histogram[i];
          miss_maximum_index = i;
      }

      if (miss_histogram[i] > histogram_threshold && miss_minimum_index == 0) {
          miss_minimum_index = i;
      }
  }

  int cache = hit_maximum_index * histogram_scale;
  int mem = miss_maximum_index * histogram_scale;
  int threshhold = mem - (mem - cache) / 2;

  for (unsigned int i = 0; i < histogram_size; i++) {
    fprintf(stdout, "%4zu: %10zu %10zu\n", i * histogram_scale, hit_histogram[i], miss_histogram[i]);
    if (logfile != NULL) {
      fprintf(logfile, "%zu,%zu,%zu\n", i * histogram_scale, hit_histogram[i], miss_histogram[i]);
    }
  }

  fprintf(stderr, "Cache access time: %d\n", cache);
  fprintf(stderr, "Memory access time: %d\n", mem);
  fprintf(stderr, "Threshhold: %d\n", threshhold);

  /* Clean-up */
  free(hit_histogram);
  free(miss_histogram);

  if (logfile != NULL) {
    fclose(logfile);
  }

  if (array != NULL) {
    munmap(array, MAP_SIZE);
  }

  /* Terminate libflush */
  if (libflush_terminate(libflush_session) == false) {
    fprintf(stderr, "Error: Could not terminate libflush\n");
    return -1;
  }

  return 0;
}

static void
flush_reload_hit(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  for (unsigned int i = 0; i < runs; i++) {
    size_t time = libflush_reload_address(libflush_session, address);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
flush_reload_miss(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  for (unsigned int i = 0; i < runs; i++) {
    size_t time = libflush_reload_address_and_flush(libflush_session, address);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
prime_probe_hit(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  size_t set_index = libflush_get_set_index(libflush_session, address);

  for (unsigned int i = 0; i < runs; i++) {
    libflush_prime(libflush_session, set_index);
    size_t time = libflush_probe(libflush_session, set_index);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
prime_probe_miss(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  size_t set_index = libflush_get_set_index(libflush_session, address);

  for (unsigned int i = 0; i < runs; i++) {
    libflush_prime(libflush_session, set_index);
    libflush_access_memory(address);
    size_t time = libflush_probe(libflush_session, set_index);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
evict_reload_hit(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  for (unsigned int i = 0; i < runs; i++) {
    size_t time = libflush_reload_address(libflush_session, address);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
evict_reload_miss(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  libflush_evict(libflush_session, address);

  for (unsigned int i = 0; i < runs; i++) {
    size_t time = libflush_reload_address_and_evict(libflush_session, address);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
flush_flush_hit(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  for (unsigned int i = 0; i < runs; i++) {
    libflush_reload_address(libflush_session, address);
    size_t time = libflush_flush_time(libflush_session, address);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
flush_flush_miss(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  for (unsigned int i = 0; i < runs; i++) {
    size_t time = libflush_flush_time(libflush_session, address);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
prefetch_hit(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  for (unsigned int i = 0; i < runs; i++) {
    size_t time = libflush_prefetch_time(libflush_session, address);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

static void
prefetch_miss(libflush_session_t* libflush_session, void* address, size_t runs, size_t* histogram, size_t
    histogram_size, size_t histogram_scale)
{
  for (unsigned int i = 0; i < runs; i++) {
    libflush_flush(libflush_session, address);
    size_t time = libflush_prefetch_time(libflush_session, address);
    histogram[MIN(histogram_size - 1, time / histogram_scale)]++;
    sched_yield();
  }
}

