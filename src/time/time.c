#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "time.h"

#include "../logging/logging.h"

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

void time_getTimeSinceStart(uint64_t *nanoseconds, uint64_t *seconds) {
  struct timespec now;
  time_getTimeSinceStartOfEpoch(&now);

  if (nanoseconds == 0 && seconds != 0) {
    // Return time since start in seconds
    *seconds = now.tv_sec - time_start.tv_sec;

  } else if (nanoseconds != 0 && seconds == 0) {
    // Return time since start in nanoseconds
    *nanoseconds = (now.tv_nsec + now.tv_sec * 1000000000) - (time_start.tv_nsec + time_start.tv_sec * 1000000000);

  } else if (nanoseconds != 0 && seconds != 0) {
    // Return time since start in separet seconds and nanoseconds
    *nanoseconds = now.tv_nsec - time_start.tv_nsec;
    *seconds = now.tv_sec - time_start.tv_sec;

  } else
    // If both in patameters was null, do nothing
    log(LOG_ERROR, "Can get time, inparameters was null");
}

void time_formatTime(uint64_t nanoseconds, uint64_t seconds, uint64_t *formatedMilliseconds, uint64_t *formatedSeconds, uint64_t *formatedMinutes, uint64_t *formatedHours, uint64_t *formatedDays, uint64_t *formatedNanoseconds) {
  // modolu 100 because it is just two decimals
  *formatedNanoseconds = (nanoseconds / (uint64_t)10000) % (uint64_t)100;
  *formatedMilliseconds = (nanoseconds / (uint64_t)1000000) % (uint64_t)1000;
  *formatedSeconds = (seconds) % SECONDS_IN_MINUTE;
  *formatedMinutes = (seconds / MINUTE_IN_SECONDS) % MINUTES_IN_HOUR;
  *formatedHours = (seconds / HOUR_IN_SECONDS) % HOURS_IN_DAY;
  *formatedDays = (seconds / DAY_IN_SECONDS);
}

uint64_t time_getElapsedTime(struct timespec *timespec) {
  struct timespec start;
  time_getTimeSinceStartOfEpoch(&start);

  uint64_t startNanoSeconds = (start.tv_sec * 1000000000) + start.tv_nsec;
  uint64_t nowNanoSeconds = (timespec->tv_sec * 1000000000) + timespec->tv_nsec;

  return nowNanoSeconds - startNanoSeconds;
}
