#!/bin/sh
#
# Delete all regular files uploaded by users and defined by query
# Allow white space in file names

local_del() {
	local tdir=/usr/html/local IFS=: fn= file=

	cd $tdir || {
		echo "cd to '$tdir' failed with status: $?"
		return 1
	}

	fn=$(echo "$*" | sed -e "s#[[:blank:]]\+$tdir/#:#g")

	for file in $fn; do
		rm -f "$file" || {
			echo "rm '$file' failed with status: $?"
			return 1
		}
	done
}

if local_del "$*"; then
	forwardurl /admin/fileUpload.shtml 0
else
	forwardurl "/admin/fileUpload.shtml?uploaderror=1" 0
fi
