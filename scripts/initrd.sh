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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/../"
BUILD_DIR="$PROJECT_DIR/build/"
OUTPUT=initrd.img
OUTPUT_PATH="$BUILD_DIR"/"$OUTPUT"

# Check that dependencies are installed
if ! which dd mcopy /sbin/mkfs.fat &> /dev/null; then
	printf "  ERROR\t You must install dd, mcopy and mkfs.fat\n"
	exit 1
fi

# Temp file for error output, or silence on success.
TEMP=$(mktemp)

# Create the ramdisk image
printf "  TOUCH\t $(basename "$OUTPUT")\n"
touch "$OUTPUT_PATH"

# Fill it with zeroes
printf "  DD\t $OUTPUT\n"
if ! dd if=/dev/zero of="$OUTPUT_PATH" bs=1024 count=1024 &> "$TEMP"; then
	cat "$TEMP"
	exit 1
fi

# Format
printf "  MKFS\t $OUTPUT\n"
if ! /sbin/mkfs.fat -F 16 "$OUTPUT_PATH" &> "$TEMP"; then
	cat "$TEMP"
	exit 1
fi

# Copies given file to the initrd at the given path
function initrd_cpy {
	printf "  MCOPY\t $OUTPUT -> $2\n"
	mcopy -i "$OUTPUT_PATH" "$1" ::"$2"
}

# Copy binaries
for file in "$PROJECT_DIR"/userspace/*; do
	if [ -f $file ] && [ -x $file ]; then
		initrd_cpy "$file" "/$(basename "$file")"
	fi
done

# Copy Hello World file
cat << EOF > "$TEMP"
Hello, World!
EOF

initrd_cpy "$TEMP" "/readme"
