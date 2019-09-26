#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "time.h"

static struct timespec time_start;

void time_getTimeSinceStartOfEpoch(struct timespec *timespec) {
#ifdef __APPLE__
  clock_serv_t clock;
  mach_timespec_t machTimeSpec;
  host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clock);
  clock_get_time(clock, &machTimeSpec);
  mach_port_deallocate(mach_task_self_, clock);
  timespec->tv_nsec = machTimeSpec.tv_nsec;
  timespec->tv_sec = machTimeSpec.tv_sec;
#else
  clock_gettime(CLOCK_MONOTONIC, timespec);
#endif
}

void time_reset() {
  time_getTimeSinceStartOfEpoch(&time_start);
}

uint64_t time_getTimeSinceStart() {
  struct timespec now;
  time_getTimeSinceStartOfEpoch(&now);

  uint64_t startNanoSeconds = (time_start.tv_sec * 1000000000) + time_start.tv_nsec;
  uint64_t nowNanoSeconds = (now.tv_sec * 1000000000) + now.tv_nsec;

  return nowNanoSeconds - startNanoSeconds;
}

uint64_t time_getElapsedTime(struct timespec *timespec) {
  struct timespec start;
  time_getTimeSinceStartOfEpoch(&start);

  uint64_t startNanoSeconds = (start.tv_sec * 1000000000) + start.tv_nsec;
  uint64_t nowNanoSeconds = (timespec->tv_sec * 1000000000) + timespec->tv_nsec;

  return nowNanoSeconds - startNanoSeconds;
}
