#!/bin/sh -e

# CGI parameters
#	hdgen=yes|no			default yes
#	title=yes|no			default no
#	format=long|short|load|vload	default long

. /usr/html/axis-cgi/lib/functions.sh
. /usr/html/axis-cgi/lib/uptime.sh

local whoami__=$(__whoami) cgi_hdgen__=yes title__=no optarg__=long tmpvar__

tmpvar__=$(__qs_getparam hdgen) && {
	[ "$tmpvar__" ] && cgi_hdgen__=$tmpvar__
}
tmpvar__=

tmpvar__=$(__qs_getparam title) && {
	[ "$tmpvar__" ] && title__=$tmpvar__
}
tmpvar__=

tmpvar__=$(__qs_getparam format) && {
	[ "$tmpvar__" ] && optarg__=$tmpvar__
}

__cgi_hdgen $cgi_hdgen__
__title $title__ $whoami__
__do_uptime $optarg__

exit 0
