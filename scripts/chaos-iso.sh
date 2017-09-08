#!/usr/bin/env bash

##############################################################################
##
##  This file is part of the Chaos Kernel, and is made available under
##  the terms of the GNU General Public License version 2.
##
##  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
##
##############################################################################

set -e -u

function print_usage {
	printf "Usage: $0 [OPTIONS]\n"
	printf "\t-b <boot_flags>		Set some addition flags to give to the kernel.\n"
	exit 1
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/../"
BUILD_DIR="$PROJECT_DIR/build/"
BOOT_ARGS=""
RULES=kernel
OUTPUT=chaos.iso

while getopts b: FLAG; do
	case $FLAG in
		b)
			BOOT_ARGS="$OPTARG";;
		*)
			printf "Unknown option\n"
			print_usage
	esac
done

if [ ! -f "$BUILD_DIR/chaos.bin" ]; then
	printf "  MAKE\t $RULES\n"
	make -C "$PROJECT_DIR" --no-print-directory "$RULES"
fi

TEMP=$(mktemp -d)

mkdir -p "$TEMP/boot/grub"
cp "$BUILD_DIR/chaos.bin" "$TEMP/boot/chaos.bin"
cp "$BUILD_DIR/initrd.img" "$TEMP/boot/initrd.img"

cat > "$TEMP/boot/grub/grub.cfg" << EOF
set timeout=0

menuentry "ChaOS" {
	multiboot2 /boot/chaos.bin ${BOOT_ARGS}
	module2 /boot/initrd.img
}
EOF

GRUB_OUTPUT=$(mktemp)

printf "  GRUB\t chaos.iso\n"

if ! grub-mkrescue -o "$PROJECT_DIR/chaos.iso" "$TEMP" &> "$GRUB_OUTPUT" ; then
	cat "$GRUB_OUTPUT"
	exit 1
fi

rm -rf "$GRUB_OUTPUT"

rm -rf "$TEMP"
