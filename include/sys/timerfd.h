#ifndef EPOLL_SHIM_SYS_TIMERFD_H_
#define EPOLL_SHIM_SYS_TIMERFD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>
#include <time.h>

#define TFD_NONBLOCK O_NONBLOCK
#define TFD_CLOEXEC O_CLOEXEC

#define TFD_TIMER_ABSTIME 1
#define TFD_TIMER_CANCEL_ON_SET (1 << 1)

#ifdef COMPAT_ENABLE_ITIMERSPEC
/*
 * The host libc's <time.h> doesn't define struct itimerspec (e.g.
 * Darwin, which has no POSIX per-process timer support), so provide
 * it here. Only defined when the build has actually detected the
 * struct is missing (see CMakeLists.txt's HAVE_TIMERFD/APPLE check),
 * to avoid ever conflicting with a real system definition.
 *
 * Guarded separately from src/compat_itimerspec.h's copy of the same
 * definition (which epoll-shim's own build force-includes into every
 * translation unit compiled for the library, this header included),
 * so whichever of the two is seen first in a given file wins and the
 * second is a no-op instead of a redefinition error.
 */
#ifndef EPOLL_SHIM_ITIMERSPEC_DEFINED_
#define EPOLL_SHIM_ITIMERSPEC_DEFINED_
struct itimerspec {
	struct timespec it_interval;
	struct timespec it_value;
};
#endif
#else
struct itimerspec;
#endif

int timerfd_create(int, int);
int timerfd_settime(int, int, struct itimerspec const *, struct itimerspec *);
int timerfd_gettime(int, struct itimerspec *);


#ifndef EPOLL_SHIM_DISABLE_WRAPPER_MACROS
#include <epoll-shim/detail/common.h>
#include <epoll-shim/detail/poll.h>
#include <epoll-shim/detail/read.h>
#endif


#ifdef __cplusplus
}
#endif

#endif
