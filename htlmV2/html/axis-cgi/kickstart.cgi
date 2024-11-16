#! /bin/sh -e

# Uncomment for debugging purpose. Regular admins are not allowed to write to
# /dev/console.
# exec 2>/dev/console

PATH=/bin:/usr/bin:/usr/local/bin

kickroot=/tmp/kickstart
kickctrl=/var/lib/kickstart

fail() {
	trap - EXIT

	# Cleanup
	rm -rf $kickroot

	echo -e "Content-Type: text/html\r\n\r"
cat << EOT
<html>
<head>
<title>Kickstart failed</title>
</head>
<body>
<h3>Kickstart failed!</h3>
EOT
	[ "$1" ] && echo -e "\n<p>$*</p>"
	echo -e "</body>\n</html>"

	exit 1
}

umask 0077

[ "`cat $kickctrl`" = 1 ] || fail "Disabled."
rm -f $kickctrl

trap fail EXIT

# There can be only one.
[ -d $kickroot ] && \
	fail "Directory \"$kickroot\" already exists." \
	     "Another kickstart process is either running or failed to cleanup."

mkdir $kickroot || fail "Cannot create directory \"$kickroot\"."

name=`file_upload -s $CONTENT_LENGTH -m 99`

cd $kickroot
tar zxf "$name" 2>/dev/null || tar xf "$name" || fail "Invalid file format."
rm -f "$name"

PATH="$kickroot/sbin:$kickroot/bin:$kickroot/usr/sbin:$kickroot/usr/bin:$PATH"
export PATH
LD_LIBRARY_PATH=$kickroot/lib:$kickroot/usr/lib
export LD_LIBRARY_PATH

# ./kickstart must handle error output on its own.
trap - EXIT

[ -x ./kickstart ] || fail "Missing contents of file."
./kickstart || logger -t "$0[$$]" "Execution failed!" || :

# Cleanup
rm -rf $kickroot

exit 0
