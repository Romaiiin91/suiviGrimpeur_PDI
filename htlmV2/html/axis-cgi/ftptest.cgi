#!/bin/sh -e


# exec 2>/dev/console

. /usr/html/axis-cgi/lib/functions.sh

cgi_hdgen=yes
tmp=
passive=
command=
raw_ip=
status=
testf=

user_login=
timeout=10

cleanup() {
	[ -z "$testf" ] || rm -f $testf
}

trap 'cleanup' EXIT

error() {
	logger -p err -t "$self" -s "$@"
	__cgi_errhd 500 "$@"
	exit 1
}

report_status() {

	local f_name=report_status

	case "$1" in
		[Yy][Ee][Ss])
			;;
		[Nn][Oo])
			;;
		*)
			error "$f_name: Generate header must be yes or no"
			;;
	esac
	case "$2" in
		*[!0-9]*)
			error "$f_name: Status code must be a positive integer"
			;;
	esac
	[ "$3" ] || error "$f_name: No status message supplied"
	if [ "$1" = yes ]; then
		__cgi_errhd $2 "$3"
	else
		echo "$3"
	fi
}

! tmp=$(__qs_getparam generate_header) || [ -z "$tmp" ] || cgi_hdgen=$tmp

testf=$(mktemp /tmp/.test.XXXXXX) || {
	report_status $cgi_hdgen 500 "mktemp failure"
	exit 1
}
echo TEST >>$testf

tmp=$(__qs_getparam address) && [ "$tmp" ] || {
	report_status $cgi_hdgen 400 "No address specified"
	exit 1
}

host=$tmp
set +e
validateaddr $tmp
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

! tmp=$(__qs_getparam portnbr) || [ -z "$tmp" ] || host=$host:$tmp

! tmp=$(__qs_getparam uploadpath) || [ -z "$tmp" ] || host="$host/$tmp/"

! tmp=$(__qs_getparam username) || [ -z "$tmp" ] || {
	user_login="$tmp"
	msg_user_login="$tmp"
}

! tmp=$(__qs_getparam password) || [ -z "$tmp" ] || {
	user_login=$(__escape_single_quotes "$user_login:$tmp")
	msg_user_login="$msg_user_login:****"
}

user_login="-u $user_login"
msg_user_login="-u '$msg_user_login'"

! tmp=$(__qs_getparam passive) || [ -z "$tmp" ] || passive=$tmp

ftp_port="-P -"
[ "$passive" != yes ] || ftp_port=

command="curl -s -S --connect-timeout $timeout $user_login -T $testf ftp://$host $ftp_port"
msg_command="curl -s -S --connect-timeout $timeout $msg_user_login -T $testf
ftp://$host $ftp_port"

error=$(eval $command 2>&1) || {
	tmp=$(printf "%s" "$error" |
		tr '\r\n' ' ' |
		tr -d '"' |
		sed -re 's/curl://' -e 's/\([0-9]+\)//' \
			-e 's/\.[[:blank:]].*/\./' -e 's/^[[:blank:]]+//' \
			-e 's/[[:blank:]]+$//')

	report_status $cgi_hdgen 500 "$tmp"
	exit 1
}

report_status $cgi_hdgen 200 "Test successful"
