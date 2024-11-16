#!/bin/sh

# Runs a few test moves to let a user test a new ptz speed before saving it.
#

F_CALLPATH__=${0%/*}
. $F_CALLPATH__/lib/functions.sh
unset F_CALLPATH__

dbus_cmd="dbus-send --system --print-reply --dest=com.axis.PTZ.Coordinator /com/axis/PTZ/Coordinator/1 com.axis.PTZ.Coordinator.MovePTZ string:Administrator int32:0"
scc_prefix="statuscacheclient get RAW PTZ_POSITION_IS1"

wait_idle() {
	local i=0 pan tilt

	while [ $i -lt 3 ]; do
		pan=
		tilt=
		i=$(($i + 1))

		pan=$($scc_prefix.PAN_STATUS) && [ "$pan" ] &&
			tilt=$($scc_prefix.TILT_STATUS) && [ "$tilt" ] ||
			continue

		[ $pan -ne 0 ] || [ $tilt -ne 0 ] || return 0

		sleep 1
	done
}

internal_error() {
	echo $1
	__cgi_errhd 500
	exit 1
}

[ -z "$REQUEST_METHOD" ] || __cgi_hdgen yes

# Get options
def_speed=100
speed=$(__qs_getparam speed) && [ "$speed" ] &&
	case $speed in
		*[!0-9]*)
			echo "Invalid speed $speed, using default $def_speed"
			! :
			;;
	esac || speed=$def_speed

# Get pan and tilt pos
if pan_pos=$($scc_prefix.PAN_DEG) && [ "$pan_pos" ]; then
	pan_pos=${pan_pos%.*}
else
	internal_error "Failed to get PAN position from status cache"
fi

if tilt_pos=$($scc_prefix.TILT_DEG) && [ "$tilt_pos" ]; then
	tilt_pos=${tilt_pos%.*}
else
	internal_error "Failed to get TILT position from status cache"
fi

# Run test
# Pan 40 degree in positive direction
$dbus_cmd double:40.0 int32:2 double:0.0 int32:2 int32:0 int32:0 int32:$speed 1>/dev/null 2>&1 ||
	internal_error "Failed to send dbus move command"
wait_idle

# Check position of tilt to determine direction
if [ $tilt_pos -gt -45 ]; then
	$dbus_cmd double:0.0 int32:2 double:-30.0 int32:2 int32:0 int32:0 int32:$speed 1>/dev/null 2>&1 ||
	internal_error "Failed to send dbus move command"
else
	$dbus_cmd double:0.0 int32:2 double:30.0 int32:2 int32:0 int32:0 int32:$speed 1>/dev/null 2>&1 ||
	internal_error "Failed to send dbus move command"
fi
wait_idle

# Go back to original position
$dbus_cmd double:$pan_pos int32:1 double:$tilt_pos int32:1 int32:0 int32:0 int32:$speed 1>/dev/null 2>&1 ||
	internal_error "Failed to send dbus move command"
wait_idle
