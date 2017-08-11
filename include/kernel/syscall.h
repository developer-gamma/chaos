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
	EXIT		= 1,
	FORK		= 2,
	WRITE		= 3,
	READ		= 4,
	BRK		= 5,
	SBRK		= 6,
	GETPID		= 7,
	WAITPID		= 8,
};

static char const *const syscalls_str[] =
{
	[UNKNOWN]	= "UNKNOWN",
	[EXIT]		= "EXIT",
	[FORK]		= "FORK",
	[WRITE]		= "WRITE",
	[READ]		= "READ",
	[BRK]		= "BRK",
	[SBRK]		= "SBRK",
	[GETPID]	= "GETPID",
	[WAITPID]	= "WAITPID",
};

#endif /* !_KERNEL_SYSCALL_H_ */
