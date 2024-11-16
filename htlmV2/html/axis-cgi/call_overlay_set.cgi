#!/bin/sh

if ./overlay_set.cgi $*; then
	/bin/forwardurl /operator/overlay.shtml 0
else
	/bin/forwardurl /operator/overlay.shtml?uploaderror=9 0
fi
