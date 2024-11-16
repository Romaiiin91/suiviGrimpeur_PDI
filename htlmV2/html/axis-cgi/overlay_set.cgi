#!/bin/sh

log="/usr/bin/logger -t $0 -p local7.info"
ret=0

if [ "$*" = "no_overlay" ] || [ "$*" = "" ]; then
	/bin/parhandclient --nocgi set root.Image.OverlayPath "" || ret=$?
	if [ $ret -ne 0 ]; then
		$log "parhandclient failed to set value."
		exit $ret
	fi
	exit 0
fi

# A file name as inparameter but it does not exist
if [ ! -f "$*" ] && [ ! -d "$*" ]; then
	$log "no such file: $1"
	exit 1
fi

/bin/parhandclient --nocgi set root.Image.OverlayPath "$*" || ret=$?
if [ $ret -ne 0 ]; then
	$log "parhandclient failed to set value."
	exit $ret
fi

exit 0
