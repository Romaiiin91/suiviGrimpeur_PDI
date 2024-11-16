#!/bin/sh -e
#
# Find all regular files uploaded by users and create HTML.
# Allow white space in file names.
#
# CGI parameters
#	hdgen=yes|no			default yes

self=${0##*/}

whine() {
	echo "$self: $*" >&2
}

. /usr/html/axis-cgi/lib/functions.sh || {
	whine "Failed to source functions.sh"
	exit 1
}

croak() {
	logger -perr -t$self -- $* || whine "$*! logger failed!!!"
	exit 1
}

# Generate http header unless specifically turned off.
hdgen=$(__qs_getparam hdgen) && [ "$hdgen" ] || hdgen=yes

# Return the requested data.
__cgi_hdgen $hdgen

cd /usr/html/local || croak "cd failed: $?"

i=0
for dir in administrator operator viewer; do
	files=$(find $dir -type f 2>/dev/null) || croak "find failed: $?"

	IFS='
'
	for file in $files; do
		if [ -r "$file" ]; then
			printf '<input type="hidden" name="f%d" value="%s">\n' 				$i "$file"
			i=$(($i + 1))
		else
			whine "file '$file' not readable"
		fi
	done
done
