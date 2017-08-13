/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/vaspace.h>
#include <kernel/thread.h>
#include <kernel/kalloc.h>
#include <string.h>

struct vaspace boot_vaspace; /* virtual address space of boot and init. */

/*
** Set up a new virtual address space
**
** binary_limit should have already been set.
*/
void
init_vaspace(void)
{
	struct vaspace *vaspace;

	vaspace = get_current_thread()->vaspace;
	assert_neq(vaspace->binary_limit, 0);

	arch_init_vaspace();

	vaspace->mmapping_start = KERNEL_VIRTUAL_BASE - PAGE_SIZE;
	vaspace->mmapping_size = 0;

	vaspace->heap_start = (void *)ALIGN(vaspace->binary_limit + PAGE_SIZE, PAGE_SIZE);
	vaspace->heap_size = 0;

	/* Allocate the first heap page or the ubrk algorithm will not work. */
	assert_neq(mmap(vaspace->heap_start, PAGE_SIZE, MMAP_USER | MMAP_WRITE), NULL);
}

/*
** Clone the given virtual space into a new one.
** Returns NULL if the clone failed.
*/
struct vaspace *
clone_vaspace(struct vaspace *src)
{
	return (arch_clone_vaspace(src));
}

/*
** Cleans up the userspace memory of the current virtual address space.
** Doesn't care about kernel memory (like the kernel stack)
*/
void
free_vaspace(void)
{
	struct vaspace *vaspace;

	vaspace = get_current_thread()->vaspace;

	/* Clean up memory space */
	munmap(NULL, vaspace->binary_limit);
	munmap(vaspace->mmapping_start - vaspace->mmapping_size + PAGE_SIZE, vaspace->mmapping_size);
	munmap(vaspace->heap_start, ALIGN(vaspace->heap_size, PAGE_SIZE) + PAGE_SIZE);

	arch_free_vaspace();
}

/*
** Called by thread_waitpid() when waiting for a zombie thread.
** Used to finish cleaning up this thread, by freeing it's kernel memory.
*/
void
free_zombie_thread(struct thread *t)
{
	arch_free_zombie_thread(t);

	if (t->vaspace->ref_count == 0) {
		kfree(t->vaspace);
	}
}

/*
** Called to default-initialize the default virtual address space.
*/
struct vaspace *
setup_boot_vaspace(void)
{
	memset(&boot_vaspace, 0, sizeof(boot_vaspace));

	init_lock(&boot_vaspace.lock);
	boot_vaspace.binary_limit = PAGE_SIZE; /* boot doesn't have a binary */
	boot_vaspace.ref_count = 0;

	return (&boot_vaspace);
}
