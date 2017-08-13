/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_VASPACE_H_
# define _KERNEL_VASPACE_H_

# include <arch/vaspace.h>
# include <kernel/spinlock.h>

struct thread;

/*
** Represents the virtual address space of a thread.
*/
struct vaspace
{
	/* 0xFFFFFFFF */

	/* Kernel space */

	/* 0xCFFFFFFF */

	/* Memory Mapping segment (stacks, dynamic libraries etc.) (goes downward) */
	void *mmapping_start;			 /* MUST BE PAGE ALIGNED */
	size_t mmapping_size;			 /* MUST BE PAGE ALIGNED */

	/* heap segment (goes upward) */
	size_t heap_size;
	void *heap_start;			/* MUST BE PAGE ALIGNED */

	/* Binary goes from 0x0 to this */
	uintptr binary_limit;			/* MUST BE PAGE ALIGNED */

	/* 0x00000000 */

	struct arch_vaspace arch;

	/* Locker to lock the virtual address space */
	struct spinlock lock;

	/* Number of threads sharing this virtual address space */
	uint ref_count;
};

struct vaspace			*setup_boot_vaspace(void);
struct vaspace			*clone_vaspace(struct vaspace *src);
void				init_vaspace(void);
void				free_vaspace(void);
void				free_zombie_thread(struct thread *t);

/* Must be re-implemented on each supported architecture */
struct vaspace			*arch_clone_vaspace(struct vaspace *src);

#endif /* !_KERNEL_VASPACE_H_ */
