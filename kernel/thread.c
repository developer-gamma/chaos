/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/thread.h>
#include <kernel/kalloc.h>
#include <stdio.h>
#include <string.h>

/* Next pid available */
static pid_t next_pid = 1;

/* Thread table */
struct thread thread_table[MAX_PID];
struct thread *init_thread = thread_table + 1;
struct spinlock thread_table_lock;

/*
** Look for the next available pid.
** Returns -1 if no pid are available.
*/
static pid_t
find_next_pid(void)
{
	bool pass;
	pid_t pid;
	pid_t limit;

	pass = false;
	pid = next_pid;
	limit = MAX_PID;
	assert(holding_lock(&thread_table_lock));
find_pid:
	while (pid < limit)
	{
		if (thread_table[pid].state == NONE) {
			return (pid);
		}
		++pid;
	}
	if (!pass)
	{
		pid = 1;
		limit = next_pid;
		pass = true;
		goto find_pid;
	}
	return (-1);
}

/*
** Sets the name of the given thread.
*/
static void
thread_set_name(struct thread *t, char const *name)
{
	strncpy(t->name, name, sizeof(t->name) - 1);
	t->name[sizeof(t->name) - 1] = '\0';
}

/*
** Creates a new thread.
** The newly created thread is in a suspended state,
** so it should be resumed with thread_resume().
**
** Returns NULL if the thread couldn't be created.
*/
struct thread *
thread_create(char const *name, thread_entry_cb entry, size_t stack_size)
{
	struct thread *t;
	pid_t pid;

	LOCK_THREAD(state)

	pid = find_next_pid();
	if (pid == -1) {
		goto err;
	}

	t = thread_table + pid;
	memset(t, 0, sizeof(*t));
	thread_set_name(t, name);
	t->pid = pid;
	t->entry = entry;
	t->state = RUNNABLE;
	t->parent = get_current_thread()->parent;
	t->vaspace = get_current_thread()->vaspace;
	t->vaspace->ref_count++;
	t->cwd = strdup(get_current_thread()->cwd);

	t->stack_size = stack_size;
	t->stack = mmap(NULL, stack_size, MMAP_USER | MMAP_WRITE);
	assert_neq(t->stack, NULL);
	t->stack += stack_size - 1;
	t->stack = (void *)ROUND_DOWN((uintptr)t->stack, sizeof(void *));

	arch_init_thread(t);

	RELEASE_THREAD(state);
	return (t);

err:
	RELEASE_THREAD(state);
	return (NULL);
}

/*
** Fork the given thread and it's virtual space
*/
struct thread *
thread_fork(void)
{
	pid_t pid;
	struct vaspace *vaspace;
	struct thread *new;
	struct thread *old;

	LOCK_THREAD(state);

	old = get_current_thread();

	pid = find_next_pid();
	if (pid == -1) {
		goto err;
	}

	/* clone virtual address space */
	vaspace = arch_clone_vaspace(old->vaspace);
	if (!vaspace) {
		goto err;
	}

	new = thread_table + pid;
	memcpy(new, old, sizeof(*new));
	new->pid = pid;
	new->state = RUNNABLE;
	new->parent = old;
	new->vaspace = vaspace;
	new->cwd = strdup(old->cwd);

	arch_init_fork_thread(new);

	RELEASE_THREAD(state);
	return (new);
err:
	RELEASE_THREAD(state);
	return (NULL);
}

/*
** Exit the current thread.
*/
void
thread_exit(int status)
{
	struct thread *t;

	t = get_current_thread();

	LOCK_THREAD(state);

	assert_eq(t->state, RUNNING);

	/* init should never finish */
	if (unlikely(t->pid == 1)) {
		panic("%s finished (exit status: %u)", t->name, status);
	}

	/* free the virtual address space if we are the last thread using it */
	t->vaspace->ref_count--;
	if (t->vaspace->ref_count == 0) {
		free_vaspace();
	}

	t->exit_status = status & 0xFFu;
	t->state = ZOMBIE;
	thread_reschedule();

	panic("Reached end of thread_exit()"); /* We shoudln't reach this portion of code. */
}

/*
** Called when a zombie thread is waited. Used to free it's
** kernel memory.
*/
void
thread_zombie_exit(struct thread *zombie)
{
	free_zombie_thread(zombie);
	zombie->state = NONE;
	next_pid = zombie->pid;
}

/*
** Change the program executed by the current thread
*/
status_t
thread_execve(char const *name, int (*main)(void))
{
	struct thread *t;

	LOCK_VASPACE(state)

	t = get_current_thread();
	thread_set_name(t, name);
	t->entry = main;

	/* TODO kill other threads here */
	assert_eq(get_current_thread()->vaspace->ref_count, 1);

	free_vaspace();

	/* TODO load the given binary here */
	get_current_thread()->vaspace->binary_limit = PAGE_SIZE;

	/* Initialize the new vaspace */
	init_vaspace();

	/* Allocate main-thread's stack */
	t->stack = mmap(NULL, t->stack_size, MMAP_USER | MMAP_WRITE);
	assert_neq(t->stack, NULL);
	t->stack += t->stack_size - 1;
	t->stack = (void *)ROUND_DOWN((uintptr)t->stack, sizeof(void *));

	/* set IP and other arch-related stuff */
	arch_thread_execve();

	RELEASE_VASPACE(state);
	return (OK);
}

/*
** Resume the given thread.
*/
void
thread_resume(struct thread *t)
{

	LOCK_THREAD(state);
	assert_neq(t->state, ZOMBIE);
	if (t->state == SUSPENDED) {
		t->state = RUNNABLE;
		RELEASE_THREAD(state);
		thread_yield();
	}
	else
		RELEASE_THREAD(state);
}

/*
** Waits for the process with the given pid to finish.
** Returns the exit status of the targeted process.
*/
int
thread_waitpid(pid_t pid)
{
	struct thread *t;
	int val;

	t = thread_table + pid;
	assert(arch_are_int_enabled());

	while (42) { /* TODO This is unsafe, a wakeup() approach is safer */
		LOCK_THREAD(state);
		if (t->state == ZOMBIE) {
			val = t->exit_status;
			thread_zombie_exit(t);

			RELEASE_THREAD(state);
			return (val);
		}
		RELEASE_THREAD(state);
	}
}

/*
** Copies the current thread's working directory within the given buffer
** and returns it, or returns NULL on error.
*/
char *
thread_getcwd(char *buff, size_t buffsize)
{
	char *path;
	size_t path_len;

	LOCK_THREAD(state);
	path = get_current_thread()->cwd;
	path_len = strlen(path);
	if (path_len >= buffsize) {
		RELEASE_THREAD(state);
		return (NULL);
	}
	memcpy(buff, path, path_len);
	buff[path_len] = '\0';
	RELEASE_THREAD(state);
	return (buff);
}

/*
** Finishes the init of the thread system.
*/
void
thread_init(void)
{
	struct thread *t;

	assert(!arch_are_int_enabled());

	/* Initialize the default virtual address space */
	init_vaspace();

	/* This needs to be done now to prevent strdup() with null ptr */
	get_current_thread()->cwd = strdup("/");

	/* Create the init thread */
	t = thread_create("init", &init_routine, DEFAULT_STACK_SIZE);
	assert_neq(t, NULL);
	assert_eq(t, init_thread);
	assert_eq(t->pid, 1);

	printf("[OK]\tMulti-threading\n");

	/* Print HelloWorld message */
	printf("\nWelcome to ChaOS\n\n");

	/* Enable interrupts (hurrah!) */
	arch_enable_interrupts();

	LOCK_THREAD(state);

	/* Remove the boot thread from the active threads */
	get_current_thread()->state = NONE;

	/* Set the init thread as runnable */
	t->state = RUNNABLE;

	/* Free the pwd */
	kfree(t->cwd);

	/* Reschedule */
	thread_reschedule();
}

/*
** Put us in some sort of thread context.
** Called from kmain.
*/
void
thread_early_init(void)
{
	struct thread *t;

	t = thread_table;
	memset(thread_table, 0, sizeof(thread_table));

	thread_set_name(t, "boot");
	t->pid = 0;
	t->state = RUNNING;
	t->vaspace = setup_boot_vaspace();

	/* Set current thread */
	set_current_thread(t);

	/* Register the timer handler. */
	register_int_handler(IRQ_TIMER_VECTOR, &irq_timer_handler);
}

/*
** Thread dumper to help debug.
*/
void
thread_dump(void)
{
	struct thread *t;

	t = thread_table;
	while (t < thread_table + MAX_PID)
	{
		if (t->state != NONE) {
			printf("%i:[%s] - [%s]\n", t->pid, t->name, thread_state_str[t->state]);
		}
		++t;
	}
}
