#!/bin/sh

# Print the path to the overlay image.

param=root.Image.OverlayPath
use_file=$(parhandclient get $param | tr -d '\"\n\r')
overlay=

if [ -z "$use_file" ] || [ -f "$use_file" ] || [ -d "$use_file" ]; then
	overlay=$use_file
else
	logger "Overlay image '$use_file' not found."
	exit 1
fi
echo -e "<input type=\"hidden\" name=\"use_overlay\" value=\"$overlay\">"
