#!/bin/sh

. ./parameters

stty $BAUD_RATE -F $DEV raw -hup -echo min 1
if [ $? -ne 0 ]; then
	echo "error on setting serial parameters"
	exit 1
fi
