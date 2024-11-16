#!/bin/sh -e


[ ! -w /dev/console ] || exec 2>/dev/console

SUPATH=$PATH:/sbin:/usr/sbin

[ -r /usr/html/axis-cgi/lib/functions.sh ] || {
	printf "Missing or unreadable helper lib!\n"
	exit 1
}
. /usr/html/axis-cgi/lib/functions.sh ||
	printf "Failed to source helper lib!\n"

HTML=yes
FIMAGE=/var/fimage

! tmpvar=$(__qs_getparam html) || [ -z "$tmpvar" ] || HTML=$tmpvar
! tmpvar=$(__qs_getparam action) || [ -z "$tmpvar" ] || action=$tmpvar

printf_filtered() {
	[ "$HTML" = no ] || printf "$@"
}

fail() {

	if [ $# -eq 1 ]; then
		[ -z "$1" ] || {
			__cgi_hdgen "$HTML" text/xml
			printf_filtered "<reply result=\"error\">"
			printf_filtered "<error message=\"$1!\"/>"
			printf_filtered "</reply>"
		}
	fi

	[ ! -e $FIMAGE ] || rm $FIMAGE ||
		printf "Failed to remove image!\r\n"

	PATH=$SUPATH init 3
	exit 1
}

return_result() {
	__cgi_hdgen "$HTML" text/xml
	printf_filtered "<reply result=\"$1\">"
	printf_filtered "</reply>"
}

set_content_type() {
	[ $# -gt 2 ] || fail "set_content_type: Missing arguments"

	[ "$1" ] || fail "No content type supplied"
	[ "$2" ] || fail "No disposition supplied"
	[ "$3" ] || fail "No content length supplied"

	printf_filtered "Content-type: %s\r\n" "$1"
	printf_filtered "Content-Disposition: %s\r\n" "$2"
	printf_filtered "Content-Length: %d\r\n" "$3"

	printf_filtered "\r\n"
}

waitfor() {

	[ $# -eq 1 ] || fail "Not enough arguments to function"
	[ "$1" ] || fail "No condition supplied"

	while eval $1; do
		sleep 1
	done
}

filesize() {
	local ign sz

	while read ign ign ign ign sz ign; do
		printf $sz
		break
	done <<-EOF
		$(ls -l $1)
	EOF
}

init_generation() {
	PATH=$SUPATH init 4 || fail "Couldn't switch runlevel"

	waitfor 'ps | grep -q "/etc/init\.d/rc[[:blank:]]\+4"'

	PATH=$SUPATH flash2image -o $FIMAGE  >/dev/null &
	return_result ok
}

check_generation() {
	if eval 'ps | grep -v grep | grep -q flash2image'; then
		return_result "in progress"
	else
		[ -e $FIMAGE ] || fail "Failed to generate firmware"
		return_result done
	fi
}

output_firmware() {
	local length

	[ -e $FIMAGE ] || fail "Failed to download firmware"

	length=$(filesize $FIMAGE)
	set_content_type "application/octet-stream" "attachment; filename=\"customfirmware.bin\""  "$length"

	cat $FIMAGE || fail "Couldn't send image"
	rm $FIMAGE || fail "Couldn't remove image"

	PATH=$SUPATH init 3 || fail "Couldn't restore runlevel"
}

case $action in
	init)
		init_generation
		;;
	check)
		check_generation
		;;
	get)
		output_firmware
		;;
	*)
		fail "Unknown action"
		;;
esac
