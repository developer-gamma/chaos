/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <arch/x86/tss.h>
#include <arch/x86/x86.h>
#include <string.h>

struct tss tss;

extern struct gdt_tss_entry gdt_tss_entry;

void
tss_setup(void)
{
	uintptr limit;
	uintptr base;

	base = (uintptr)&tss;
	limit = (uintptr)sizeof(tss);

	gdt_tss_entry.limit_low = limit & 0xFFFF;
	gdt_tss_entry.base_low = base & 0xFFFFFF;
	gdt_tss_entry.limit_high = (limit & 0x0F0000) >> 16u;
	gdt_tss_entry.base_high = (base & 0xFF000000) >> 24u;

	memset(&tss, 0, sizeof(tss));
	tss.esp0 = 0;
	tss.ss0 = KERNEL_DATA_SELECTOR;
	tss.ss1 = 0;
	tss.ss2 = 0;
	tss.eflags = 0x00003002;
}

void
set_kernel_stack(uintptr stack)
{
	tss.esp0 = stack;
}
