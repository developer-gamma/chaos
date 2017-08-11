/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _UNISTD_H_
# define _UNISTD_H_

# include <chaosdef.h>

typedef int	pid_t;

/*
** Userspace way of calling each syscalls.
** These functions are implemented in each architecture.
*/
void		exit(void);
pid_t		fork(void);
ssize_t		write(int fd, const void *buff, size_t count);
ssize_t		read(int fd, void *buff, size_t count);
int		brk(void *addr);
void		*sbrk(intptr inc);
pid_t		getpid(void);

#endif /* !_UNISTD_H_ */
