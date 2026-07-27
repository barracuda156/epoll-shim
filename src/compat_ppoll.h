#ifndef COMPAT_PPOLL_H
#define COMPAT_PPOLL_H

#include <poll.h>
#include <signal.h>
#include <time.h>

int compat_ppoll(struct pollfd fds[], nfds_t nfds,
    struct timespec const *restrict timeout, sigset_t const *restrict sigmask);
#ifdef COMPAT_ENABLE_PPOLL
#define ppoll compat_ppoll
#endif

/* poll() emulated over select(); see the comment in compat_ppoll.c.
 * Only built on platforms that build compat_ppoll.c (i.e. Apple). */
int compat_poll_select(struct pollfd fds[], nfds_t nfds, int timeout_ms);

#endif
