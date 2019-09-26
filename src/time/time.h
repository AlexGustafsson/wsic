#ifndef TIME_H
#define TIME_H

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

// Note that the start of epoch may be anything - since start of UNIX epoch or since start of computer
// Only use for telling time difference
void time_getTimeSinceStartOfEpoch(struct timespec *timespec);
// Resets the time
void time_reset();
// Time since time_reset() was called. Undefined behaviour if it hasn't been called
uint64_t time_getTimeSinceStart();
uint64_t time_getElapsedTime(struct timespec *timespec);

#endif
