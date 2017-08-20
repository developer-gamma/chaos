/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_LIST_H_
# define _KERNEL_LIST_H_

# include <chaosdef.h>

/*
** Double linked list implementation
**
** Inspired by the Linux Kernel implementation.
*/

struct list_node
{
	struct list_node *prev;
	struct list_node *next;
};

# define LIST_INIT_HEAD(ptr)			\
	do {					\
		(ptr)->next = (ptr);		\
		(ptr)->prev = (ptr);		\
	} while (0);

# define LIST_INIT_VALUE(list) { &(list), &(list) }
# define LIST_CLEAR_VALUE { NULL, NULL }

/*
** Adds a new node to the list between the two specified node.
*/
static inline void
list_add_between(struct list_node *new, struct list_node *prev, struct list_node *next)
{
	prev->next = new;
	new->prev = prev;
	new->next = next;
	next->prev = new;
}

/*
** Inserts a new node to the list after the specified head.
** Good for stacks
*/
static inline void
list_add(struct list_node *new, struct list_node *head)
{
	list_add_between(new, head, head->next);
}

/*
** Inserts a new node to the list before the specified head.
** Good for queues.
*/
static inline void
list_add_tail(struct list_node *new, struct list_node *head)
{
	list_add_between(new, head->prev, head);
}

/*
** Removes a node of the list by making it's prev/next pointers point to each other.
*/
static inline void
list_remove(struct list_node *prev, struct list_node *next)
{
	next->prev = prev;
	prev->next = next;
}

/*
** Deletes a node of the list, by removing it and reset it's pointer to default values.
*/
static inline void
list_delete(struct list_node *node)
{
	list_remove(node->prev, node->next);
	node->next = NULL;
	node->prev = NULL;
}

/*
** Deletes from one list and add as another's head.
*/
static inline void
list_move(struct list_node *list, struct list_node *head)
{
	list_remove(list->prev, list->next);
	list_add(list, head);
}

/*
** Deletes from one list and add as another's tail
*/
static inline void
list_move_tail(struct list_node *list, struct list_node *head)
{
	list_remove(list->prev, list->next);
	list_add_tail(list, head);
}

/*
** Returns true if the given list is empty, false otherwise.
*/
static inline bool
list_empty(struct list_node *head)
{
	return (head->next == head);
}

static inline void
__list_zip(struct list_node *list, struct list_node *head)
{
	struct list_node *first = list->next;
	struct list_node *last = list->prev;
	struct list_node *at = head->next;

	first->prev = head;
	head->next = first;
	last->next = at;
	at->prev = last;
}

/*
** Joins two lists
**
** Appends the list 'list' at place 'head'.
*/
static inline void
list_zip(struct list_node *list, struct list_node *head)
{
	if (!list_empty(list))
		__list_zip(list, head);
}

/*
** Gets the content of the given node
*/
# define get_content(ptr, type, member)  \
	((type *)((char *)(ptr)-(uintptr)(&((type *)0)->member)))

/*
** Iterates over a list.
*/
# define list_foreach(pos, head) \
	for (pos = (head)->next; pos != (head); pos = (pos)->next)

/*
** Iterates over a list backwards.
*/
# define list_foreach_back(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = (pos)->prev)

/*
** Iterates over a list.
**
** Safe against removal of a list node.
*/
# define list_safe_foreach(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n;  n = pos->next)

/*
** Iterates over a list content.
*/
# define list_foreach_content(pos, head, member)			\
	for (pos = get_content((head)->next, typeof(*pos), member);	\
		&pos->member != (head);					\
		pos = get_content(pos->member.next, typeof(*pos), member))

/*
** Iterates over a list content.
**
** Safe against removal of a list node.
*/
# define list_safe_foreach_content(pos, n, head, member)		\
	for (pos = get_content((head->next), typeof(*pos), member);	\
	     n = get_content((pos)->member.next, typeof(*pos), member);	\
		&pos->member != (head);					\
		pos = n, n = get_content(n->member.next, typeof(*pos), member))

#endif /* !_KERNEL_LIST_H_ */
