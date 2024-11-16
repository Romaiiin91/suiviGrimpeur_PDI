#!/bin/sh

. /usr/html/axis-cgi/lib/functions.sh

trap 'rm -rf $tmpdir' EXIT
PRG=$(__whoami)

croak() {
	logger -t$PRG -pcrit "$*"
	__cgi_errhd 500 "Error while compiling Server Report: $*!"
	exit 1
}

start_keep_alive() {
	while :; do
		sleep 1
		printf " "
	done
}

stop_keep_alive() {
	kill $1 2>/dev/null || croak "Failed to kill process '$1'!"
}

generate_report() {
	local keep_alive_proc

	[ $# -gt 0 ] && [ "$1" ] ||
		croak "Bad destination file '$1' argument!"

	start_keep_alive &
	keep_alive_proc=$!

	/usr/html/axis-cgi/gen_serverreport.sh >$1

	stop_keep_alive $keep_alive_proc
}

construct_and_send_zip() {
	local p mac zipname

	[ $# -gt 1 ] && [ "$1" ] && [ -d "$1" ] ||
		croak "Bad directory '$1' argument!"

	p=Properties.System.SerialNumber
	mac=$(parhandclient --nocgi get $p - RAW) ||
		croak "Failed to get parameter $p!"

	zipname=Axis_SR_$(date '+%Y%m%d_%H%M%S')_$mac.zip

	cd $1
	shift

	zip -jq $zipname $* || croak "Failed to perform zip operation!"

	printf "Content-Type: application/zip\r
Content-Disposition: attachment; filename=%s\r
Expires: Thu, 01 Dec 1994 16:00:00 GMT\r\n\r\n" $zipname

	cat $zipname
}

if [ $# -gt 0 ] && [ "$1" ]; then
	mode=$1
else
	mode=$(__qs_getparam mode) || :
fi

tmpcgidir=/var/cache/httpd/cgi
[ -d $tmpcgidir ] || mkdir -p $tmpcgidir || croak "Error creating '$tmpcgidir'!"

name=${PRG%.cgi}

tmpdir=$(mktemp -d "$tmpcgidir/${name}XXXXXX") ||
	croak "Error creating temporary directory!"

tmpfile=$tmpdir/${name}_cgi.txt
tmpimage=$tmpdir/${name}_image.jpg

case $mode in
	zip)
		generate_report $tmpfile
		construct_and_send_zip $tmpdir $tmpfile
		;;
	zip_with_image)
		/usr/sbin/jpeg_snapshot $tmpimage ||
			croak "Error getting image!"
		generate_report $tmpfile
		construct_and_send_zip $tmpdir $tmpfile $tmpimage
		;;
	text|'')
		__cgi_hdgen yes text/plain
		generate_report $tmpfile
		cat $tmpfile
		;;
	*)
		croak "Invalid mode '$mode'!"
		;;
esac
