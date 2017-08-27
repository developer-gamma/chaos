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
#include <kernel/kalloc.h>
#include <stdio.h>
#include <string.h>

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
** Does the opendir system call.
*/
int
sys_opendir(char const *path)
{
	struct dirhandler *dirhandler;
	status_t err;
	int fd;

	fd = thread_reserve_fd();
	if (fd < 0) {
		return (-1);
	}
	err = fs_opendir(path, &dirhandler);
	if (err) {
		thread_free_fd(fd);
		return (-1);
	}
	thread_set_fd_handler(fd, dirhandler);
	return (fd);
}

/*
** Does the opendir system call.
*/
int
sys_readdir(int fd, struct dirent *dirent)
{
	struct dirhandler *dirhandler;

	dirhandler = thread_get_fd_handler(fd);
	if (dirhandler == NULL) {
		return (-1);
	}
	if (fs_readdir(dirhandler, dirent)) {
		return (-1);
	}
	return (0);
}

/*
** Does the closedir system call.
*/
int
sys_closedir(int fd)
{
	struct dirhandler *dirhandler;

	dirhandler = thread_get_fd_handler(fd);
	if (dirhandler == NULL) {
		return (-1);
	}
	fs_closedir(dirhandler);
	thread_free_fd(fd);
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

/*
** Does the execve system call.
*/
int
sys_execve(char const *name, int (*main)(), char const *args[])
{
	char **argv;
	int argc;

	/* We need to copy args or they will point to invalid memory */
	argc = 0;
	while (args[argc]) {
		++argc;
	}
	argv = kalloc(sizeof(char *) * (argc + 1));
	assert_neq(argv, NULL);
	argc = 0;
	while (args[argc]) {
		argv[argc] = strdup(args[argc]);
		++argc;
	}
	argv[argc] = NULL;

	thread_execve(name, main, argc, argv);

	argc = 0;
	while (argv[argc]) {
		kfree(argv[argc]);
		++argc;
	}
	kfree(argv);
	return (0);
}
