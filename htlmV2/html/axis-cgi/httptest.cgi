#!/bin/sh -e


# exec 2>/dev/console

. /usr/html/axis-cgi/lib/functions.sh


cmd="curl -s -S -g --anyauth -f --connect-timeout 10"
address=
status=

user_login=
proxy=
proxy_login=

handle_bad_input() {
	if [ $cgi_hdgen = yes ]; then
		__cgi_errhd 400 "$1"
	else
		echo "$1"
	fi
	exit 1
}

tmp=$(__qs_getparam generate_header) || tmp=
[ -z "$tmp" ] || cgi_hdgen=$tmp

tmp=$(__qs_getparam address) ||
	handle_bad_input "Server address missing"
[ -z "$tmp" ] || address=$tmp

username=$(__qs_getparam username)		|| username=
password=$(__qs_getparam password)		|| password=
proxyaddress=$(__qs_getparam proxyaddress)	|| proxyaddress=
proxyport=$(__qs_getparam proxyport)		|| proxyport=
proxylogin=$(__qs_getparam proxylogin)		|| proxylogin=
proxypassword=$(__qs_getparam proxypassword)	|| proxypassword=
validate_cert=$(__qs_getparam validatecert)	|| validate_cert=0

[ -z "$proxyaddress" ]	|| proxy="-x $proxyaddress"
[ -z "$proxyport" ]	|| proxy="$proxy:$proxyport"

[ -z "$proxylogin" ]	|| proxy_login="$proxylogin"
[ -z "$proxypassword" ]	|| proxy_login="$proxy_login:$proxypassword"

logmsg=$proxy

[ -z "$proxy_login" ] || {
	proxy_login=$(__escape_single_quotes "$proxy_login")
	proxy_login="-U $proxy_login"
	logmsg="$logmsg -U '$proxylogin:<passwd>'"
}

protocol=${address%%://*}

[ -z "$username" ] || user_login="$username"
[ -z "$password" ] || user_login="$user_login:$password"
[ -z "$user_login" ] || {
	user_login=$(__escape_single_quotes "$user_login")
	user_login="-u $user_login"
	logmsg="$logmsg -u '$username:<passwd>'"
}

logmsg="$cmd $logmsg $address"
#logger -p info "$logmsg"

params="$user_login $proxy $proxy_login"


cmd="$cmd $params"

[ "$protocol" != https ] || {
	case $validate_cert in
		*[!0-9]*)
			handle_bad_input "validatecert must be a positive integer"
			;;
		*)
			[ $validate_cert -ge 0 ] && [ $validate_cert -le 1 ] ||
				handle_bad_input "validatecert must be in the range [0..1]"
			;;
	esac
	if [ $validate_cert -eq 1 ]; then
		ca_path=$(ae_client_cert -p http -t http_test22234) ||
			error "Certificate set failed!"
		cmd="$cmd --capath $ca_path"
	else
		cmd="$cmd --insecure"
	fi
}

cmd="$cmd $address"

#logger -p info "$cmd"

set +e
validateaddr $address
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
		cmd="$cmd $address"
		;;
esac
set -e

error=$(eval $cmd 2>&1) || {
	tmp=$(printf "%s" "$error" |
		tr '\r\n' ' ' |
		tr -d '"' |
		sed -re 's/curl://' -e 's/\([0-9]+\)//' \
			-e 's/\.[[:blank:]].*/\./' -e 's/^[[:blank:]]+//' \
			-e 's/[[:blank:]]+$//')

	if [ $cgi_hdgen = yes ]; then
		__cgi_errhd 500 "$tmp."
	else
		echo "$tmp."
	fi
	exit 1
}
if [ $cgi_hdgen = yes ]; then
	__cgi_errhd 200 "Test successful."
else
	echo "Test successful."
