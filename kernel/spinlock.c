/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/spinlock.h>
#include <arch/common_op.h>
#include <stdio.h>

void
init_lock(struct spinlock *lock)
{
	lock->locked = 0;
	lock->depth = 0;
}

bool
holding_lock(struct spinlock *lock)
{
	return (lock->locked && lock->depth);
}

void
acquire_lock(struct spinlock *lock)
{
	lock->depth++;
	while (!atomic_exchange(&lock->locked, 1));
}

void
release_lock(struct spinlock *lock)
{
	assert(holding_lock(lock));
	lock->depth--;
	if (!lock->depth)
		atomic_exchange(&lock->locked, 0);
}
