#!/bin/sh

if ./overlay_del.cgi $*; then
	/bin/forwardurl /operator/overlay.shtml 0
else
	/bin/forwardurl /operator/overlay.shtml?uploaderror=8 0
fi
