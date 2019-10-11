#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../logging/logging.h"

#include "daemon.h"

pid_t daemonize() {
  // Fork the process
  fflush(stdout);
  fflush(stderr);
  pid_t pid = fork();
  if (pid < 0) {
    // Fail if we're unable to fork
    log(LOG_DEBUG, "Unable to fork process");
    return -1;
  } else if (pid > 0) {
    log(LOG_DEBUG, "Exiting the parent");
    // Let the parent exit if the fork was successful
    exit(EXIT_SUCCESS);
  }

  // Reset file modes (to ensure that file modes from open(), mkdir() etc. are followed)
  umask(0);

  // Create a session for the child (daemon) process
  // This creates a new process group where the child is the leader
  pid_t sid = setsid();
  if (sid < 0) {
    log(LOG_DEBUG, "Unable to create child session");
    // Fail if we're unable to create a session
    return -1;
  }

  // Change the CWD of the child process to root to ensure that the process
  // doesn't block any mount points etc.
  if ((chdir("/")) < 0) {
    log(LOG_DEBUG, "Unable to chroot the child process");
    // Fail if we're unable to chroot the child process
    return -1;
  }

  return sid;
}
