/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/init.h>
#include <kernel/multiboot.h>
#include <multiboot2.h>
#include <stdio.h>
#include <string.h>

/* The first tag given by multiboot, if any */
struct multiboot_tag *mb_tag = NULL;

/* Informations given to us using multiboot */
struct multiboot_info multiboot_infos;

/* Command line arguments */
struct cmd_options cmd_options =
{
	.unit_test = false,
};

/*
** Parse the command line arguments
*/
static void
parse_cmd_options(void)
{
	if (multiboot_infos.args) {
		cmd_options.unit_test = strstr(multiboot_infos.args, "--unit-test") != NULL;
	}
}

/*
** Goes through the multiboot structure, parsing it's content
*/
static void
multiboot_load(void)
{
	struct multiboot_tag *tag;

	printf("[..]\t Multiboot");
	memset(&multiboot_infos, 0, sizeof(multiboot_infos));
	tag = mb_tag;
	while (tag->type != MULTIBOOT_TAG_TYPE_END)
	{
		switch (tag->type)
		{
		case MULTIBOOT_TAG_TYPE_CMDLINE:
			multiboot_infos.args = ((struct multiboot_tag_string *)tag)->string;
			break;
		case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
			multiboot_infos.bootloader = ((struct multiboot_tag_string *)tag)->string;
			break;
		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			multiboot_infos.mem_start = ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower;
			multiboot_infos.mem_stop = ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper;
			break;
		case MULTIBOOT_TAG_TYPE_MMAP:
			multiboot_infos.mmap = ((struct multiboot_tag_mmap *)tag)->entries;
			multiboot_infos.mmap_entry_size = ((struct multiboot_tag_mmap *)tag)->entry_size;
			multiboot_infos.mmap_end = (multiboot_memory_map_t *)((uchar *)tag + tag->size);
			break;
		}
		tag = (struct multiboot_tag *)((uchar *)tag + ((tag->size + 7) & ~7));
	}
	parse_cmd_options();
	printf("\r[OK]\tMultiboot [%s] [%s]\n", multiboot_infos.bootloader, multiboot_infos.args);
}

static void
multiboot_init(enum init_level il __unused)
{
	if (mb_tag) {
		multiboot_load();
	}
}

NEW_INIT_HOOK(multiboot, &multiboot_init, CHAOS_INIT_LEVEL_MULTIBOOT);
