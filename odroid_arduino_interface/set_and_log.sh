#!/bin/sh

. ./parameters

stty $BAUD_RATE -F $DEV raw -hup -echo min 1
if [ $? -ne 0 ]; then
	echo "error on setting serial parameters"
	exit 1
fi
(while read r
do
	logger -p daemon.info "Arduino: $r"
	if [ $? -ne 0 ]; then
		echo "error on logging $r"
		exit 1
	fi
done) < $DEV
