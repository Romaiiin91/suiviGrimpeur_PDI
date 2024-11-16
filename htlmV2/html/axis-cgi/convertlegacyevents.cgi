#!/bin/sh

. /usr/html/axis-cgi/lib/functions.sh

# CGI compliant by default
local cgi_hdgen=yes msg= tag_open= tag_close=

tag_open='<ConvertLegacyEventsResponse
xmlns="http://www.axis.com/vapix/http_cgi/convertlegacyevents"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.axis.com/vapix/http_cgi/convertlegacyevents
http://www.axis.com/vapix/http_cgi/convertlegacyevents.xsd">'
tag_close='</ConvertLegacyEventsResponse>'

report_status() {
	# Arguments:
	#       $1      generate header if arg value is 'yes'
	#       $2      status code
	#       $3      success/error message
	if [ "$1" = yes ]; then
		__cgi_errhd_ct $2 "text/xml" "$3"
	else
		echo "$3"
	fi
}

if /bin/parhand2actionengine -c; then
	msg="<GeneralSuccess/>"
else
	msg="<GeneralError/>"
fi

report_status $cgi_hdgen "200" "$tag_open$msg$tag_close"

exit 0
