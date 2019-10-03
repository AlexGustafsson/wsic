#ifndef TIME_H
#define TIME_H

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#define DAY_IN_SECONDS 86400
#define HOUR_IN_SECONDS 3600
#define MINUTE_IN_SECONDS 60

#define HOURS_IN_DAY 24
#define MINUTES_IN_HOUR 60
#define SECONDS_IN_MINUTE 60

// Note that the start of epoch may be anything - since start of UNIX epoch or since start of computer
// Only use for telling time difference
void time_getTimeSinceStartOfEpoch(struct timespec *timespec);
// Resets the time
void time_reset();
// Time since time_reset() was called. Undefined behaviour if it hasn't been called
void time_getTimeSinceStart(uint64_t *nanoseconds, uint64_t *seconds);
uint64_t time_getElapsedTime(struct timespec *timespec);
// Format time in [d h m s ms.ns]
void time_formatTime(uint64_t nanoseconds, uint64_t seconds, uint64_t *formatedMilliseconds, uint64_t *formatedSeconds, uint64_t *formatedMinutes, uint64_t *formatedHours, uint64_t *formatedDays, uint64_t *formatedNanoseconds);
#endif
