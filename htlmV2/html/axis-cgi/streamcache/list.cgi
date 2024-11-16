#!/bin/sh

## Add, update, remove and list streamcaches.
## Adding, removing, updating streamcaches are handled via parhandclient.
## List action is performed via dbus call to the monolith.
##
##	Usage:
##	add.cgi?requestedlengthtime=<int>
##		[&options=<string>]
##		[&enabled={yes|no}]
##	Default options=''
##	add.cgi returns the id of the group created.
##
##	remove.cgi?id=<returned id>
##	ex: remove.cgi?id=s1
##
##	list.cgi?action=list
##
##	update.cgi?id=<returned id>&...
##	ex: update.cgi?id=s1
##
########################################

# global variables
STR_XML_START='<?xml version="1.0"?> <root>'
STR_XML_END='</root>'

SET_OPTIONS=
SET_LENGTH=
SET_ENABLED=

ID=
LENGTH=
OPTIONS=
ENABLED=

CGI_HDGEN=yes


. /usr/html/axis-cgi/lib/functions.sh
. /usr/html/axis-cgi/streamcache/streamcache_functions.sh

# helpers
#
type_check_int() {
	[ $# -gt 1 ] && [ "$1" ] ||
		sc_err type_check_int: $SC_STR_WRONG_NUM_ARG

	local value=$1
	local name=${2:-'(unkown)'}
	local func=${3:-'(unkown)'}

	type_is_int "$value" ||
		sc_err $func called with argument $name: \"$value\"
}

# responses
#
output_response() {
	[ $# -gt 1 ] && [ "$1" ] ||
		sc_err output_add_response: $SC_STR_WRONG_NUM_ARG

	local name=$1 output=$2
	shift; shift

	__cgi_hdgen $CGI_HDGEN
	echo "$STR_XML_START"
	sc_output_element '<' $name success true $*
	[ -z "$output" ] || echo "$output"
	sc_output_element '>' $name
	echo "$STR_XML_END"
}

fail_response() {
	[ $# -gt 1 ] && [ "$1" ] && [ "$2" ] ||
		sc_err fail_response: $SC_STR_WRONG_NUM_ARG

	type_check_int $2 errcode fail_response

	local name=$1 errcode=$2
	shift; shift

	__cgi_hdgen $CGI_HDGEN
	echo "$STR_XML_START"
	sc_output_element '<' $name success false $*
	sc_gen_err_element $errcode
	sc_output_element '>' $name
	echo "$STR_XML_END"

	exit 1
}

fail_http() {
	# errcode=$1 errmsg=$2

	[ $# -gt 1 ] || sc_err fail_http: $SC_STR_WRONG_NUM_ARG

	type_check_int "$1" errcode fail_http

	__cgi_errhd $*

	exit 1
}

get_id() {
	eval "$1"="$(__qs_getparam id)" || fail_http 400 "Missing parameter 'id'"
}

get_update_parameters(){
	SET_OPTIONS=y
	SET_LENGTH=y
	SET_ENABLED=y

	get_id ID

	LENGTH=$(__qs_getparam requestedlengthtime) && [ "$LENGTH" ] ||
		SET_LENGTH=n

	ENABLED=$(__qs_getparam enabled) && [ "$ENABLED" ] ||
		SET_ENABLED=n

	OPTIONS=$(__qs_getparam options) ||
		SET_OPTIONS=n
}

# Get CGI parameters in streamcache LENGTH, ENABLED
# ..or exit with apropriate message
get_add_parameters() {
	LENGTH=$(__qs_getparam requestedlengthtime) ||
		fail_http 400 "Missing parameter 'requestedlengthtime'"
	type_is_int $LENGTH ||
		fail_http 400 "Parameter 'requestedlengthtime' must be integer"
	ENABLED=$(__qs_getparam enabled) || ENABLED=yes
	OPTIONS=$(__qs_getparam options) || OPTIONS=
}

handle_action() {
	[ $# -eq 1 ] && [ "$1" ] || sc_err handle_action: $SC_STR_WRONG_NUM_ARG
	local action=$1 output= response= maxcount=

	response=streamcache${1}response

	case "$action" in
		add)
			get_add_parameters

			output=$(sc_add "$OPTIONS" 				"$LENGTH" "$ENABLED") ||
				fail_response $response $?

			output_response $response "$output"
			;;
		update)
			get_update_parameters

			output=$(sc_update "$ID" 					"$OPTIONS" "$SET_OPTIONS" 					"$LENGTH" "$SET_LENGTH" 					"$ENABLED" "$SET_ENABLED") ||
					fail_response $response $?

			output_response $response "$output"
			;;
		remove)
			get_id ID

			output=$(sc_remove_group "$ID") ||
				fail_response $response $?

			output_response $response "$output"
			;;
		list)
			output=$(sc_list) || fail_http 500 "List not available"

			maxcount=$(sc_get_maxcount) || :
			echo "maxcount $maxcount"
			if [ $maxcount -eq 0 ]; then
				output_response $response "$output"
			else
				output_response $response "$output" maxcount 					$maxcount
			fi
			;;
		*)
			fail_http 400 "Invalid value for parameter action:" 				"\"$action\""
			;;
	esac

	exit 0
}

action=$(__qs_getparam action) || {
	action=${0##*/}
	action=${action%.cgi}
}

[ "$action" ] && [ "$action" != streamcache ] ||
	fail_http 400 "Missing parameter: \"action\""

handle_action $action

# Not reached
fail_http 500 "Unexpected failure"

exit 1
