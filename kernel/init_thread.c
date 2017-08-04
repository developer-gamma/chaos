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

static void
dumper_routine(void)
{
	thread_dump();
}

static void
init2_routine(void)
{
	size_t i;
	struct thread *t;

	i = 0;
	while (42)
	{
		assert(are_int_enabled());
		/*
		** Horrible way (but still not as much as you) to delay the output.
		** This is of course here only for debugging purposes,
		** so please stop yelling everywhere. Thanks.
		** FIXME
		*/
		if (unlikely(i >= 1000000)) {
			t = thread_create("dumper", &dumper_routine, DEFAULT_STACK_SIZE);
			assert_neq(t, NULL);
			thread_resume(t);
			i = 0;
		}
		++i;
	}
}

/*
** The first thread launched by the kernel when it has finish booting.
** TODO make this user space
*/
void
init_routine(void)
{
	struct thread *t;

	t = thread_create("init2", &init2_routine, DEFAULT_STACK_SIZE);
	assert(are_int_enabled());
	assert_neq(t, NULL);
	assert(are_int_enabled());
	thread_resume(t);
	assert(are_int_enabled());
	while (42) {
		assert(are_int_enabled());
	}
}

