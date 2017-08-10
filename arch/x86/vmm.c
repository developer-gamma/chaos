/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/unit-tests.h>
#include <kernel/thread.h>
#include <arch/x86/asm.h>
#include <arch/x86/vmm.h>
#include <stdio.h>
#include <string.h>

status_t
arch_map_virt_to_phys(virt_addr_t va, phys_addr_t pa)
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
		invlpg(pt);
		memset(pt, 0, PAGE_SIZE);
		allocated_pde = true;
	}
	pte = pt->entries + GET_PT_IDX(va);
	/* Return NULL if the page is already mapped */
	if (pte->present)
	{
		if (allocated_pde) {
			arch_munmap(pt, PAGE_SIZE);
		}
		return (ERR_ALREADY_MAPPED);
	}
	pte->value = pa;
	pte->present = true;
	pte->rw = true;
	pte->accessed = false;
	pte->dirty = 0;
	invlpg(va);

	/* Clean the new page with random shitty values */
	memset((void *)((uintptr)va & ~PAGE_SIZE_MASK), 42, PAGE_SIZE);
	return (OK);
}

status_t
arch_map_page(virt_addr_t va)
{
	phys_addr_t pa;
	status_t s;

	pa = alloc_frame();
	if (pa != NULL_FRAME)
	{
		s = arch_map_virt_to_phys(va, pa);
		if (s == OK) {
			return (OK);
		}
		free_frame(pa);
		return (s);
	}
	return (ERR_NO_MEMORY);
}

void
arch_munmap(virt_addr_t va, size_t size)
{
	virt_addr_t ori_va;
	struct pagedir_entry *pde;
	struct pagetable_entry *pte;

	assert(IS_PAGE_ALIGNED(va));
	assert(IS_PAGE_ALIGNED(size));
	ori_va = va;
	while (va < ori_va + size)
	{
		pde = GET_PAGE_DIRECTORY->entries + GET_PD_IDX(va);
		pte = GET_PAGE_TABLE(GET_PD_IDX(va))->entries + GET_PT_IDX(va);
		if (pde->present && pte->present)
		{
			free_frame(pte->frame << 12u);
			pte->value = 0;
			invlpg(va);
		}
		va += PAGE_SIZE;
	}
}

void
arch_vmm_init(void)
{
	size_t i;
	status_t s;

	/* Defined in kernel/thread.c */
	extern struct vaspace default_vaspace;

	i = GET_PD_IDX(KERNEL_VIRTUAL_BASE);
	default_vaspace.arch.pagedir = get_cr3();

	/* Allocates all kernel page tables, so that each future processes share kernel memory. */
	while (i < 1023)
	{
		s = arch_map_page(GET_PAGE_TABLE(i));
		assert(s == OK || s == ERR_ALREADY_MAPPED);
		++i;
	}
}

/* Unit tests functions */

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

bool
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
	virt_addr_t mmap1;
	virt_addr_t mmap2;

	/* Defined in kernel/kalloc.c */
	extern virt_addr_t kernel_heap_start;
	extern size_t kernel_heap_size;

	assert(!arch_is_allocated((virt_addr_t)0xDEADB000));
	assert_eq(arch_map_page((virt_addr_t)0xDEADB000), OK);
	assert(arch_is_allocated((virt_addr_t)0xDEADB000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADA000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADC000));
	assert_eq(*(char *)0xDEADB000, 42);
	*(char *)0xDEADB000 = 43;
	assert_eq(*(char *)0xDEADB000, 43);
	assert_eq(arch_map_page((virt_addr_t)0xDEADB000), ERR_ALREADY_MAPPED);

	/* munmap */
	arch_munmap((virt_addr_t)0xDEADB000, PAGE_SIZE);
	assert(!arch_is_allocated((virt_addr_t)0xDEADB000));

	/* mmap */
	assert_eq(mmap((virt_addr_t)0xDEADA000, 3 * PAGE_SIZE), (virt_addr_t)0xDEADA000);
	assert(!arch_is_allocated((virt_addr_t)0xDEAD9000));
	assert(arch_is_allocated((virt_addr_t)0xDEADA000));
	assert(arch_is_allocated((virt_addr_t)0xDEADB000));
	assert(arch_is_allocated((virt_addr_t)0xDEADC000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADD000));

	/* overlapping mmap */
	assert_eq(mmap((virt_addr_t)0xDEAD8000, 5 * PAGE_SIZE), NULL);
	assert(!arch_is_allocated((virt_addr_t)0xDEAD7000));
	assert(!arch_is_allocated((virt_addr_t)0xDEAD8000));
	assert(!arch_is_allocated((virt_addr_t)0xDEAD9000));
	assert(arch_is_allocated((virt_addr_t)0xDEADA000));
	assert(arch_is_allocated((virt_addr_t)0xDEADB000));
	assert(arch_is_allocated((virt_addr_t)0xDEADC000));
	assert(!arch_is_allocated((virt_addr_t)0xDEADD000));

	/* sized munmap */
	arch_munmap((virt_addr_t)0xDEADA000, 3 * PAGE_SIZE);
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

	/* NULL mmap tests */
	mmap1 = mmap(NULL, PAGE_SIZE);
	assert_neq(mmap1, NULL);
	assert(arch_is_allocated(mmap1));
	assert(IS_PAGE_ALIGNED(mmap1));
	mmap2 = mmap(NULL, 10 * PAGE_SIZE);
	assert_neq(mmap2, NULL);
	assert(arch_is_allocated(mmap1));
	assert(arch_is_allocated(mmap2));
	assert(arch_is_allocated(mmap2 + 9 * PAGE_SIZE));
	assert(arch_is_allocated(mmap2 + 10 * PAGE_SIZE));
	assert_eq(get_current_thread()->vaspace->mmapping_size, 11 * PAGE_SIZE);
	arch_munmap(mmap2, 11 * PAGE_SIZE);
	get_current_thread()->vaspace->mmapping_size = 0;
}

NEW_UNIT_TEST(vmm, &vmm_test, UNIT_TEST_LEVEL_VMM);
