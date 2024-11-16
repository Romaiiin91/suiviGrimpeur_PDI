#!/bin/sh

return_url=/admin/upload_file.shtml

file_id=$(file_cache -S $CONTENT_LENGTH -n 102400) ||
	return_url="$return_url?uploaderror=1"

if [ "$file_id" ]; then
	return_url="$return_url?fileID=$file_id"
else
	return_url="$return_url?uploaderror=2"
fi

forwardurl "$return_url" 0
