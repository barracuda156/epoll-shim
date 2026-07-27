#ifndef COMPAT_ITIMERSPEC_H
#define COMPAT_ITIMERSPEC_H

#ifdef COMPAT_ENABLE_ITIMERSPEC

#include <time.h>

/*
 * Guarded separately from the copy of this definition in
 * include/sys/timerfd.h (which is what external consumers of
 * epoll-shim see, and which epoll-shim's own timerfd.c also includes
 * directly), so whichever of the two headers is seen first in a
 * given translation unit wins and the second is a no-op instead of
 * a redefinition error.
 */
#ifndef EPOLL_SHIM_ITIMERSPEC_DEFINED_
#define EPOLL_SHIM_ITIMERSPEC_DEFINED_
struct itimerspec {
	struct timespec it_interval;
	struct timespec it_value;
};
#endif
#endif

#endif
