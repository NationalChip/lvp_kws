#ifndef __MULTICORE_LOCK_H__
#define __MULTICORE_LOCK_H__

#include <autoconf.h>

typedef enum {
	MULTICORE_LOCK_OTP = (1 << 0),
} MULTICORE_LOCK;

#ifdef CONFIG_MULTICORE_LOCK

int multicore_trylock(MULTICORE_LOCK lock);
int multicore_lock(MULTICORE_LOCK lock);
int multicore_unlock(MULTICORE_LOCK lock);

#else

inline int multicore_trylock(MULTICORE_LOCK lock)
{
	return 0;
}

inline int multicore_lock(MULTICORE_LOCK lock)
{
	return 0;
}

inline int multicore_unlock(MULTICORE_LOCK lock)
{
	return 0;
}
#endif

#endif
