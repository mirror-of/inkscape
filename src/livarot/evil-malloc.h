/* this file is in the public domain */

#ifndef SEEN_EVIL_MALLOC
#define SEEN_EVIL_MALLOC

#include <stdlib.h>
#include <sys/types.h>

/* get around livarot's malloc abuse so we can run diagnostic tools
 * like ElectricFence on the rest of the codebase */

/* explanation: standard malloc is _not_ guaranteed to return NULL
 * for 0 size allocations, and the returned pointer may have strange
 * properties.  For this reason ElectricFence and other tools usually
 * spit warnings or errors when zero-length allocations are attempted. */

inline __attribute__((deprecated)) void *evil_malloc(size_t size) {
	return size ? malloc(size) : NULL;
}

#endif 

