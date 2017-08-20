##############################################################################
##
##  This file is part of the Chaos Kernel, and is made available under
##  the terms of the GNU General Public License version 2.
##
##  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
##
##############################################################################

# Initrd
BINS		:= touch cat
INITRD		:= initrd.img

# Flags
LD		?= ld
CFLAGS		+= \
		-m32 \
		-mno-sse \
		-mno-sse2 \
		-mno-sse3 \
		-mno-sse4.1 \
		-mno-sse4.2 \
		-mno-sse4 \
		-nostdlib \
		-nostdinc \
		-fno-builtin \
		-Wall \
		-Wextra \
		-std=gnu11 \
		-O2 \
		-Wno-frame-address \
		-Wno-unknown-warning-option \
		-isystem ../include
LDFLAGS		:= -m elf_i386

all:		$(INITRD)

$(INITRD):	$(BINS)
		touch $@
		printf "  DD\t $@\n"
		dd if=/dev/zero of=$@ bs=1024 count=1024 2> /tmp/dd_err || cat /tmp/dd_err
		printf "  MKFS\t $@\n"
		/sbin/mkfs.fat -F 16 $@ > /tmp/mkfs_err 2>&1 || cat /tmp/mkfs_err
		mcopy -i $@ $(BINS) ::/
		printf "  MCOPY\t $(BINS) -> $@\n"

clean:
		$(RM) $(addsuffix .o, $(BINS))
		$(RM) $(BINS)

re:		clean all

.SECONDEXPANSION:
$(BINS):	$$(addsuffix .o, $$@)
		$(LD) $(LDFLAGS) $< -o $@ && printf "  LD\t $<\n"


-include	$(DEP)
%.o:		%.c
		$(CC) $(CFLAGS) -c $< -o $@ && printf "  CC\t $<\n"

.PHONY: all clean re

.SILENT: all clean re $(INITRD) $(BINS) $(addsuffix .o, $(BINS))
