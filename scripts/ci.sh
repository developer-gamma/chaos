#!/usr/bin/env bash

##############################################################################
##
##  This file is part of the Chaos Kernel, and is made available under
##  the terms of the GNU General Public License version 2.
##
##  Copyright (C) 2017 - Antoine Motet
##
##############################################################################

set -e -u

SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
PROJECT_DIR="$SCRIPT_DIR/../"

cd "$PROJECT_DIR"

docker build -t chaos -f ./scripts/Dockerfile .

docker run --rm --name chaos -d chaos \
	qemu-system-i386 \
		-nographic \
		-serial stdio \
		-monitor none \
		-boot d \
		-cdrom chaos.iso

sleep 2

logs=$(docker logs chaos)
docker stop chaos

echo "$logs" > /tmp/logs

printf "===== LOGS =====\n"
cat /tmp/logs
printf "================\n"

grep "Welcome to ChaOS" /tmp/logs &> /dev/null

printf "Test passed\n"
