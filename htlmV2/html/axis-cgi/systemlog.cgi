#!/bin/sh -e

# CGI parameters
#	none
#
# CGI parameters for internal use
#	hdgen=yes|no		default yes
#	title=yes|no		default yes
#	tail=[:digit:]
#	format=text|html	default text
#	appname=[:alpha:]

. /usr/html/axis-cgi/lib/functions.sh
. /usr/html/axis-cgi/lib/systemlog.sh
. /usr/html/axis-cgi/lib/adp.sh

local cgi_hdgen__=yes title__=yes format__=text tail__= log__= tmpvar__=
local tmpfile__= file__=/var/log/messages

__cleanup () {
	rm -f $tmpfile__
}

trap __cleanup EXIT

tmpvar__=$(__qs_getparam hdgen) && {
	[ "$tmpvar__" ] && cgi_hdgen__=$tmpvar__
}
tmpvar__=

tmpvar__=$(__qs_getparam title) && {
	[ "$tmpvar__" ] && title__=$tmpvar__
}
tmpvar__=

tmpvar__=$(__qs_getparam format) && {
	[ "$tmpvar__" ] && format__=$tmpvar__
}
tmpvar__=

tmpvar__=$(__qs_getparam tail) && {
	[ "$tmpvar__" ] && tail__=$tmpvar__
}
tmpvar__=

tmpvar__=$(__qs_getparam appname) && {
	[ "$tmpvar__" ] && appname__=$tmpvar__
}
tmpvar__=

if [ $(__whoami) = accesslog.cgi ]; then
	tmpfile__=$(mktemp /tmp/access.XXXXXX) || {
		__cgi_errhd 500
		exit 1
	}
	/usr/sbin/access_format.sh > $tmpfile__ || {
		__cgi_errhd 500
		exit 1
	}
	file__=$tmpfile__
fi

__cgi_hdgen $cgi_hdgen__

if [ "$tail__" ]; then
    if [ "$appname__" ]; then
        __do_show_logfile_for_appname $appname__ $format__ $title__ $file__ | tail -n $tail__
    else
        __do_show_logfile $format__ $title__ $file__ | tail -n $tail__
    fi
else
     if [ "$appname__" ]; then
         __do_show_logfile_for_appname $appname__ $format__ $title__ $file__
     else
         __do_show_logfile $format__ $title__ $file__
     fi
fi

exit 0
