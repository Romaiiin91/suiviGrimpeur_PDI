#! /bin/sh

# Script that tests if tcp/udp ports are in use.
# Usage: http://<camera_ip>/axis-cgi/operator/porttest.cgi?proto=tcp&ports=23,97,128,3021,...
#
# CGI parameters				default
#	proto=udp|tcp
#	ports=<portnr>[,<portnr>[,...]]
#	generate_header=yes|no			yes

# Error output to console
# exec 2> /dev/console

. /usr/html/axis-cgi/lib/functions.sh

local proto= ports= generate_header=
local proto_arg= usedports= errmsg= p1= p2=

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

proto=$(__qs_getparam proto) || :
if [ "$proto" = tcp ]; then
	proto_arg=-t
elif [ "$proto" = udp ]; then
	proto_arg=-u
else
	status 400 "invalid proto='$proto'"
fi

ports=$(__qs_getparam ports) || :
if [ "$ports" ]; then
	if [ $(expr "$ports" : '[0-9,]\+') -ne ${#ports} ]; then
		status 400 "invalid 'ports' value '$ports'"
	fi
else
	status 400 "missing 'ports' parameter"
fi

# Make ports a proper list.
ports=$(echo $ports | tr ',' ' ') || :

# Find which ports have listening sockets.
usedports=$(netstat -l -n $proto_arg | \
		sed -rne 's/.*:([0-9]+).*/\1/g p' | \
		sort -nu)

for p1 in $usedports; do
	for p2 in $ports; do
		if [ $p1 = $p2 ]; then
			errmsg="${errmsg}Port $p2 is in use! "
		fi
	done
done

status 200 "$errmsg"
