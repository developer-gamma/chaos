/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_VMM_H_
# define _KERNEL_VMM_H_

# include <kernel/pmm.h>
# include <kernel/spinlock.h>
# include <arch/vaspace.h>
# include <chaosdef.h>
# include <chaoserr.h>

typedef void 		*virt_addr_t;

/*
** Represents the virtual address space of a process.
*/
struct vaspace
{
	/* 0xFFFFFFFF */

	/* Kernel space */

	/* 0xCFFFFFFF */

	/* Memory Mapping segment (stacks, dynamic libraries etc.) (goes downward) */
	uintptr mmapping_start;
	size_t mmapping_size;
	uintptr mmapping_pos;

	/* heap segment (goes upward) */
	uintptr heap_pos;
	size_t heap_size;
	void *heap_start;
	struct spinlock heap_lock;

	uintptr binary_limit; /* Binary goes from 0x0 to this */

	/* 0x00000000 */
	struct arch_vaspace arch;
};

status_t		map_virt_to_phys();
status_t		map_page(virt_addr_t va);
virt_addr_t		mmap(virt_addr_t va, size_t size);
void			munmap(virt_addr_t va, size_t size);
status_t		kbrk(virt_addr_t new_brk);
virt_addr_t		ksbrk(intptr);

#endif /* !_KERNEL_VMM_H_ */
