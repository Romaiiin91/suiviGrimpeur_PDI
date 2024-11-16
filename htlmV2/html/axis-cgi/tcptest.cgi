#!/bin/sh

. /usr/html/axis-cgi/lib/functions.sh

# CGI compliant by default
local cgi_hdgen=yes msg= status=
local addr__ port__

report_status() {
	# Arguments:
	#       $1      generate header if arg value is 'yes'
	#       $2      status code
	#       $3      success/error message
	if [ "$1" = yes ]; then
		__cgi_errhd $2 "$3"
	else
		echo "$3"
	fi
}

addr__=$(__qs_getparam address) || {
	report_status $cgi_hdgen 400 "Please specify host name or address"
	exit 1
}

set +e
validateaddr $addr__
validation_res=$?
case $validation_res in
	1)
		report_status "$cgi_hdgen" 400 "Invalid localhost address $tmp"
		exit 1
		;;
	2)
		report_status "$cgi_hdgen" 400 "Could not resolve address $tmp"
		exit 1
		;;
	3)
		report_status "$cgi_hdgen" 400 "Error validating address $tmp"
		exit 1
		;;
	*)
		;;
esac
set -e

port__=$(__qs_getparam port) || {
	report_status $cgi_hdgen 400 "Please specify a TCP port number"
	exit 1
}

msg="Test successful."
status=200

res=$(echo TEST | nc -w 10 $addr__ $port__ 2>&1) || {
	# If nc failed sending teststring, report error message
	msg=$(echo $res | sed -re 's/(nc:[[:space:]]*)?(socket:[[:space:]]*)?(.*)/\3/')
	status=500
}

report_status $cgi_hdgen $status "$msg"

exit 0
