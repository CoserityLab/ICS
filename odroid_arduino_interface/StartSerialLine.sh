#!/bin/sh

stty 2400 -F /dev/ttyS0 raw -hup -echo min 1
