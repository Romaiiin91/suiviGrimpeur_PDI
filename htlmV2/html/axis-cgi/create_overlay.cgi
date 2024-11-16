#!/bin/sh

echo -e "Cache-Control: no-cache\r
Pragma: no-cache\r
Expires: Thu, 01 Dec 1994 16:00:00 GMT\r
Content-Type: text/html\r
\r"

forwardurl() {
echo -e "<html>\r
  <head>\r
    <meta http-equiv=\"refresh\"\r
	  content=\"0;URL=$1\">\r
  </head>\r
  <body>\r
  \r
  </body>\r
</html>\r
\r"
}

# Read the FORM parameters from stdin.
read POST_INPUT

#
# Parse a setting in $QUERY on the form setting=<value>& and
# return the value.
#
parse() {
	VALUE=${POST_INPUT##*$1=}
	if [ "$VALUE" = "$POST_INPUT" ]; then
		unset VALUE
		return 1
	else
		echo ${VALUE%%&*}
		unset VALUE
		return 0
	fi
}

type=`parse type`

# Get the path to the uploaded bmp image.
ov_path=`parse ov_path | urldecode`
# Get the file prefix, later used in the overlay file name.
prefix=${ov_path##*/}
prefix=${prefix%.*}

# Scalable overlays
if usescalable=$(parse usescalable); then
	ovl_dir=/etc/overlays/$prefix.ovl
	# Prevent overwrite problems
	rm -rf $ovl_dir || croak "Error deleting '$ovl_dir'!"
	mkdir -m775 $ovl_dir || croak "Error creating '$ovl_dir'!"
	chgrp streamer $ovl_dir ||
		croak "Failed to change owner on '$ovl_dir'!"
	# Write the palette file from the parameter given by the web page
	if usetransparent=$(parse usetransparent); then
		colorcode=$(parse colorcode) ||
			croak "Failed to read transparent color!"
		overlay_palette=$ovl_dir/pal
		echo "$colorcode" >$overlay_palette
		# Set the file as readable by all programs
		chmod 644 $overlay_palette ||
			croak "Failed to change permissions on '$overlay_palette'!"
	fi
	# Put the uploaded bitmap in a safe place
	mv "$ov_path" $ovl_dir/bmp ||
		croak "Failed to move '$ov_path' to '$ovl_dir/bmp'!"
	chmod 644 $ovl_dir/bmp ||
		croak "Failed to change permissions on '$ovl_dir/bmp'!"
	forwardurl /operator/overlay.shtml ||
		croak "Failed to forward url /operator/overlay.shtml"
	exit 0
else
	# Non-scalable overlays
	if [ "$type" = "fullcolor" ]; then
		usetransparent=`parse usetransparent`
		if [ $? -ne 0 ]; then
			echo "f" >>/tmp/overlay_palette
		else
			colorcode=`parse colorcode`
			echo "f $colorcode" >>/tmp/overlay_palette
		fi
	else
		i=0
		while [ $i -lt 16 ]; do
			k=$(($i+1))
			c_p=`parse "c_p"$k` || c_p="fullcolor"
			o_t=`parse "o_t"$k` || o_t="solid"
			l_p=`parse "l_p"$k` || l_p="normal"
			echo "i "$i" "$c_p" "$o_t" "$l_p >>/tmp/overlay_palette
			i=$(($i+1))
		done
	fi

	# Run bmp2overlay in background and check every 1s that it is still
	# running. If it is, feed the web browser a space in order to avoid
	# timeouts. A temporary file is used to capture output (i.e. resulting
	# ovl filename).
	ovl_filename=`mktemp -q /tmp/create_overlay.XXXXXX` || exit 1
	exit_status=`mktemp -q /tmp/create_overlay.XXXXXX` || exit 1
	(
		bmp2overlay /tmp/overlay_palette "$ov_path"\
		/etc/overlays/"$prefix" > $ovl_filename
		echo $? > $exit_status
	)&
	ret=0
	while [ $ret -eq 0 ]; do
		echo -e " "
		kill -0 $!
		ret=$?
		sleep 1
	done

	outfile=`cat $ovl_filename`
	rm -f "$ovl_filename"

	err=`cat $exit_status`
	rm -f "$exit_status"
	if [ -f "$outfile" ] && [ $err -eq 0 ]; then
		# Make sure 64kB is still available in the destination file
		# system. We need to do this test this late because the
		# destination file system may compress the files.
		if [ `df -k "$outfile" |
			sed '1d;s/.* \([0-9]\+\) \+[0-9]\+% \+.*/\1/'` -lt 64 ]; then
				rm -f "$outfile"
				err=6
		fi
	elif [ ! -f "$outfile" ] || [ $err -ne 0 ]; then
		forwardurl "/operator/overlay.shtml?uploaderror=$err" 0
		rm -f "$ov_path"
		rm -f /tmp/overlay_palette
		exit 1
	fi

	rm -f /tmp/overlay_palette
	rm -f "$ov_path"

	if [ $err -eq 6 ]; then
		forwardurl "/operator/overlay.shtml?uploaderror=$err" 0
		exit 1
	else
		chmod 644 "$outfile"
		forwardurl "/operator/overlay.shtml" 0
		exit 0
	fi
fi
