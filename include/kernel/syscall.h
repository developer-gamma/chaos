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
# include <unistd.h>

enum syscalls_values
{
	UNKNOWN		= 0x00,
	EXIT		= 0x01,
	FORK		= 0x02,
	WRITE		= 0x03,
	READ		= 0x04,
	BRK		= 0x05,
	SBRK		= 0x06,
	GETPID		= 0x07,
	WAITPID		= 0x08,
	EXECVE		= 0x09,
	GETCWD		= 0x0A,
	CHDIR		= 0x0B,
	GETDENTS	= 0x0C,
	OPEN		= 0x0D,
	CLOSE		= 0x0E,
	OPENDIR		= 0x0F,
	CLOSEDIR	= 0xA0,
	READDIR		= 0xA1,
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
	[OPENDIR]	= "OPENDIR",
	[CLOSEDIR]	= "CLOSEDIR",
	[READDIR]	= "READDIR",
};

int			sys_open(char const *path);
int			sys_close(int);
int			sys_write(int fd, char const *, size_t);
int			sys_read(int fd, char *, size_t);
pid_t			sys_fork(void);
int			sys_opendir(char const *path);
int			sys_closedir(int);
int			sys_readdir(int fd, struct dirent *dirent);
int			sys_execve(char const *name, int (*main)(), char const *args[]);

#endif /* !_KERNEL_SYSCALL_H_ */
