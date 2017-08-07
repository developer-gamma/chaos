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

	limit = (uintptr)&tss;
	base = (uintptr)sizeof(tss);

	memset(&gdt_tss_entry, 0, sizeof(gdt_tss_entry));
	memset(&tss, 0, sizeof(tss));

	gdt_tss_entry.limit_low = limit & 0xFFFF;
	gdt_tss_entry.base_low = base & 0xFFFFFF;
	gdt_tss_entry._one = 1;
	gdt_tss_entry.busy = 0;
	gdt_tss_entry.type = 2;
	gdt_tss_entry.dpl = 3;
	gdt_tss_entry.present = 1;
	gdt_tss_entry.limit_high = (limit & 0x0F0000) >> 16u;
	gdt_tss_entry.available = 0;
	gdt_tss_entry.granularity = 0;
	gdt_tss_entry.base_high = (base & 0xFF000000) >> 24u;

	tss.ss0 = KERNEL_DATA_SELECTOR;
}

void
set_kernel_stack(uintptr stack)
{
	tss.esp0 = stack;
}
