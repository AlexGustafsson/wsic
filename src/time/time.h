#ifndef TIME_H
#define TIME_H

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#include "../string/string.h"

// One day in seconds
#define DAY_IN_SECONDS 86400
// One hour in seconds
#define HOUR_IN_SECONDS 3600
// One minute in seconds
#define MINUTE_IN_SECONDS 60

// How many hours it is in one day
#define HOURS_IN_DAY 24
// How many minutes it is in one hour
#define MINUTES_IN_HOUR 60
// How many seconds it is in one minute
#define SECONDS_IN_MINUTE 60

// Note that the start of epoch may be anything - since start of UNIX epoch or since start of computer
// Only use for telling time difference
void time_getTimeSinceStartOfEpoch(struct timespec *timespec);
// Resets the time
void time_reset();
// Convert epoch time to human readable format
void time_getFormattedElapsedTime(uint64_t nanoseconds, uint64_t seconds, uint64_t *formattedNanoseconds, uint64_t *formattedMilliseconds, uint64_t *formattedSeconds, uint64_t *formattedMinutes, uint64_t *formattedHours, uint64_t *formattedDays);
// Time since time_reset() was called. Undefined behaviour if it hasn't been called
void time_getTimeSinceStart(uint64_t *nanoseconds, uint64_t *seconds);

uint64_t time_getElapsedTime(struct timespec *timespec);
#endif
