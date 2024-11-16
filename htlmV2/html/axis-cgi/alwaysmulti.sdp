#!/bin/sh

. /lib/rcscripts/sh/rc-std2parse.sh

. /usr/html/axis-cgi/lib/functions.sh
LOG_E="logger -s -t${0##*/}[$$] -perr"

CONF_FILE=/etc/dynamic/mcast-always.conf
DESCRIPTION=
DBUS="dbus-send --system --dest=com.axis.Streamer --print-reply --type=method_call"
DOBJ_PATH=/com/axis/Streamer/RTP
METHOD_PREFIX=com.axis.Streamer.RTP

log_and_exit() {
	$LOG_E $1
	__cgi_errhd ${2:-500} $1
	exit 1
}

add_description() {
	local f=add_description id pipeline_desc media_desc

	case $1 in
		audio|video)
			;;
		*)
			$LOG_E "$f: invalid argument."
			return 1
			;;
	esac

	id=$(sed -rne "s|^$CHANNEL[[:blank:]]+$1[[:blank:]]+id(.*)$|\1|p" $CONF_FILE) || {
		$LOG_E "$f: could not retrieve session id."
		return 1
	}
	case $id in
		*[!0-9]*)
			$LOG_E "$f: bad session id."
			return 1
			;;
	esac

	pipeline_desc=$($DBUS $DOBJ_PATH/Pipeline/$id \
		$METHOD_PREFIX.Pipeline.Describe |
		sed -r  -e '/^.*method[[:blank:]]+return/d' \
			-e '/^"$/d' \
			-e '/^$/d' \
			-e 's/^[[:blank:]]+string[[:blank:]]+"(.*)$/\1/') &&
	[ "$pipeline_desc" ] || return 1

	if [ -z "$DESCRIPTION" ]; then
		DESCRIPTION=$pipeline_desc
	else
		media_desc="${pipeline_desc##*
m=}"
		DESCRIPTION="${DESCRIPTION}
m=$media_desc"
	fi
}

CHANNEL=$(__qs_getparam camera) && [ "$CHANNEL" ] ||
	log_and_exit "failed to retrieve the camera parameter from URL."

std2parse /etc/sysconfig/rtp.conf

[ $CHANNEL -gt 0 ] && [ $CHANNEL -le $STD2_NETWORK_RTP_NBROFRTPGROUPS ] ||
	log_and_exit "the 'camera' parameter should be at least 1 and maximum \
$STD2_NETWORK_RTP_NBROFRTPGROUPS." 400

CHANNEL=$(($CHANNEL - 1))

eval VIDEO_ENABLED=\$STD2_NETWORK_RTP_R${CHANNEL}_ALWAYSMULTICASTVIDEO
eval AUDIO_ENABLED=\$STD2_NETWORK_RTP_R${CHANNEL}_ALWAYSMULTICASTAUDIO

[ "$VIDEO_ENABLED" = yes ] || [ "$AUDIO_ENABLED" = yes ] ||
	log_and_exit "no sdp description to provide; \
Always Multicast is not started on this channel." 400

[ -r $CONF_FILE ] ||
	log_and_exit "$CONF_FILE is not readable, can't provide sdp description."

[ ! "$VIDEO_ENABLED" = yes ] || add_description video ||
	log_and_exit "failed to retrieve video sdp description."

[ ! "$AUDIO_ENABLED" = yes ] || add_description audio ||
	log_and_exit "failed to retrieve audio sdp description."

__cgi_hdgen yes application/sdp && echo "$DESCRIPTION" || {
	$LOG_E "failed to send HTTP packet."
	exit 1
}
