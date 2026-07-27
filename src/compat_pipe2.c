#include "compat_pipe2.h"

#include <errno.h>
#include <stdlib.h>

#include <unistd.h>

#include "wrap.h"

/*
 * On pre-10.7 Darwin SDKs there is no O_CLOEXEC (the open(2) flag
 * arrived in Lion); MacPorts legacy-support defines it to 0 so that
 * code compiles, which makes "flags & O_CLOEXEC" dead code. Every
 * caller of this shim passes O_CLOEXEC, so treat a zero O_CLOEXEC
 * as always requested -- otherwise the shim's self-pipes leak into
 * every child process spawned by the application.
 */
#if defined(__APPLE__) && O_CLOEXEC == 0
#define CLOEXEC_REQUESTED(flags) 1
#else
#define CLOEXEC_REQUESTED(flags) (((flags) & O_CLOEXEC) != 0)
#endif

static errno_t
compat_pipe2_impl(int pipefd[2], int flags)
{
	errno_t ec;

	if (flags & ~(O_CLOEXEC | O_NONBLOCK)) {
		return EINVAL;
	}

	int p[2];
	if (pipe(p) < 0) {
		return errno;
	}

	{
		int r;

		if (flags & O_NONBLOCK) {
			if ((r = fcntl(p[0], F_GETFL, 0)) < 0 ||
			    fcntl(p[0], F_SETFL, r | O_NONBLOCK) < 0 ||
			    (r = fcntl(p[1], F_GETFL, 0)) < 0 ||
			    fcntl(p[1], F_SETFL, r | O_NONBLOCK) < 0) {
				ec = errno;
				goto out;
			}
		}

		if (CLOEXEC_REQUESTED(flags)) {
			if ((r = fcntl(p[0], F_GETFD, 0)) < 0 ||
			    fcntl(p[0], F_SETFD, r | FD_CLOEXEC) < 0 ||
			    (r = fcntl(p[1], F_GETFD, 0)) < 0 ||
			    fcntl(p[1], F_SETFD, r | FD_CLOEXEC) < 0) {
				ec = errno;
				goto out;
			}
		}
	}

	pipefd[0] = p[0];
	pipefd[1] = p[1];

	return 0;

out:
	(void)real_close(p[0]);
	(void)real_close(p[1]);
	return ec;
}

int
compat_pipe2(int pipefd[2], int flags)
{
	errno_t ec = compat_pipe2_impl(pipefd, flags);
	if (ec != 0) {
		errno = ec;
		return -1;
	}
	return 0;
}
