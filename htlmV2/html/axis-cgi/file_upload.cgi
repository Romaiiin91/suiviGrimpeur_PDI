#!/bin/sh

. /lib/rcscripts/sh/error.sh
. /lib/rcscripts/sh/integer.sh

[ "$CONTENT_LENGTH" ] && is_integer $CONTENT_LENGTH ||
        error "${0##*/}: CONTENT_LENGTH missing or not integer"

if file_upload -s $CONTENT_LENGTH -n 65536 -a; then
	forwardurl /admin/fileUpload.shtml 0
else
	forwardurl "/admin/fileUpload.shtml?uploaderror=1" 0
fi
