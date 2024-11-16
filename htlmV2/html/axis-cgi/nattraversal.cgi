#!/bin/sh

# CGI parameters				default
#	action=enable|disable|wait
#	port=<portnr>
#	router=<ip address>
#	generate_header=yes|no			yes

# Error output to console
#exec 2> /dev/console

. /usr/html/axis-cgi/lib/functions.sh

local generate_header= phcnc="parhandclient --nocgi"

generate_header=$(__qs_getparam generate_header) || :

# CGI compliant by default.
[ "$generate_header" ] || generate_header=yes

status () {
	local code= msg=

	if [ $# -lt 1 ]; then
		msg="status: missing function argumen(s)"
		echo "$msg" >&2
		logger -t $(__whoami)[$$] -p user.warn "$msg"
		exit 1
	fi

	code=$1
	shift
	msg="$*"

	if [ $generate_header = yes ]; then
		__cgi_errhd $code "$msg"
	else
		echo "$msg"
	fi

	if [ $code -lt 400 ]; then
		exit 0
	else
		exit 1
	fi
}

do_wait() {
	local sleepTime=2 maxIter=15 iter=0 isActive=

	while [ $iter -lt $maxIter ]; do
		isActive=$($phcnc get \
				Network.UPnP.NATTraversal.Active - RAW) ||
			status 500 "parhandclient failed activating" \
				"NAT traversal"
		[ "$isActive" != yes ] || status 200 OK
		sleep $sleepTime
		iter=$(($iter + 1))
	done
	status 500 "waited $(($maxIter * $sleepTime))s but failed" \
		"activating NAT traversal"
}

local action= port= router=

action=$(__qs_getparam action) || :
[ "$action" ] || status 400 "missing 'action' parameter"

port=$(__qs_getparam port) || :
if [ "$port" ]; then
	$phcnc set System.AlternateBoaPort $port > /dev/null ||
		status 500 "parhandclient failed setting alternate http port"
fi

router=$(__qs_getparam router) || :
if [ "$router" ]; then
	$phcnc set Network.UPnP.NATTraversal.Router $router > /dev/null ||
		status 500 "parhandclient failed setting router ip"
fi

case $action in
	enable)
		$phcnc set Network.UPnP.NATTraversal.Enabled yes > /dev/null ||
			status 500 "parhandclient failed enabling" \
				"NAT traversal"
		do_wait
		;;
	wait)
		do_wait
		;;
	disable)
		$phcnc set Network.UPnP.NATTraversal.Enabled no > /dev/null ||
			status 500 "parhandclient failed disabling NAT" \
				"traversal"
		status 200 OK
		;;
	*)
		status 400 "invalid action=$action"
		;;
esac

status 400 "NAT traversal could not be activated"
