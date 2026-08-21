#ifndef COMPAT_SEM_H
#define COMPAT_SEM_H

/*
 * Darwin's POSIX unnamed semaphores are stubs that always fail with
 * ENOSYS, so the handful of semaphore operations epoll-shim needs are
 * mapped onto something else.
 *
 * libdispatch first shipped with Mac OS X 10.6; on 10.5 and older
 * <dispatch/dispatch.h> does not exist. For those deployment targets
 * fall back to Mach semaphores, which have been available since well
 * before that.
 */
#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#if defined(MAC_OS_X_VERSION_MIN_REQUIRED) && \
    MAC_OS_X_VERSION_MIN_REQUIRED < 1060
#define COMPAT_SEM_USE_MACH
#endif
#endif

#ifdef COMPAT_SEM_USE_MACH
/*
 * Wraps a Mach semaphore_t (a mach_port_t, i.e. an unsigned int)
 * without pulling the Mach headers into every translation unit this
 * header gets force-included into.
 */
typedef struct {
	unsigned int mach_semaphore;
} compat_sem_t;

int compat_sem_init(compat_sem_t *sem, int pshared, unsigned int value);
int compat_sem_post(compat_sem_t *sem);
int compat_sem_wait(compat_sem_t *sem);
int compat_sem_destroy(compat_sem_t *sem);
#endif

#ifdef COMPAT_ENABLE_SEM
#include <semaphore.h>

#ifdef COMPAT_SEM_USE_MACH

#define sem_t compat_sem_t

#define sem_init(sem, pshared, value) \
	compat_sem_init((sem), (pshared), (value))
#define sem_post(sem) compat_sem_post(sem)
#define sem_wait(sem) compat_sem_wait(sem)
#define sem_destroy(sem) compat_sem_destroy(sem)

#else

#include <dispatch/dispatch.h>

#define sem_t dispatch_semaphore_t

#define sem_init(sem, pshared, value) \
	(*(sem) = dispatch_semaphore_create(value))
#define sem_post(sem) dispatch_semaphore_signal(*(sem))
#define sem_wait(sem) dispatch_semaphore_wait(*(sem), DISPATCH_TIME_FOREVER)
#define sem_destroy(sem) dispatch_release(*(sem))

#endif
#endif

#endif
