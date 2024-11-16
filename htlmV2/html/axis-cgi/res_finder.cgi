#!/bin/sh -e

# Script that requests valid resolutions and values for max zoom to be written
# in the webpages.
# Usage: <!--#include virtual="/axis-cgi/operator/res_finder.cgi?action=ACTION&hdgen=HDGEN" -->
#
# CGI parameters
#	ACTION  default_res|max_parhand_resolution|max_resolution|max_resolution_standard|max_zoom_resolutions|max_zoom_values|resolutions
#	HDGEN	yes|no  (optional, default is 'yes')

. /usr/html/axis-cgi/lib/functions.sh

self=${0##*/}[$$]

fail() {
	local err_code=$1

	[ $# -gt 0 ] || {
		__cgi_errhd 500 "$self: missing argument(s)"
		exit 1
	}
	shift

	__cgi_errhd $err_code "$self: $*"
	exit 1
}

# Generate http header unless specifically turned off
hdgen=$(__qs_getparam hdgen) || :
[ "$hdgen" ] || hdgen=yes
case "$hdgen" in
	yes|no)
		;;
	*)
		fail 400 "Invalid hdgen value '$hdgen'"
		;;
esac

# Get the requested data - return error if fail
action=$(__qs_getparam action) || :
case "$action" in
	default_res|max_parhand_resolution|max_resolution|max_resolution_standard|max_zoom_resolutions|max_zoom_values|resolutions)
		;;
	*)
		fail 400 "Invalid action '$action'"
		;;
esac

retval=$(res_finder --$action) ||
	fail 500 "Could not retrieve action '$action'"

# Return the requested data
__cgi_hdgen $hdgen
echo $retval
