# Global variables
CGI_HDGEN=yes
SC_PHC="parhandclient --nocgi --nolog --casesensitive"
SC_DBUS="dbus-send --print-reply=literaly --system --type=method_call 	--dest=com.axis.Streamer"
SC_STR_WRONG_NUM_ARG='Wrong number of arguments'

SC_COM_AXIS=/com/axis/Streamer
SC_AXIS_COM=com.axis.Streamer

SC_ERR_ADDGRP=1
SC_ERR_ADDSET=2
SC_ERR_REMGRP=3
SC_ERR_LIST=5
SC_ERR_UPSAVE=6
SC_ERR_UPSET=7

# NOTE:sc_update() uses signal handler to remove
# all the temporary files created
SC_TMPFILES=

SC_SELF=$(__whoami)
SC_LOG="logger -t '$SC_SELF[$$]'"

sc_err() {
	echo -e "$@\r"

	# Send to syslog as well.
	$SC_LOG -p user.err -t -- $*

	exit 1
}

sc_output_element() {
	[ $# -gt 1 ] && [ "$2" ] ||
		sc_err sc_output_element: $SC_STR_WRONG_NUM_ARG
	local start= end=

	# type=$1 name=$2
	case $1 in
		'<')
			start="<$2"
			end='>'
			;;
		'>')
			start="</$2"
			end='>'
			;;
		'<>')
			start="<$2"
			end=' />'
			;;
		*)
			sc_err sc_output_element: Bad type argument
			;;
	esac
	shift; shift

	echo -n "$start"

	local par= val=
	while [ $# -gt 1 ]; do
		par=$1; shift
		val=$1; shift
		echo -n " $par=\"$val\""
	done

	echo "$end"
}

type_is_int() {
	[ $# -eq 1 ] && [ "$1" ] ||
		sc_err type_is_int: $SC_STR_WRONG_NUM_ARG

	case $1 in
		'')
			return 1
			;;
		*[!0-9]*)
			return 2
			;;
	esac
}

sc_gen_err_element() {
	[ $# -eq 1 ] && [ "$1" ] ||
		sc_err sc_gen_err_element: $SC_STR_WRONG_NUM_ARG

	type_is_int $1 || sc_err fail_add_response: 		"called with bad value \"$1\" for errcode"
	local description=

	case $1 in
		1)
			description="Could not add streamcache because the maximum 					number of streamcaches has been reached"
			;;
		2)
			description="Could not set parameter during add"
			;;
		3)
			description="Id not found during remove"
			;;
		4)
			description="Group not found during remove"
			;;
		5)
			description="Service not available during list"
			;;
		6)
			description="Id not found during update"
			;;
		7)
			description="Could not set parameter during update"
			;;
	esac

	sc_output_element '<>' error errorcode $1 description "$description"
}

sc_dbus_get_state() {
	[ $# -eq 1 ] && [ "$1" ] ||
		sc_err sc_dbus_get_state: $SC_STR_WRONG_NUM_ARG

	local number=${1#S}

	type_is_int $number || return 1

	$SC_DBUS $SC_COM_AXIS/StreamCache/$number 		org.freedesktop.DBus.Properties.Get 		string:$SC_AXIS_COM.StreamCache 		string:State 2>/dev/null |
		sed -nre 's/^[[:blank:]]*variant[[:blank:]]+int32[[:blank:]]+(-?[[:digit:]]+)$/\1/p' ||
		return 1
}

# Set values to parameters
sc_add() {
	#	Returns
	#	0: success
	#	1: Could not add group
	[ $# -eq 3 ] || sc_err sc_add: $SC_STR_WRONG_NUM_ARG

	type_is_int "$2" || sc_err sc_add: 		"called with bad value \"$4\" for requestedlengthtime"

	local group= id= state=

	# $SC_PHC addgroup <group> <template>
	group=$($SC_PHC addgroup StreamCache streamcache) ||
		return $SC_ERR_ADDGRP
	id=${group#StreamCache.}

	# Add parameters to the created group.And get state of streamcache
	# via dbus call. If returned state is failing delete the group and
	# return proper errorcode
	if $SC_PHC set $group.Enabled no 		set $group.Options "$1" 		set $group.RequestedLengthTime "$2" 		set $group.Enabled "$3" 2>/dev/null &&
		state=$(sc_dbus_get_state "$id") &&
		[ "$state" ] && [ "$state" != -1 ]; then
		sc_output_element '<>' streamcache id "$id"
	else
		$SC_PHC deletegroup $group 2>/dev/null || :
		return $SC_ERR_ADDSET
	fi
}

# Remove streamcache or exit with proper errorcode
sc_remove_group() {
	[ $# -eq 1 ] && [ "$1" ] || sc_err sc_remove: $SC_STR_WRONG_NUM_ARG

	$SC_PHC deletegroup StreamCache.$1 2>/dev/null || return $SC_ERR_REMGRP
}

sc_get_maxcount() {
	local maxcount=

	maxcount=$($SC_PHC get StreamCache.MaxGroups - RAW 2>/dev/null) ||
		maxcount="not available"

	echo $maxcount
}

sc_list() {
	$SC_DBUS $SC_COM_AXIS/StreamCacheFactory 		$SC_AXIS_COM.StreamCacheFactory.List 2>/dev/null ||
		return $SC_ERR_LIST
}

sc_cleanup() {
	[ -z "$SC_TMPFILES" ] ||
	rm -f $SC_TMPFILES 2>/dev/null
}

# update group parameters or exit with proper errorcode
sc_update() {
	[ $# -eq 7 ] && [ "$1" ] || sc_err sc_update: $SC_STR_WRONG_NUM_ARG

	local state= query= self= value= tmpfile=

	# Save the current parameter values in a file. If the update
	# succeeds update with new parameter values otherwise restore with
	# old parameter values
	# create a temporary file in /tmp directory
	tmpfile=$(mktemp /tmp/$SC_SELF.XXXXXX) || {
		$SC_LOG user.info -- failed to mktemp
		return $SC_ERR_UPSAVE
	}

	# collect all the tempfiles created
	SC_TMPFILES="$SC_TMPFILES $tmpfile"

	# Remove all the tempfiles created
	trap 'sc_cleanup' EXIT TERM

	$SC_PHC getgroup StreamCache.$1 - NAMEVALUESECTIONS >$tmpfile ||
		return $SC_ERR_UPSAVE

	if [ "$3" = y ]; then
		# Options can contain special characters
		# like %26 need to urldecode
		# Return error if printf and/or urldecode fails
		value=$(echo "$2" | urldecode 2>/dev/null) ||
			return $SC_ERR_UPSET
		query="$query set StreamCache.$1.Options '$value'"
	fi

	[ -z "$4" ] || [ "$5" != y ] ||
		query="$query set StreamCache.$1.RequestedLengthTime $4"

	[ -z "$6" ] || [ "$7" != y ] ||
		query="$query set StreamCache.$1.Enabled $6"

	# Return error if query is empty
	[ "$query" ] || return $SC_ERR_UPSET

	if eval $SC_PHC $query 2>/dev/null && state=$(sc_dbus_get_state "$1") &&
		[ "$state" ] && [ "$state" != -1 ]; then

		sc_output_element '<>' streamcache id "$1"
	else
		# restore with the old parameter values
		$SC_PHC setparams $tmpfile

		return $SC_ERR_UPSET
	fi
}
