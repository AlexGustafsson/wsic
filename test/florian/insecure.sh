#!/bin/sh

if [ $# -ne 1 ]; then
	echo "usage: $0 <src-folder>"
	exit 1
fi

echo "strcpy:"
grep -Rn strcpy "$1"

echo "strcat:"
grep -Rn strcat "$1"

echo "sprintf:"
grep -Rn sprintf "$1"

echo "gets:"
grep -Rn gets "$1"

echo "strtok:"
grep -Rn strtok "$1"

echo "scanf:"
grep -Rn scanf "$1"

echo "itoa:"
grep -Rn itoa "$1"
