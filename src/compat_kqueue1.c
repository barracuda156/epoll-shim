#include "compat_kqueue1.h"

#include <sys/types.h>

#include <sys/event.h>
#include <sys/time.h>

#include <errno.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include "errno_return.h"
#include "wrap.h"

/*
 * On pre-10.7 Darwin SDKs there is no O_CLOEXEC (the open(2) flag
 * arrived in Lion); MacPorts legacy-support defines it to 0 so that
 * code compiles, which makes "flags & O_CLOEXEC" dead code. Worse,
 * the Apple else-branch below would then *always* run and strip
 * FD_CLOEXEC. All emulated fds (epollfd/timerfd/eventfd/signalfd)
 * are created through kqueue1(), and their callers pass *_CLOEXEC
 * (aliases of O_CLOEXEC), so treat a zero O_CLOEXEC as always
 * requested -- otherwise every one of those fds leaks into every
 * child process spawned by the application.
 */
#if defined(__APPLE__) && O_CLOEXEC == 0
#define CLOEXEC_REQUESTED(flags) 1
#else
#define CLOEXEC_REQUESTED(flags) (((flags) & O_CLOEXEC) != 0)
#endif

static errno_t
compat_kqueue1_impl(int *fd_out, int flags)
{
	errno_t ec;

	if (flags & ~(O_CLOEXEC | O_NONBLOCK)) {
		return EINVAL;
	}

	int fd = kqueue();
	if (fd < 0) {
		return errno;
	}

	{
		int r;

		if (CLOEXEC_REQUESTED(flags)) {
			if ((r = real_fcntl(fd, F_GETFD)) < 0 ||
			    real_fcntl(fd, F_SETFD, r | FD_CLOEXEC) < 0) {
				ec = errno;
				goto out;
			}
		}
#ifdef __APPLE__
		else {
			if ((r = fcntl(fd, F_GETFD, 0)) < 0 ||
			    fcntl(fd, F_SETFD, r & ~FD_CLOEXEC) < 0) {
				ec = errno;
				goto out;
			}
		}
#endif

		if (flags & O_NONBLOCK) {
			if ((r = real_fcntl(fd, F_GETFL)) < 0) {
				ec = errno;
				goto out;
			}

			if (real_fcntl(fd, F_SETFL, r | O_NONBLOCK) < 0 &&
			    errno != ENOTTY) {
				ec = errno;
				goto out;
			}
		}
	}

	*fd_out = fd;
	return 0;

out:
	(void)real_close(fd);
	return ec;
}

int
compat_kqueue1(int flags)
{
	ERRNO_SAVE;
	errno_t ec;

	int fd;
	ec = compat_kqueue1_impl(&fd, flags);

	ERRNO_RETURN(ec, -1, fd);
}
