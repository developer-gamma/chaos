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
	UNKNOWN		= 0x0,
	EXIT		= 0x1,
	FORK		= 0x2,
	WRITE		= 0x3,
	READ		= 0x4,
	BRK		= 0x5,
	SBRK		= 0x6,
	GETPID		= 0x7,
	WAITPID		= 0x8,
	EXECVE		= 0x9,
	GETCWD		= 0xA,
	CHDIR		= 0xB,
	GETDENTS	= 0xC,
	OPEN		= 0xD,
	CLOSE		= 0xE,
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
	[GETCWD]	= "GETCWD",
	[CHDIR]		= "CHDIR",
	[GETDENTS]	= "GETDENTS",
	[OPEN]		= "OPEN",
	[CLOSE]		= "CLOSE",
};

int			sys_open(char const *path);
int			sys_close(int);
int			sys_write(int fd, char const *, size_t);
int			sys_read(int fd, char *, size_t);
pid_t			sys_fork(void);

#endif /* !_KERNEL_SYSCALL_H_ */
