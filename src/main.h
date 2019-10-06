#ifndef MAIN_H
#define MAIN_H

void main_printHelp();
void main_printVersion();

// Handle SIGINT (CTRL + C)
void handleSignalSIGINT(int signalNumber);
// Handle SIGTERM (kill etc.)
void handleSignalSIGTERM(int signalNumber);
// Handle SIGKILL (unblockable - just used for logging)
void handleSignalSIGKILL(int signalNumber);
// Handle SIGCHLD (a child exited)
void handleSignalSIGCHLD(int signalNumber);

#endif
