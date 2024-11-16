#!/bin/sh -e


# exec 2>/dev/console
# set -x

. /usr/html/axis-cgi/lib/functions.sh

error() {
	local msg__

	if [ -z "$@" ]; then
		msg__="unknown error"
	else
		msg__=$(echo "$@" | tr '\r\n' ' ' | tr -d '"')
	fi

	printf '%s' "$msg__"
	__cgi_errhd 500 "$msg__"
	exit 1
}

is_ipv6_address() {
	local rest

	[ $# -eq 1 ] || error "is_ipv6_address: wrong number of arguments"

	rest=${1%:*}

	[ "$rest" != "${rest%:*}" ]
}

validate_server() {

	local f_name=validate_server rest
	local proto host port

	[ $# -eq 5 ] || error "$f_name: wrong number of arguments"

	proto=${4%://*}
	[ "$proto" != "$4" ] || proto=
	[ -z "$proto" ] || [ $proto = $5 ] ||
		error "Unsupported protocol '$proto'!"

	rest=${4#$proto://}

	if is_ipv6_address "$rest"; then
		host=${rest%]:*}

		port=${rest#*]:}
		[ "$port" != "$host" ] || port=

		[ -z "$port" ] || {
			rest=$host

			host=${rest#\[}
		}
	else
		host=${rest%:*}

		port=${rest#*:}
		[ "$port" != "$host" ] || port=
	fi

	set +e
	validateaddr $host
	validation_res=$?
	case $validation_res in
		1)
			__cgi_errhd 400 "Invalid localhost address $tmp"
			exit 1
			;;
		2)
			__cgi_errhd 400 "Could not resolve address $tmp"
			exit 1
			;;
		3)
			__cgi_errhd 400 "Error validating address $tmp"
			exit 1
			;;
		*)
			;;
	esac
	set -e

	[ "$host" ] || error "Missing host information!"
	validate_parameters.sh -h $host || error "Invalid host '$host'!"
	[ -z "$port" ] || validate_parameters.sh -p $port ||
		error "Invalid port '$port'!"

	eval $1=\$proto
	eval $2=\$host
	eval $3=\$port
}

escape_param() {
	echo "$1" | sed -re "s/'/'\\\\''/g"
}

mailserver=
from=
to=
subject=
body=
user=
pass=
popserver=
popuser=
poppass=
encryption=
validatecert=

! tmpvar=$(__qs_getparam mailserver) || [ -z "$tmpvar" ] ||
	mailserver=$tmpvar
[ "$mailserver" ] || error 'Missing parameter: "mailserver"'

validate_server smtpproto smtphost smtpport "$mailserver" smtp

! tmpvar=$(__qs_getparam port) || {
	[ -z "$tmpvar" ] || [ -z "$smtpport" ] ||
		error "SMTP port '$smtpport' already defined in the URL!"
	smtpport=$tmpvar
}
[ "$smtpport" ] || error 'Missing SMTP port number'

tmpvar=
! tmpvar=$(__qs_getparam from) || [ -z "$tmpvar" ] || from=$tmpvar
[ "$from" ] || error 'Missing parameter: "from"'
validate_parameters.sh -e $from || error "Invalid email address '$from'!"

tmpvar=
case "$to" in
	*+*)
		to=$(echo $to | sed s/+/' '/g)
		;;
esac
! tmpvar=$(__qs_getparam to) || [ -z "$tmpvar" ] || to=$tmpvar
[ "$to" ] || error 'Missing parameter: "to"'
case "$to" in
	*' '*)
		to=$(echo $to | sed s/' '/+/g)
		;;
esac
validate_parameters.sh -e $to || error "Invalid email address '$to'!"

tmpvar=
! tmpvar=$(__qs_getparam subject) || [ -z "$tmpvar" ] || subject=$tmpvar

tmpvar=
! tmpvar=$(__qs_getparam body) || [ -z "$tmpvar" ] || body=$tmpvar

tmpvar=
! tmpvar=$(__qs_getparam user) || [ -z "$tmpvar" ] || user=$tmpvar

tmpvar=
! tmpvar=$(__qs_getparam pass) || [ -z "$tmpvar" ] || pass=$tmpvar

[ -z "$user" ] || [ "$pass" ] ||
	error 'Missing password for SMTP authentication!'
[ -z "$pass" ] || [ "$user" ] ||
	error 'Missing user name for SMTP authentication!'

tmpvar=
! tmpvar=$(__qs_getparam popserver) || [ -z "$tmpvar" ] || popserver=$tmpvar

[ -z "$popserver" ] || validate_server popproto pophost popport $popserver pop3

! tmpvar=$(__qs_getparam popport) || {
	[ -z "$tmpvar" ] || [ -z "$popport" ] ||
		error "POP port already defined in the URL string '$popport'!"
	popport=$tmpvar
}
[ -z "$popserver" ] || [ "$popport" ] || error 'Missing POP port number!'

tmpvar=
! tmpvar=$(__qs_getparam popuser) || [ -z "$tmpvar" ] || popuser=$tmpvar

tmpvar=
! tmpvar=$(__qs_getparam poppass) || [ -z "$tmpvar" ] || poppass=$tmpvar

[ -z "$popserver" ] || [ "$popuser" ] ||
	error 'Missing username for POP authentication!'
[ -z "$popserver" ] || [ "$poppass" ] ||
	error 'Missing password for POP authentication!'

tmpvar=
! tmpvar=$(__qs_getparam encryption) || [ -z "$tmpvar" ] || encryption=$tmpvar

tmpvar=
! tmpvar=$(__qs_getparam validatecert) || [ -z "$tmpvar" ] ||
	validatecert=$tmpvar

cmd=smtpwrapper.sh

cmd="$cmd -r \"$to\""
cmd="$cmd -s $from"

[ -z "$user" ] || {
	user=$(__escape_single_quotes "$user")
	cmd="$cmd -u $user"
}
[ -z "$pass" ] || {
	pass=$(__escape_single_quotes "$pass")
	cmd="$cmd -p $pass"
}
[ -z "$popuser" ] || {
	popuser=$(__escape_single_quotes "$popuser")
	cmd="$cmd -t $popuser"
}
[ -z "$poppass" ] || {
	poppass=$(__escape_single_quotes "$poppass")
	cmd="$cmd -v $poppass"
}
[ -z "$subject" ] || {
	subject=$(__escape_single_quotes "$subject")
	cmd="$cmd -i $subject"
}
[ -z "$body" ] || {
	body=$(__escape_single_quotes "$body")
	cmd="$cmd -m $body"
}

[ -z "$smtpport" ] || cmd="$cmd -o \"$smtpport\""
[ -z "$pophost" ] || cmd="$cmd -h \"$pophost\""
[ -z "$popport" ] || cmd="$cmd -z \"$popport\""
[ -z "$encryption" ] || cmd="$cmd -k \"$encryption\""
[ -z "$validatecert" ] || cmd="$cmd -c \"$validatecert\""

cmd="$cmd -q -a \"$smtphost\""
error=$(eval $cmd 2>&1) || {
	__cgi_errhd 500 "$error"
	exit 1
}
__cgi_errhd 200 "Test successful."
