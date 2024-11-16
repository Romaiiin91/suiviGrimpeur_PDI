#!/bin/sh
#
# Find all regular files uploaded by users and create HTML.
# Allow white space in file names

ret=0
dir=/etc/overlays

cd $dir || ret=$?
if [ $ret -ne 0 ]; then
    echo -e "cd failed:" $ret
    exit $ret
fi

# A .ovl can be a directory (scalable overlay) or a file (non-scalable)
files=`find $dir -name \*ovl 2>/dev/null` || ret=$?
if [ $ret -ne 0 ]; then
    echo -e "find failed:" $ret
    exit $ret
fi

i=0
fn=
old_IFS="$IFS"
IFS="
"
for file in $files; do

    if [ "$file" = "/etc/overlays/use_overlay" ]; then
	continue
    fi

    if [ -e $file ]; then
	    echo -e "<input type=\"hidden\" name=\"f$i\" value=\"$file\">"
	    i=`expr "$i" + 1`
	    fn=
    else
	    if [ -z "$fn" ]; then
		fn=$file
	    else
		fn="$fn $file"
	    fi
    fi

    if [ -e "$fn" ]; then
	    echo -e "<input type=\"hidden\" name=\"f$i\" value=\"$fn\">"
	    i=`expr "$i" + 1`
	    fn=
    fi
done
IFS="$old_IFS"

exit 0
