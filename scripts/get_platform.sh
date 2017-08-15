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

if [ $# -ne 1 ]; then
	printf "Usage: $0 <arch>\n"
	exit 1
fi

case $1 in
	x86) printf "pc\n";;
	*)
		printf "unknown\n"
		exit 1
esac
