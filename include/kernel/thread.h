/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_THREAD_H_
# define _KERNEL_THREAD_H_

# include <kernel/vmm.h>
# include <kernel/vaspace.h>
# include <arch/thread.h>
# include <chaosdef.h>
# include <config.h>

# define IRQ_TIMER_VECTOR	(0x0)

typedef int			pid_t;
typedef int			(*thread_entry_cb)(void);

enum			thread_state
{
	NONE = 0,
	SUSPENDED,
	RUNNABLE,
	RUNNING,
	ZOMBIE,
};

/* String to print thread state */
static char const *thread_state_str[] =
{
	[NONE]		= "NONE",
	[SUSPENDED]	= "SUSPENDED",
	[RUNNABLE]	= "RUNNABLE",
	[RUNNING]	= "RUNNING",
	[ZOMBIE]	= "ZOMBIE",
};

struct filehandler;

struct filedesc
{
	bool taken;
	struct filehandler *handler;
};

struct			thread
{
	/* Thread basic infos*/
	char name[255];
	pid_t pid;
	uchar exit_status;
	enum thread_state state;
	struct thread *parent;
	char *cwd;

	/* File descriptors */
	struct filedesc *fd_tab;
	size_t fd_size;

	/* Thread stack */
	virt_addr_t stack;
	size_t stack_size;

	/* arch stuff */
	struct arch_thread arch;

	/* entry point */
	thread_entry_cb entry;

	/* virtual address space */
	struct vaspace *vaspace;
};

void			thread_init(void);
void			thread_early_init(void);
int			init_routine(void);

struct thread		*thread_fork(void);
struct thread		*thread_create(char const *name, thread_entry_cb entry, size_t stack_size);
int			thread_reserve_fd(void);
void			thread_set_fd_handler(int fd, struct filehandler *hd);
void			thread_free_fd(int fd);
struct			filehandler *thread_get_fd_handler(int fd);
void			thread_dump(void);
void			thread_yield(void);
void			thread_reschedule(void);
void			thread_resume(struct thread *);
void			thread_exit(int);
int			thread_waitpid(pid_t);
char			*thread_getcwd(char *buffer, size_t len);
status_t		thread_execve(char const *, int (*)(void));
enum handler_return	irq_timer_handler(void);

/* Must be implemented in each architecture */
void			set_current_thread(struct thread *);
struct thread		*get_current_thread(void);
void			arch_context_switch(struct thread *old, struct thread *new);
void			arch_init_thread(struct thread *);
void			arch_init_fork_thread(struct thread *new);
void			arch_thread_execve(void);

# define LOCK_THREAD(state)	LOCK(&thread_table_lock, state)
# define RELEASE_THREAD(state)	RELEASE(&thread_table_lock, state)

#endif /* !_KERNEL_THREAD_H_ */
