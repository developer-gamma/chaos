/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/syscall.h>
#include <stdio.h>

/*
** Do the write system call.
*/
int
sys_write(int fd __unused, char const *buff, size_t size)
{
	return (putsn(buff, size));
}

extern char	keyboard_next_input(void);

/*
** Do the read system call.
*/
int
sys_read(int fd __unused, char *buff, size_t size)
{
	char c;

	if (size)
	{
		c = keyboard_next_input();
		if (c != '\0') {
			buff[0] = c;
			return (1);
		}
	}
	return (0);
}

/*
** Do the fork system call.
** Fork the current process and returns the new process's pid,
** or -1 if the operation failed.
*/
pid_t
sys_fork(void)
{
	struct thread *new;

	new = thread_fork();
	if (new) {
		return (new->pid);
	}
	return (-1);
}
