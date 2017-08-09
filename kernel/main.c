/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

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

	/* Super super early hooks goes first. */
	kernel_init_level(CHAOS_INIT_LEVEL_EARLIEST, CHAOS_INIT_LEVEL_ARCH_EARLY - 1);

	/* Then goes early arch and platform stuff */
	kernel_init_level(CHAOS_INIT_LEVEL_ARCH_EARLY, CHAOS_INIT_LEVEL_PLATFORM_EARLY - 1);
	kernel_init_level(CHAOS_INIT_LEVEL_PLATFORM_EARLY, CHAOS_INIT_LEVEL_MULTIBOOT - 1);

	/* Load the multiboot structure */
	kernel_init_level(CHAOS_INIT_LEVEL_MULTIBOOT, CHAOS_INIT_LEVEL_PMM - 1);

	/* Initialize the memory management */
	kernel_init_level(CHAOS_INIT_LEVEL_PMM, CHAOS_INIT_LEVEL_VMM - 1);
	kernel_init_level(CHAOS_INIT_LEVEL_VMM, CHAOS_INIT_LEVEL_ARCH - 1);

	/* It's time to initialize arch, platform and drivers */
	kernel_init_level(CHAOS_INIT_LEVEL_ARCH, CHAOS_INIT_LEVEL_PLATFORM - 1);
	kernel_init_level(CHAOS_INIT_LEVEL_PLATFORM, CHAOS_INIT_LEVEL_LATEST);

	/* Build the init process, print hello message, enable multithreading and let's go! */
	thread_init();

	panic("Leaving kernel main");
}
