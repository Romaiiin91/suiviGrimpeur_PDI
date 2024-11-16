#!/bin/sh
# CGI parameters
#       action=get|modify
#       name=<application name>
#       configuration=<xml configuration>

# Output errors to the console if user can write to it.
! touch /dev/console 2>/dev/null || exec 2>/dev/console

. /usr/html/axis-cgi/lib/functions.sh

TAG=${0##*/}
LOGINFO="logger -t $TAG -p INFO"
LOGWARN="logger -t $TAG -p WARNING"
LOGCRIT="logger -t $TAG -p CRIT"

PACKAGE_DIRECTORY=/usr/local/packages
ADPPACKCFG=package.conf

# Error in XML format.
__error() {
	__cgi_hdgen yes text/xml
	echo "<reply result=\"error\">"
	echo "<error type=\"$1\" message=\"$2\" />"
	echo "</reply>"

	exit 1
}

# Check what applications are installed. Returns a list of applications.
__list_applications() {
	local listpack= found= p

        # Make sure that the package directory exists and list the name of the
        # subdirectories of the PACKAGE_DIRECTORY.
	if [ -d $PACKAGE_DIRECTORY ]; then
		cd $PACKAGE_DIRECTORY
		listpack=$(echo *)
		[ "$listpack" != '*' ] || listpack=
		found=0
		for p in $listpack; do
			if [ -r $p/$ADPPACKCFG ]; then
				if [ $found -eq 0 ]; then
					found=1
					__cgi_hdgen yes text/xml
					printf "<reply result=\"ok\">\n"
				fi
				. $p/$ADPPACKCFG || :
				if [ "$APPTYPE" = lua ]; then
					APPNAME=${APPNAME%.*}
				fi
				printf "\t<application name=\"$APPNAME\" />\n"
			fi
		done
		if [ $found -eq 1 ]; then
			printf "</reply>\n"
		else
			__cgi_hdgen yes text/xml
	                printf "<reply result=\"ok\">\n</reply>\n"
		fi
	else
		__cgi_hdgen yes text/xml
                printf "<reply result=\"ok\">\n</reply>\n"
	fi
}

# Retrieves an application configuration.
__get_configuration() {
	local packdir= fpath

	# Make sure that the application exists and get the name of the
	# XML configuration file ($APPNAME).
	packdir="$PACKAGE_DIRECTORY/$1"
	[ -d $packdir ] || {
		$LOGINFO "Could not find application '$1'"
		__error no_such_application "No application '$1' exists"
	}
	fpath=$packdir/$ADPPACKCFG
	. $fpath || {
		$LOGCRIT "Could not source configuration file '$fpath'"
		__error internal "Failed to get configuration for '$1'"
	}

	fpath=$packdir/$APPNAME
	[ -r $fpath ] || {
		$LOGCRIT "Could not read the file '$fpath'"
		__error internal "Failed to get configuration for '$1'"
	}

	__cgi_hdgen yes text/xml
	echo "<reply result=\"ok\">"
	cat $fpath
	echo "</reply>"
}

# Replaces the existing configuration (XML file) with a new one.
__modify_configuration() {
	local line= packdir= tmpfile= fpath

	tmpfile=$(mktemp /tmp/$TAG.XXXXXX 2>&1) || {
		$LOGCRIT "Could not create temporary file=$?"
		__error internal "Failed to upload configuration"
	}

	# Read the XML configuration from the POST command.
	while read line; do
		echo $line >> $tmpfile
	done

	# If the last input line is terminated by a new-line character,
	# $line will be empty since the last read is done when there is no more
	# input data. If, on the other hand, the last input line is _not_
	# terminated by a new-line character, $line will contain actual data,
	# even though the read-command returned false. If that is the case,
	# we need to add it.
	[ -z "$line" ] || echo $line >> $tmpfile

	# Make sure that the application exists and get the name of the
	# XML configuration file ($APPNAME).
	packdir="$PACKAGE_DIRECTORY/$1"
	[ -d $packdir ] || {
		$LOGCRIT "Could not find application '$1'"
		__error no_such_application "No application '$1' exists"
	}

	fpath=$packdir/$ADPPACKCFG
	. $fpath || {
		$LOGCRIT "Could not source the package configuration file '$fpath'"
		rm -f $tmpfile
		__error internal "Failed to upload configuration"
	}

	# Move the file to replace the old one.
	fpath=$packdir/$APPNAME
	res=$(mv $tmpfile $packdir/$APPNAME 2>&1) || {
		$LOGCRIT "Failed to move $tmpfile to $fpath: $res"
		rm -f $tmpfile
		__error internal "Failed to upload configuration"
	}

	# Set permissions to read it.
	res=$(chmod 644 $fpath 2>&1) || {
		$LOGCRIT "Failed to set permissions on $fpath: $res"
		rm -f $tmpfile
		__error internal "Failed to upload configuration"
	}

	# Make sure the group is admin.
	res=$(chgrp admin $fpath 2>&1) || {
		$LOGCRIT "Failed to set group on $fpath: $res"
		rm -f $tmpfile
		__error internal "Failed to upload configuration"
	}

	# If the application is currently running, restart it.
	if [ "$(dbus-send --system --print-reply --dest=com.axis.Streamer /com/axis/Streamer/RuleEngine com.axis.Streamer.RuleEngine.GetApplicationStatus string:$fpath 2>/dev/null)" ]; then
		/etc/init.d/sdk$1 restart > /dev/null || {
			$LOGCRIT "Failed to restart application '$fpath'"
			__error internal "Failed to restart application '$1'"
		}
	fi

	__cgi_hdgen yes text/xml
	echo "<reply result=\"ok\" />"
}


[ "$REQUEST_METHOD" != POST ] || read QUERY_STRING
va_action=$(__qs_getparam action) || __error bad_request "No action provided"


case "$va_action" in
	list)
		__list_applications 
		;;
	get)
                va_name=$(__qs_getparam name) || __error bad_request "No name provided"
		__get_configuration "$va_name"
		;;
	modify)
		va_name=$(__qs_getparam name) || __error bad_request "No name provided"
                __modify_configuration "$va_name"
		;;
	*)
		__error bad_request "Invalid action '$va_action'"
		;;
esac

exit 0
