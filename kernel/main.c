/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/unit_tests.h>
#include <kernel/init.h>
#include <kernel/thread.h>

/*
** Common entry point of the kernel.
*/
void
kernel_main(void)
{
	/* Put us in the boot thread */
	thread_early_init();

	/* Ensure libc is working correctly */
	trigger_unit_tests(UNIT_TEST_LEVEL_LIBC);

	/* Go through all init levels */
	kernel_init_level(CHAOS_INIT_LEVEL_EARLIEST, CHAOS_INIT_LEVEL_LATEST);

	/* Build the init process, print hello message, enable multithreading and let's go! */
	thread_init();

	panic("Leaving kernel main");
}
