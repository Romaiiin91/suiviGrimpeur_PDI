#!/bin/sh
#
# Delete one regular file uploaded by users and defined by query
# Allow white space in file name

ret=0
cd /etc/overlays || ret=$?
if [ $ret -ne 0 ]; then
	echo -e "cd failed:" $ret
	exit $ret
fi

if test $# -eq 0; then
	echo -e "No file name provided"
	exit 1
fi

param=root.Image.OverlayPath
use_file=`/bin/parhandclient get $param | tr -d '\"\n\r'`
if [ "$*" = "$use_file" ]; then
	/bin/parhandclient --nocgi set $param "" > /dev/null 2>&1
fi

file=`echo $* | sed -e 's/\/etc\/overlays\///'`
rm -rf "$file" || ret=$?
if [ $ret -ne 0 ]; then
	echo -e "rm failed:" $ret
	exit 1
fi

exit 0
