/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/filesystem.h>
#include <kernel/init.h>
#include <kernel/kalloc.h>
#include <string.h>

/* Debug */
#include <stdio.h>

static struct fs_node *root_node;

static char *resolve_path(char const *cwd, char const *input);

/*
** Opens the given node
*/
static void
node_open(struct fs_node *node)
{
	node->ref_count++;
	if (node->open) {
		node->open(node);
	}
}

/*
** Looks for a given file in the given directory node.
*/
static struct fs_node *
node_find_file(struct fs_node *dir, char const *filename)
{
	if (dir->type & FS_DIRECTORY && dir->find_file) {
		return (dir->find_file(dir, filename));
	}
	return (NULL);
}

/*
** Opens a file by name.
*/
struct fs_node *
kopen(char const *relative_to, char const *filename)
{
	struct fs_node *node;
	char *abs_path;
	size_t abs_path_len;

	char *path_offset;
	size_t path_depth = 0;
	size_t depth;

	abs_path = resolve_path(relative_to, filename);
	abs_path_len = strlen(abs_path);

	/* if abs_path_len == 1, we're looking for '/', the root node */
	if (abs_path_len == 1) {
		node = root_node;
		goto ok;
	}

	/* Overwhise, let's tokenize the path */
	path_offset = abs_path;
	while (path_offset < abs_path + abs_path_len) {
		if (*path_offset == PATH_SEPARATOR) {
		*path_offset = 0;
			++path_depth;
		}
		++path_offset;
	}
	path_offset = abs_path + 1;

	/* Look */
	depth = 0;
	node = root_node;
	while (depth < path_depth)
	{
		node = node_find_file(node, path_offset);
		if (!node) { /* We've failed to find the given file */
			goto err;
		}

		/* If this is the last layer of folder */
		if (depth == path_depth - 1) {
			goto ok;
		}

		path_offset += strlen(path_offset) + 1;
		++depth;
	}

err:
	kfree(abs_path);
	return (NULL);

ok:
	node_open(node);
	kfree(abs_path);
	return (node);
}

/*
** Appends the two path in one, and resolves simlinks, '..', etc.
** (That's a lie, simlinks aren't implemented yet... :p)
**
** Returns an absoluthe path, or NULL on error.
** The returned strings must be free'd using kfree().
**
** Inspired by lk's 'fs_normalize_path()'
*/
static char *
resolve_path(char const *cwd, char const *input)
{
	char *path;
	int outpos;
	int pos;
	bool done;
	char c;

	enum {
		INITIAL,
		FIELD_START,
		IN_FIELD,
		SEPARATOR,
		SEEN_SEPARATOR,
		DOT,
		SEEN_DOT,
		DOTDOT,
		SEEN_DOTDOT,
	} state;

	state = INITIAL;
	pos = 0;
	outpos = 0;
	done = false;

	/* Is input an absoluthe or relative path ? */
	if (*input == PATH_SEPARATOR) {
		path = strdup(input);
	} else {
		path = kalloc(sizeof(char) * (strlen(input) + strlen(cwd) + 2));
		strcpy(path, cwd);
		strcat(path, PATH_SEPARATOR_STR);
		strcat(path, input);
	}

	if (path == NULL) {
		return (NULL);
	}

	/* Resolve the given path */
	while (!done)
	{
		c = path[pos];
		switch (state) {
		case INITIAL:
			if (c == PATH_SEPARATOR) {
				state = SEPARATOR;
			}
			else if (c == PATH_CURRENT) {
				state = DOT;
			} else {
				state = FIELD_START;
			}
			break;
		case FIELD_START:
			if (c == PATH_CURRENT) {
				state = DOT;
			} else if ( c == '\0') {
				done = true;
			} else {
				state = IN_FIELD;
			}
			break;
		case IN_FIELD:
			if (c == PATH_SEPARATOR) {
				state = SEPARATOR;
			} else if (c == '\0') {
				done = true;
			} else {
				path[outpos++] = c;
				pos++;
			}
			break;
		case SEPARATOR:
			path[outpos++] = PATH_SEPARATOR;
			pos++;
			state = SEEN_SEPARATOR;
			break;
		case SEEN_SEPARATOR:
			if (c == PATH_SEPARATOR) {
				++pos;
			} else if (c == '\0') {
				done = true;
			} else {
				state = FIELD_START;
			}
			break;
		case DOT:
			pos++;
			state = SEEN_DOT;
			break;
		case SEEN_DOT:
			if (c == PATH_CURRENT) {
				state = DOTDOT;
			} else if (c == PATH_SEPARATOR) { /* Filename == '.', eat it */
				++pos;
				state = SEEN_SEPARATOR;
			} else if (c == '\0') {
				done = true;
			} else { /* Filename starting with '.' */
				path[outpos++] = PATH_CURRENT;
				state = IN_FIELD;
			}
			break;
		case DOTDOT:
			pos++;
			state = SEEN_DOTDOT;
			break;
		case SEEN_DOTDOT:
			if (c == PATH_SEPARATOR || c == '\0') { /* Filename == '..' */
				outpos -= (outpos > 0);
				while (outpos > 0) { /* Walk backward */
					if (path[outpos - 1] == PATH_SEPARATOR) {
						break;
					}
					--outpos;
				}
				pos++;
				state = SEEN_SEPARATOR;
				if (c == '\0') {
					done = true;
				}
			} else { /* Filename starting with '.. */
				path[outpos++] = PATH_CURRENT;
				path[outpos++] = PATH_CURRENT;
				state = IN_FIELD;
			}
			break;
		}
	}

	/* Remove trailing slash */
	if (outpos == 0) {
		path[outpos++] = PATH_SEPARATOR;
	} else if (outpos > 1 && path[outpos - 1] == PATH_SEPARATOR) {
		--outpos;
	}

	path[outpos] = 0;
	return (path);
}

static void
init_fs(enum init_level il __unused)
{
	root_node = kalloc(sizeof(struct fs_node));
	assert_neq(root_node, NULL);

	memset(root_node, 0, sizeof(struct fs_node));
	root_node->name[0] = '/';
	root_node->type |= FS_DIRECTORY;

	printf("[OK]\tFilesystem\n");
}

NEW_INIT_HOOK(filesystem, &init_fs, CHAOS_INIT_LEVEL_FILESYSTEM);
