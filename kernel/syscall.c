/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/thread.h>
#include <kernel/fs.h>
#include <stdio.h>

/*
** Does the open system call.
*/
int
sys_open(char const *path)
{
	struct filehandler *filehandler;
	status_t err;
	int fd;

	fd = thread_reserve_fd();
	if (fd < 0) {
		return (-1);
	}
	err = fs_open(path, &filehandler);
	if (err) {
		thread_free_fd(fd);
		return (-1);
	}
	thread_set_fd_handler(fd, filehandler);
	return (fd);
}

/*
** Does the close system call.
*/
int
sys_close(int fd)
{
	struct filehandler *filehandler;

	filehandler = thread_get_fd_handler(fd);
	if (filehandler == NULL) {
		return (-1);
	}
	fs_close(filehandler);
	thread_free_fd(fd);
	return (0);
}

/*
** Does the write system call.
*/
int
sys_write(int fd __unused, char const *buff, size_t size)
{
	return (putsn(buff, size));
}

extern char	keyboard_next_input(void);

/*
** Does the read system call.
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
** Does the fork system call.
** Forks the current process and returns the new process's pid,
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
