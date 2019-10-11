#ifndef DAEMON_H
#define DAEMON_H

#include <unistd.h>

// Creates a deamon process and returns its pid or -1 if it failed
// Keeps STDOUT, STDIN and STDERR open - close seperately
pid_t daemonize();

#endif
