#!/bin/sh

ret=0

# Don't fill tmpfs. Save at least 100kB.
ov_path=$(file_upload -S $CONTENT_LENGTH -n 102400) || ret=$?
if [ $ret -ne 0 ]; then
	forwardurl "/operator/overlay.shtml?uploaderror=6" 0
	exit 1
fi

# file_upload does not report file not found but catches failed uploads
# so we can catch file not found after the transfer. If the file has
# size 0 and execution came here, then the original file never existed.
if [ ! -s "$ov_path" ]; then
	forwardurl "/operator/overlay.shtml?uploaderror=7" 0
	rm -f "$ov_path"
	exit 1
fi
palette=$(bmp2overlay -dump-palette "$ov_path") > /dev/null 2>&1
ret=$?
if [ $ret -ne 0 ]; then
	rm -f "$ov_path"
	forwardurl "/operator/overlay.shtml?uploaderror=$ret" 0
	exit 1
fi

# Extract the first parameter, the type of palette (or actually image).
type=${palette%%&*}
type=${type#*\=}
# Remove this first parameter from the palette-string
palette=${palette#*&}

if [ "$type" = fullcolor ]; then
	args=ov_path=$(echo "$ov_path" | urldecode -e)
	forwardurl "/operator/ovl_settings_full.shtml?$args" 0
else
	args=$palette"&"ov_path=$(echo "$ov_path" | urldecode -e)
	forwardurl "/operator/ovl_settings_16.shtml?$args" 0
fi
