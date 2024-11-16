#!/bin/sh -e

. /usr/html/axis-cgi/lib/functions.sh

ERR_MSG_START="Request failed:"
MIN=0
MAX=100

err() {
	logger -s -p err -t $0 -- "$*" || echo "$0: logger failed!" >&2
	exit 1
}

check_numeric() {
	case $1 in
		*[!0-9]*|'')
			return 1
			;;
	esac
}

check_limits() {
	if ! check_numeric $1 || [ $1 -lt $MIN ] || [ $1 -gt $MAX ]; then
		local msg
		msg="Invalid range [$MIN - $MAX]"
		__cgi_errhd 200 $ERR_MSG_START "$msg"
		err "$msg"
	fi
}

check_camera_limits() {
	if ! check_numeric $1 || [ $1 -lt 1 ] ||
		[ $1 -gt 1 ]; then
		local msg
		msg="Invalid camera [1 - 1]"
		__cgi_errhd 200 $ERR_MSG_START "$msg"
		err "$msg"
	fi
}

check_alpha() {
	case $1 in
		*[!a-zA-Z]*|'')
			return 1
			;;
	esac
}

# ex: send_dbus_msg Brightness 40 1
send_dbus_msg() {
	dbus-send 		--print-reply 		--system 		--type=method_call 		--dest=com.axis.VideoControl /com/axis/VideoControl/$3 		org.freedesktop.DBus.Properties.Set 		string:com.axis.VideoControl string:$1 		variant:int32:$2 >/dev/null
}

param__=$(__qs_getparam param) || :
value__=$(__qs_getparam value) || :
# To be backward compatible, if no camera parameter is present assume
# the first channel should be used.
camera__=$(__qs_getparam camera) && [ "$camera__" ] || camera__=1

if [ "$param__" ] && [ "$value__" ] && check_alpha $param__ ; then
	check_camera_limits $camera__
	check_limits $value__
	case $param__ in
		[Cc]ontrast)
			send_dbus_msg Contrast $value__ $camera__
			;;
		[Bb]rightness)
			send_dbus_msg Brightness $value__ $camera__
			;;
		[Ss]aturation)
			send_dbus_msg Saturation $value__ $camera__
			;;
		*)
			__cgi_errhd 200 $ERR_MSG_START 				"Invalid image parameters"
			exit 1
			;;
	esac
	__cgi_hdgen yes
else
	__cgi_errhd 200 $ERR_MSG_START "Invalid image parameters"
	exit 1
fi
