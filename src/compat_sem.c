#include "compat_sem.h"

#ifdef COMPAT_SEM_USE_MACH

#include <errno.h>
#include <limits.h>

#include <mach/mach_init.h>
#include <mach/semaphore.h>
#include <mach/sync_policy.h>
#include <mach/task.h>

/*
 * Mach semaphores, used to emulate the subset of POSIX semaphores that
 * epoll-shim needs on Darwin releases without libdispatch (10.5 and
 * older). See compat_sem.h.
 */

static int
compat_sem_fail(kern_return_t kr)
{
	switch (kr) {
	case KERN_RESOURCE_SHORTAGE:
		errno = ENOMEM;
		break;
	case KERN_ABORTED:
		/*
		 * The wait was interrupted (a signal, thread_abort()).
		 * POSIX callers retry on EINTR.
		 */
		errno = EINTR;
		break;
	default:
		errno = EINVAL;
		break;
	}

	return -1;
}

int
compat_sem_init(compat_sem_t *sem, int pshared, unsigned int value)
{
	if (pshared != 0) {
		/* A task local Mach semaphore cannot be shared. */
		errno = ENOSYS;
		return -1;
	}

	if (value > INT_MAX) {
		errno = EINVAL;
		return -1;
	}

	semaphore_t semaphore;
	kern_return_t kr = semaphore_create(mach_task_self(), &semaphore,
	    SYNC_POLICY_FIFO, (int)value);
	if (kr != KERN_SUCCESS) {
		return compat_sem_fail(kr);
	}

	sem->mach_semaphore = semaphore;
	return 0;
}

int
compat_sem_post(compat_sem_t *sem)
{
	kern_return_t kr = semaphore_signal(sem->mach_semaphore);
	if (kr != KERN_SUCCESS) {
		return compat_sem_fail(kr);
	}

	return 0;
}

int
compat_sem_wait(compat_sem_t *sem)
{
	kern_return_t kr = semaphore_wait(sem->mach_semaphore);
	if (kr != KERN_SUCCESS) {
		return compat_sem_fail(kr);
	}

	return 0;
}

int
compat_sem_destroy(compat_sem_t *sem)
{
	kern_return_t kr = semaphore_destroy(mach_task_self(),
	    sem->mach_semaphore);
	if (kr != KERN_SUCCESS) {
		return compat_sem_fail(kr);
	}

	sem->mach_semaphore = MACH_PORT_NULL;
	return 0;
}

#endif
