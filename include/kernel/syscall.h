/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_SYSCALL_H_
# define _KERNEL_SYSCALL_H_

enum syscalls_values
{
	UNKNOWN		= 0,
	WRITE,
	READ,
	BRK,
	SBRK,
};

static char const *const syscalls_str[] =
{
	[UNKNOWN]	= "UNKNOWN",
	[WRITE]		= "WRITE",
	[READ]		= "READ",
	[BRK]		= "BRK",
	[SBRK]		= "SBRK",
};

#endif /* !_KERNEL_SYSCALL_H_ */
