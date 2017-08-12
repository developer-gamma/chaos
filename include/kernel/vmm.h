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

typedef void 		*virt_addr_t;

# include <kernel/pmm.h>
# include <kernel/spinlock.h>
# include <arch/vaspace.h>
# include <chaosdef.h>
# include <chaoserr.h>

/*
** Represents the virtual address space of a process.
*/
struct vaspace
{
	/* 0xFFFFFFFF */

	/* Kernel space */

	/* 0xCFFFFFFF */

	/* Memory Mapping segment (stacks, dynamic libraries etc.) (goes downward) */
	void *mmapping_start;
	size_t mmapping_size;

	/* heap segment (goes upward) */
	size_t heap_size;
	void *heap_start;

	uintptr binary_limit; /* Binary goes from 0x0 to this */

	/* 0x00000000 */

	struct arch_vaspace arch;

	/* Locker to lock the virtual address space */
	struct spinlock lock;

	/* Number of threads sharing this virtual address space */
	uint ref_count;
};

/* Arch-indepentant flags for pages */
# define MMAP_DEFAULT		0b00000000	/* Kernel space, read only */
# define MMAP_USER		0b00000001	/* Page belongs to user space */
# define MMAP_WRITE		0b00000010	/* Page is writtable */

/* The integer type corresponding to the flags above */
typedef uintptr			mmap_flags_t;

/*
** Used for debugging purposes. Dumps the memory state
*/
void			arch_dump_mem(void);

/*
** Used for debugging purposes. Returns true if the given virtual address is allocated.
*/
bool			arch_is_allocated(virt_addr_t);

/*
** Maps a physical address to a virtual one.
*/
status_t		arch_map_virt_to_phys(virt_addr_t, phys_addr_t, mmap_flags_t);

/*
** Maps the given virtual address to a random physical addresses.
*/
status_t		arch_map_page(virt_addr_t va, mmap_flags_t);

/*
** Unmaps a virtual address.
*/
void			arch_munmap_va(virt_addr_t va);

/*
** Initialises the arch-dependent stuff of virtual memory management.
*/
void			arch_vmm_init(void);

virt_addr_t		mmap(virt_addr_t va, size_t size, mmap_flags_t);
void			munmap(virt_addr_t va, size_t size);
status_t		kbrk(virt_addr_t new_brk);
virt_addr_t		ksbrk(intptr);
status_t		ubrk(virt_addr_t new_brk);
virt_addr_t		usbrk(intptr inc);

# define LOCK_VASPACE(state)	LOCK(&get_current_thread()->vaspace->lock, state)
# define RELEASE_VASPACE(state)	RELEASE(&get_current_thread()->vaspace->lock, state)

#endif /* !_KERNEL_VMM_H_ */
