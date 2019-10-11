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

void time_getFormattedElapsedTime(uint64_t nanoseconds, uint64_t seconds, uint64_t *formattedNanoseconds, uint64_t *formattedMilliseconds, uint64_t *formattedSeconds, uint64_t *formattedMinutes, uint64_t *formattedHours, uint64_t *formattedDays) {
  if (formattedNanoseconds != 0)
    *formattedNanoseconds = (nanoseconds / (uint64_t)10000) % (uint64_t)100;

  if (formattedMilliseconds != 0)
    *formattedMilliseconds = (nanoseconds / (uint64_t)1000000) % (uint64_t)1000;

  if (formattedSeconds != 0)
    *formattedSeconds = (seconds) % SECONDS_IN_MINUTE;

  if (formattedMinutes != 0)
    *formattedMinutes = (seconds / MINUTE_IN_SECONDS) % MINUTES_IN_HOUR;

  if (formattedHours != 0)
    *formattedHours = (seconds / HOUR_IN_SECONDS) % HOURS_IN_DAY;

  if (formattedDays != 0)
    *formattedDays = (seconds / DAY_IN_SECONDS);

  return;
}

bool time_getTimeSinceStart(uint64_t *nanoseconds, uint64_t *seconds) {
  struct timespec now;
  time_getTimeSinceStartOfEpoch(&now);

  if (nanoseconds == 0 && seconds != 0) {
    // Return time since start in seconds
    *seconds = now.tv_sec - time_start.tv_sec;
    return true;
  } else if (nanoseconds != 0 && seconds == 0) {
    // Return time since start in nanoseconds
    *nanoseconds = (now.tv_nsec + now.tv_sec * 1000000000) - (time_start.tv_nsec + time_start.tv_sec * 1000000000);
    return true;
  } else if (nanoseconds != 0 && seconds != 0) {
    // Return time since start in separet seconds and nanoseconds
    *nanoseconds = now.tv_nsec - time_start.tv_nsec;
    *seconds = now.tv_sec - time_start.tv_sec;
    return true;
  } else {
    // If both in patameters was null, do nothing
    return false;
  }
}

uint64_t time_getElapsedTime(const struct timespec *timespec) {
  struct timespec start;
  time_getTimeSinceStartOfEpoch(&start);

  uint64_t startNanoSeconds = (start.tv_sec * 1000000000) + start.tv_nsec;
  uint64_t nowNanoSeconds = (timespec->tv_sec * 1000000000) + timespec->tv_nsec;

  return nowNanoSeconds - startNanoSeconds;
}
