/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_PMM_H_
# define _KERNEL_PMM_H_

# include <chaosdef.h>
# include <kernel/linker.h>
# include <limits.h>

/* A physical address */
typedef uintptr			phys_addr_t;

/* The NULL equivalent for physical memory */
# define NULL_FRAME		(-1u)

/* The number of frames */
# define NB_FRAMES		((UINTPTR_MAX / PAGE_SIZE) + 1)

/* *The number of cells of the frame bitmap */
# define FRAME_BITMAP_SIZE	(NB_FRAMES / 8u)

/* Macro to easily get the index in the bitmap of any physical address */
# define GET_FRAME_IDX(x)		(((x) >> 12u) / 8u)
# define GET_FRAME_MASK(x)		(1 << (((x) >> 12) % 8u))

/* Physical allocation functions */
phys_addr_t			alloc_frame(void);
void				free_frame(phys_addr_t);
size_t				nb_free_frames(void);

extern uchar			frame_bitmap[FRAME_BITMAP_SIZE];

/*
** Returns true if the given address is taken.
*/
static bool
is_frame_allocated(phys_addr_t frame)
{
	assert(IS_PAGE_ALIGNED(frame));
	return (frame_bitmap[GET_FRAME_IDX(frame)] & (GET_FRAME_MASK(frame)));
}

/*
** Mark a frame as allocated.
*/
static inline void
mark_frame_as_allocated(phys_addr_t frame)
{
	assert(IS_PAGE_ALIGNED(frame));
	frame_bitmap[GET_FRAME_IDX(frame)] |= GET_FRAME_MASK(frame);
}

/*
** Mark a frame as freed.
*/
static inline void
mark_frame_as_free(phys_addr_t frame)
{
	assert(IS_PAGE_ALIGNED(frame));
	frame_bitmap[GET_FRAME_IDX(frame)] &= ~GET_FRAME_MASK(frame);
}

#endif /* !_KERNEL_PMM_H_ */
