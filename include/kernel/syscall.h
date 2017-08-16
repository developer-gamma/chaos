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

# include <chaosdef.h>
# include <kernel/thread.h>

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
	EXECVE		= 9,
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
	[EXECVE]	= "EXECVE",
};

int			sys_open(char const *path);
int			sys_write(int fd, char const *, size_t);
int			sys_read(int fd, char *, size_t);
pid_t			sys_fork(void);

#endif /* !_KERNEL_SYSCALL_H_ */
