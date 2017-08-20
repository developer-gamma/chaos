/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/unit-tests.h>
#include <kernel/kalloc.h>
#include <kernel/multiboot.h>
#include <arch/x86/vmm.h>
#include <arch/x86/asm.h>
#include <stdio.h>
#include <string.h>

status_t
arch_map_virt_to_phys(virt_addr_t va, phys_addr_t pa, mmap_flags_t flags)
{
	struct pagedir_entry *pde;
	struct pagetable_entry *pte;
	struct page_table *pt;
	bool allocated_pde;

	allocated_pde = false;
	assert(IS_PAGE_ALIGNED(va));
	assert(IS_PAGE_ALIGNED(pa));
	pde = GET_PAGE_DIRECTORY->entries + GET_PD_IDX(va);
	pt = GET_PAGE_TABLE(GET_PD_IDX(va));
	if (pde->present == false)
	{
		pde->value = alloc_frame();
		if (pde->value == NULL_FRAME) {
			pde->value = 0;
			return (ERR_NO_MEMORY);
		}
		pde->present = true;
		pde->rw = true;
		pde->user = (bool)(flags & MMAP_USER);
		invlpg(pt);
		memset(pt, 0, PAGE_SIZE);
		allocated_pde = true;
	}
	pte = pt->entries + GET_PT_IDX(va);
	/* Return NULL if the page is already mapped */
	if (pte->present)
	{
		if (allocated_pde) {
			munmap(pt, PAGE_SIZE);
		}
		return (ERR_ALREADY_MAPPED);
	}
	pte->value = pa;
	pte->present = true;
	pte->rw = (bool)(flags & MMAP_WRITE);
	pte->user = (bool)(flags & MMAP_USER);
	pte->accessed = false;
	pte->dirty = 0;
	invlpg(va);
	return (OK);
}

status_t
arch_map_page(virt_addr_t va, mmap_flags_t flags)
{
	phys_addr_t pa;
	status_t s;

	pa = alloc_frame();
	if (pa != NULL_FRAME)
	{
		s = arch_map_virt_to_phys(va, pa, flags);
		if (s == OK) {
			/* Clean the new page with random shitty values */
			memset(va, 42, PAGE_SIZE);
			return (OK);
		}
		free_frame(pa);
		return (s);
	}
	return (ERR_NO_MEMORY);
}

void
arch_munmap_va(virt_addr_t va)
{
	struct pagedir_entry *pde;
	struct pagetable_entry *pte;

	pde = GET_PAGE_DIRECTORY->entries + GET_PD_IDX(va);
	pte = GET_PAGE_TABLE(GET_PD_IDX(va))->entries + GET_PT_IDX(va);
	if (pde->present && pte->present)
	{
		free_frame(pte->frame << 12u);
		pte->value = 0;
		invlpg(va);
	}
}

/*
** Gets the physical address behind the given virtual address.
** Returns NULL_FRAME if the given virtual address is not allocated.
*/
phys_addr_t
get_paddr(virt_addr_t va)
{
	struct pagedir_entry *pde;
	struct pagetable_entry *pte;

	pde = GET_PAGE_DIRECTORY->entries + GET_PD_IDX(va);
	pte = GET_PAGE_TABLE(GET_PD_IDX(va))->entries + GET_PT_IDX(va);
	if (pde->present && pte->present) {
		return (pte->frame << 12u);
	}
	return (NULL_FRAME);
}

/*
** Sets the frame for a given virtual page.
** The given virtual page must already be allocated, and the actual frame will NOT
** be free.
** The old frame is returned, or NULL_FRAME if the virtual page wasn't allocated.
*/
phys_addr_t
set_paddr(virt_addr_t va, phys_addr_t pa)
{
	phys_addr_t old;
	struct pagedir_entry *pde;
	struct pagetable_entry *pte;

	pde = GET_PAGE_DIRECTORY->entries + GET_PD_IDX(va);
	pte = GET_PAGE_TABLE(GET_PD_IDX(va))->entries + GET_PT_IDX(va);
	if (pde->present && pte->present) {
		old = pte->frame << 12u;
		pte->frame = pa >> 12u;
		invlpg(va);
		return (old);
	}
	return (NULL_FRAME);
}

/*
** Marks the initrd as allocated & accessible.
*/
void
arch_vmm_mark_initrd(void)
{
	void *initrd;
	size_t i;

	/* Allocate the initrd */
	if (multiboot_infos.initrd.present) {

		/* Page alignment tricks (TODO add a page-aligned alocator */
		initrd = kalloc(multiboot_infos.initrd.size + 2 * PAGE_SIZE);
		assert_neq(initrd, NULL);

		/* We don't give a shit about losing the pointer, we will never kfree() it.*/
		initrd = (void *)ALIGN((uintptr)initrd, PAGE_SIZE);

		i = 0;
		while (i < multiboot_infos.initrd.size) {
			free_frame(get_paddr(initrd + i));
			set_paddr(initrd + i, multiboot_infos.initrd.pstart + i);
			i += PAGE_SIZE;
		}
		multiboot_infos.initrd.vstart = initrd;
		multiboot_infos.initrd.vend = initrd + multiboot_infos.initrd.size;
	}
}

void
arch_vmm_init(void)
{
	size_t i;
	size_t j;
	phys_addr_t pa;
	status_t s;

	/* Remove the top part of identity mapping */
	i = GET_PD_IDX(KERNEL_VIRTUAL_END);
	j = GET_PT_IDX(KERNEL_VIRTUAL_END) + 1;
	while (j < 1024)
	{
		pa = GET_PAGE_TABLE(i)->entries[j].frame << 12u;
		assert(pa > KERNEL_PHYSICAL_END);

		GET_PAGE_TABLE(i)->entries[j].value = 0;
		++j;
	}

	/* Allocates all kernel page tables, so that each future processes share kernel memory. */
	i = GET_PD_IDX(KERNEL_VIRTUAL_BASE);
	while (i < 1023)
	{
		s = arch_map_page(GET_PAGE_TABLE(i), MMAP_WRITE);
		assert(s == OK || s == ERR_ALREADY_MAPPED);
		++i;
	}
}

/*
** Unit tests function
*/

/*
** Print the virtual memory state.
** Only used for debugging.
*/
__unused void
arch_dump_mem(void)
{
	uint i;
	uint j;
	struct page_dir *pd;
	struct page_table *pt;

	i = 0;
	pd = GET_PAGE_DIRECTORY;
	while (i < 1024u)
	{
		if (pd->entries[i].present)
		{
			printf("[%4u] [%#p -> %#p] %s %c %c %c %c %c\n",
				i,
				i << 22u,
				pd->entries[i].frame << 12ul,
				&"RO\0RW"[pd->entries[i].rw * 3],
				"ku"[pd->entries[i].user],
				"-w"[pd->entries[i].wtrough],
				"-d"[pd->entries[i].cache],
				"-a"[pd->entries[i].accessed],
				"-H"[pd->entries[i].size]);
			if (pd->entries[i].size == false && i != 1023)
			{
				pt = GET_PAGE_TABLE(i);
				j = 0;
				while (j < 1024u)
				{
					if (pt->entries[j].present)
					{
						printf("\t[%4u] [%#p -> %#p] %s %c %c %c %c %c\n",
							j,
							(i << 22u) | (j << 12u),
							pt->entries[j].frame << 12ul,
							&"RO\0RW"[pt->entries[j].rw * 3],
							"ku"[pt->entries[j].user],
							"-w"[pt->entries[j].wtrough],
							"-d"[pt->entries[j].cache],
							"-a"[pt->entries[j].accessed],
							"-d"[pt->entries[j].dirty]);
					}
					++j;
				}
			}
		}
		++i;
	}
}

__unused bool
arch_is_allocated(virt_addr_t va)
{
	struct pagedir_entry *pde;
	struct pagetable_entry *pte;

	assert(IS_PAGE_ALIGNED(va));
	pde = GET_PAGE_DIRECTORY->entries + GET_PD_IDX(va);
	pte = GET_PAGE_TABLE(GET_PD_IDX(va))->entries + GET_PT_IDX(va);
	return (pde->present && pte->present);
}

static void
vmm_test(void)
{
	virt_addr_t brk;

	/* Defined in kernel/kalloc.c */
	extern virt_addr_t kernel_heap_start;
	extern size_t kernel_heap_size;

	assert(!arch_is_allocated((virt_addr_t)0xDEADB000));
	assert_eq(arch_map_page((virt_addr_t)0xDEADB000, MMAP_DEFAULT), OK);
	assert(arch_is_allocated((virt_addr_t)0xDEADB000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADA000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADC000));
	assert_eq(*(char *)0xDEADB000, 42);
	*(char *)0xDEADB000 = 43;
	assert_eq(*(char *)0xDEADB000, 43);
	assert_eq(arch_map_page((virt_addr_t)0xDEADB000, MMAP_DEFAULT), ERR_ALREADY_MAPPED);

	/* munmap */
	munmap((virt_addr_t)0xDEADB000, PAGE_SIZE);
	assert(!arch_is_allocated((virt_addr_t)0xDEADB000));

	/* mmap */
	assert_eq(mmap((virt_addr_t)0xDEADA000, 3 * PAGE_SIZE, MMAP_DEFAULT), (virt_addr_t)0xDEADA000);
	assert(!arch_is_allocated((virt_addr_t)0xDEAD9000));
	assert(arch_is_allocated((virt_addr_t)0xDEADA000));
	assert(arch_is_allocated((virt_addr_t)0xDEADB000));
	assert(arch_is_allocated((virt_addr_t)0xDEADC000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADD000));

	/* overlapping mmap */
	assert_eq(mmap((virt_addr_t)0xDEAD8000, 5 * PAGE_SIZE, MMAP_DEFAULT), NULL);
	assert(!arch_is_allocated((virt_addr_t)0xDEAD7000));
	assert(!arch_is_allocated((virt_addr_t)0xDEAD8000));
	assert(!arch_is_allocated((virt_addr_t)0xDEAD9000));
	assert(arch_is_allocated((virt_addr_t)0xDEADA000));
	assert(arch_is_allocated((virt_addr_t)0xDEADB000));
	assert(arch_is_allocated((virt_addr_t)0xDEADC000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADD000));

	/* sized munmap */
	munmap((virt_addr_t)0xDEADA000, 3 * PAGE_SIZE);
	assert(!arch_is_allocated((virt_addr_t)0xDEAD9000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADA000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADB000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADC000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADD000));

	/* tiny ksbrk tests */
	brk = ksbrk(0);
	assert(arch_is_allocated(brk));
	assert_eq(brk, kernel_heap_start);
	assert_eq(kernel_heap_size, 0);
	ksbrk(0);
	assert(arch_is_allocated(brk));
	assert_eq(kernel_heap_size, 0);
	assert_eq(ksbrk(-1), (virt_addr_t)-1u);
	assert_eq(ksbrk(0), brk);
	ksbrk(1);
	assert(arch_is_allocated(brk));
	assert_eq(ksbrk(0), brk + 1);
	assert_eq(kernel_heap_size, 1);
	ksbrk(-1);
	assert(arch_is_allocated(brk));
	assert_eq(kernel_heap_size, 0);
	assert_eq(ksbrk(0), brk);

	/* medium ksbrk tests */
	assert_eq(ksbrk(PAGE_SIZE), brk);
	assert_eq(kernel_heap_size, PAGE_SIZE);
	assert(arch_is_allocated(brk));
	assert(arch_is_allocated(brk + PAGE_SIZE));
	assert(arch_is_allocated(ksbrk(0)));
	assert_eq(ksbrk(-PAGE_SIZE), brk + PAGE_SIZE);
	assert_eq(kernel_heap_size, 0);
	assert(arch_is_allocated(brk));
	assert(!arch_is_allocated(brk + PAGE_SIZE));
	assert(arch_is_allocated(ksbrk(0)));
	assert_eq(ksbrk(0), brk);

	/* huge ksbrk tests */
	assert_eq(ksbrk(10 * PAGE_SIZE), brk);
	assert(arch_is_allocated(brk));
	assert(arch_is_allocated(brk + 5 * PAGE_SIZE));
	assert(arch_is_allocated(brk + 10 * PAGE_SIZE));
	assert(!arch_is_allocated(brk + 11 * PAGE_SIZE));
	assert_eq(ksbrk(0), brk + 10 * PAGE_SIZE);
	assert_eq(ksbrk(-5 * PAGE_SIZE), brk + 10 * PAGE_SIZE);
	assert_eq(ksbrk(0), brk + 5 * PAGE_SIZE);
	assert_eq(kernel_heap_size, 5 * PAGE_SIZE);
	assert(arch_is_allocated(brk + 5 * PAGE_SIZE));
	assert(!arch_is_allocated(brk + 6 * PAGE_SIZE));
	assert_eq(ksbrk(-5 * PAGE_SIZE), brk + 5 * PAGE_SIZE);
	assert_eq(ksbrk(0), brk);
	assert_eq(kernel_heap_size, 0);
	assert(!arch_is_allocated(brk + 5 * PAGE_SIZE));
	assert(arch_is_allocated(brk));

	/* tricky kbrk tests */
	assert_eq(kbrk(NULL), ERR_INVALID_ARGS);
	assert_eq(ksbrk(0), brk);
	assert_eq(ksbrk(-100000), (virt_addr_t)-1u);
}

NEW_UNIT_TEST(vmm, &vmm_test, UNIT_TEST_LEVEL_VMM);
