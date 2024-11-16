#!/bin/sh -e

# CGI parameters
#       action=list|info|type      default list
#	return_page=<return page path>


. /usr/html/axis-cgi/lib/functions.sh
. /usr/html/axis-cgi/lib/language/language_viewer.sh

ACTION=list
RETURN_PAGE=

TMPVAR=$(__qs_getparam action) && {
        [ "$TMPVAR" ] && ACTION=$TMPVAR
}
TMPVAR=$(__qs_getparam return_page) && {
        [ "$TMPVAR" ] && RETURN_PAGE=$TMPVAR
}
unset TMPVAR

[ -z "$ACTION" ] && {
	ACTION="list"
}

case $ACTION in
	list)
		__list_language
		;;
	info)
		__info
		;;
	type)
		__type
		;;
	*)
		__cgi_errhd 400
		;;
esac

unset ACTION
unset RETURN_PAGE

exit 0
