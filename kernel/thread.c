/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/spinlock.h>
#include <kernel/init.h>
#include <kernel/thread.h>
#include <kernel/interrupts.h>
#include <stdio.h>
#include <string.h>

/* Next pid available */
static pid_t next_pid = 1;

/* Thread table */
struct thread thread_table[MAX_PID];
struct thread *init_thread = thread_table + 1;
struct spinlock thread_table_lock;

/* Default virtual address space for the boot thread */
struct vaspace default_vaspace;

/*
** Look for the next available pid.
** Returns -1 if no pid are available.
*/
static pid_t
find_next_pid()
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
		RELEASE_THREAD(state);
		return (NULL);
	}

	t = thread_table + pid;
	memset(t, 0, sizeof(*t));
	thread_set_name(t, name);
	t->pid = pid;
	t->stack_size = stack_size;
	t->entry = entry;
	t->state = RUNNABLE;
	t->parent = get_current_thread()->parent;
	t->vaspace = get_current_thread()->vaspace;
	t->vaspace->ref_count++;

	t->stack = mmap(NULL, stack_size);
	assert_neq(t->stack, NULL);

	arch_init_thread(t);

	RELEASE_THREAD(state);
	return (t);
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
thread_exit(void)
{
	struct thread *t;

	t = get_current_thread();

	LOCK_THREAD(state);

	assert_eq(t->state, RUNNING);

	if (unlikely(t->pid == 1)) {
		panic("init finished");
	}

	t->vaspace->ref_count--;
	/* TODO free thread's memory space and kernel stack */

	t->state = NONE; /* TODO change this in ZOMBIE */
	next_pid = t->pid;
	thread_reschedule();

	panic("Reached end of thread_exit()"); /* We shoudln't reach this portion of code. */
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
** Finishes the init of the thread system.
*/
void
thread_init(void)
{
	struct thread *t;

	assert(!arch_are_int_enabled());

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
	t->vaspace = &default_vaspace;

	/* Set-up default virtual space */
	memset(&default_vaspace, 0, sizeof(default_vaspace));
	init_lock(&default_vaspace.lock);

	default_vaspace.mmapping_start = (char *)ALIGN((uintptr)KERNEL_VIRTUAL_BASE, PAGE_SIZE) - PAGE_SIZE;
	default_vaspace.binary_limit = ALIGN(KERNEL_PHYSICAL_END, PAGE_SIZE);
	default_vaspace.heap_start = (char *)default_vaspace.binary_limit + PAGE_SIZE;
	default_vaspace.binary_limit = ALIGN(KERNEL_PHYSICAL_END, PAGE_SIZE);

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
