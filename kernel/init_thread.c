/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/thread.h>
#include <kernel/alloc.h>
#include <lib/interrupts.h>
#include <stdio.h>

/*
** The first thread launched by the kernel when it has finish booting.
** TODO make this user space
*/
void
init_routine(void)
{
	while (42) {
		assert(are_int_enabled());
	}
}

