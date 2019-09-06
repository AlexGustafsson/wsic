/**
* This file contains definitions that are set compile time.
* This is mostly for documentation, but also to stop false positives
* when using tools such as clangd for static analysis
*/

// The current version of the application
#ifndef WSIC_VERSION
#define WSIC_VERSION ""
#endif

// The current version of the compiler used
#ifndef COMPILER_VERSION
#define COMPILER_VERSION ""
#endif

// The date and time the project was compiled
// NOTE: depends on the current source file
#ifndef COMPILE_TIME
#define COMPILE_TIME ""
#endif
