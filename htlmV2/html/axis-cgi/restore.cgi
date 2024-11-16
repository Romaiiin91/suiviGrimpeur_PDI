#!/bin/sh -e
#
# Restore a backup of the JFFS filesystem either by HTTP or FTP.
# The backup contains the serial number of the unit and is unit specific.
# HTTP: requires REQUEST_METHOD=POST
# FTP: requires FTP_METHOD=PUT from the virtual target execution.

#exec 2>/dev/console

# Switch to runlevel 3 on exit.
#
trap "init 3" EXIT

# Trap sigpipe (13), letting the above defined trap switch to runlevel 3.
# Sigpipe will be received whenever the user cancels a restore.
#
trap "exit 0" 13

# Set PATH
#
PATH=/bin:/sbin:/usr/bin:/usr/sbin

# Busybox tar subtracts the user's umask from each file permission, even for
# root and does not support the -p option. Therefore, explicitly clear umask
# since this script is always run as root.
umask 0

# Test return values
#
ret=0

# Error messages
#
errmsg=

# Language file in tar-file. This file should not exist if the product do not
# support different languages.
#
language_file=LANGUAGE.orig

# Various files with static names
#
c_serno=SERNO.curr
o_serno=SERNO.orig
o_etc=ETC.orig
tarfilename=/tmp/rbackup.tar
relf=etc/release

shout() {
	echo -e "$*\r"
}

# Send error message
#
panic() {
	shout $errmsg $ret
	shout "The backup was not restored due to error conditions"
}

# Remove the created files
#
cleanup() {
	rm -rf etc $o_serno $o_etc $c_serno \
		$tarfilename $language_file
}

# Messages to the client
#
if [ "$REQUEST_METHOD" ]; then
	shout "Content-Type: text/plain"
	shout
fi

# Call only by HTTP or FTP
#
if [ -z "$REQUEST_METHOD" ] && [ -z "$FTP_METHOD" ]; then
	errmsg="Environment variables REQUEST_METHOD or FTP_METHOD must be: "
	ret="(POST,PUT)"
	panic
	exit 1
fi

# HTTP POST required to obtain a backup file
#
if [ "$REQUEST_METHOD" ] && [ $REQUEST_METHOD != POST ]; then
	errmsg="Invalid request method for application/x-tar: "
	ret=$REQUEST_METHOD
	panic
	exit 1
fi

# FTP PUT required to obtain a backup file
#
if [ "$FTP_METHOD" ] && [ $FTP_METHOD != PUT ]; then
	errmsg="Invalid request method for application/x-tar: "
	ret=$FTP_METHOD
	panic
	exit 1
fi

# Init single user mode
#
init 4 2>&1 || ret=$?
if [ $ret -ne 0 ]; then
	errmsg="Could not enter single user run level, init 4 failed: "
	panic
	exit $ret
fi
# Arbitrary amount of time. Why is this necessary?
sleep 3

# Work in /tmp
#
cd /tmp 2>&1 || ret=$?
if [ $ret -ne 0 ]; then
	errmsg="Could not change to temporary directory, cd failed: "
	panic
	exit $ret
fi

# Receive the backup from the client by HTTP
#
if [ "$REQUEST_METHOD" ]; then
	# Parse the multipart/form
	#
	tarfilename=$(file_upload -S $CONTENT_LENGTH -n 100000) || ret=$?
	if [ $ret -ne 0 ]; then
		errmsg="Did not receive the archive file; file_upload failed: "
		panic
		exit $ret
	fi

	# Anything?
	#
	[ "$tarfilename" ] || {
		errmsg="Did not receive an archive file, but file_upload returned: "
		panic
		exit 0
	}
fi

# Receive the backup from the client by FTP
#
if [ "$FTP_METHOD" ]; then
	cat > $tarfilename
	if [ $ret -ne 0 ]; then
		errmsg="Did not receive an archive file; cat failed: "
		panic
		exit $ret
	fi
fi

# Unpack the backup
#
tar xf $tarfilename 2>&1 || ret=$?
if [ $ret -ne 0 ]; then
	errmsg="Erroneous archive file format; restoring $tarfilename failed: "
	panic
	exit $ret
fi

# Compare the revisions of the JFFS filesystem
#
if [ -r /$relf ] && [ -r $relf ]; then
	if cmp -s /$relf $relf; then
		shout "Firmware revision of backup is correct."
	else
		errmsg="Inconsistent file system revisions, "
		ret="the firmware revision differs from the backup revision"
		panic
		exit 0
	fi
fi

# Compare the file name of the backup with the present serial number
#
bootblocktool -nocgi -x SERNO > $c_serno || ret=$?
if [ $ret -ne 0 ]; then
	errmsg="Could not find the serial number, bootblocktool failed: "
	panic
	exit $ret
fi

# The backup must be taken from this unit
#
if [ -r $o_serno ]; then
	if cmp -s $o_serno $c_serno; then
		shout "Restore backup..."
		if [ -r $o_etc ]; then
			# Replace files which differ. Add files in the file
			# system which exist in the tape archive but not in
			# the file system
			#
			replace=0

			# Go through file names in the backup tape archive
			#
			for file in $(cat $o_etc); do
				if [ -r /$file ]; then
					if cmp -s /$file $file; then
						replace=0
					else
						shout "Replacing file: /$file"
						replace=1
					fi
				else
					shout "Adding file: /$file"
					replace=1
				fi

				if [ $replace -eq 1 ]; then
					cp $file /$file 2>&1 || ret=$?
					if [ $ret -ne 0 ]; then
						errmsg="Could not copy file, cp failed: "
						panic
						exit $ret
					fi
					replace=0
				fi
			done

			# Remove files in the file system which do not exist
			# in the tape archive.
			#
			for file in $(find /etc/ -type f | \
				      sed -re 's#^/(.*)#\1#'); do
				if [ ! -r $file ];then
					shout "Removing file: /$file"
					rm -f /$file 2>&1
				fi
			done
		else
			errmsg="Erroneous file content, "
			ret="the file $o_etc is not part of the backup"
			panic
			exit 0
		fi

		# Restore language,
		# it is _not_ considered a failure if the
		# language was not saved and present in the backup.
		language_linkfile=/etc/language/langfile.dat
		if [ -r $language_file ]; then
			rm -f $language_linkfile
			ln -s $(cat $language_file) $language_linkfile
		fi

		shout "The backup was restored successfully"
	else
		errmsg="This is not the unit from where the backup was made"
		ret=
		cleanup
		panic
		exit 0
	fi
else
	errmsg="Erroneous file content, "
	ret="the file $o_serno is not part of the backup"
	panic
	exit 0
fi

# remove the created files
cleanup

# Reboot
#
shout "The unit will now reboot."
shout "To continue, please connect to the unit again."
shout "There may be a short delay before the new connection is accepted."
shout "This connection will now close."

trap - EXIT
init 6

exit 0
