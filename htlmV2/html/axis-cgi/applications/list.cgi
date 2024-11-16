#!/bin/sh

# CGI parameters
#       reload_page=yes|no (default: no)

# Output errors to the console if user can write to it.
[ ! -w /dev/console ] || exec 2>/dev/console


RELOAD_PAGE=no

. /usr/html/axis-cgi/lib/functions.sh
. /usr/html/axis-cgi/lib/adp.sh

! tmpvar=$(__qs_getparam reload_page) || [ -z "$tmpvar" ] ||
	RELOAD_PAGE=$tmpvar

__list_adp_packages

