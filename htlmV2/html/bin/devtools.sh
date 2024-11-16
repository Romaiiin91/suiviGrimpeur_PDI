#!/bin/sh

! touch /dev/console 2>/dev/null || exec 2> /dev/console

DEBUG=0

PACKAGE_DIRECTORY=/usr/local/packages
ADPPACKCFG=package.conf
LICENSEFILE=lic.xml
HTMLROOT=/usr/html
TAG=${0##*/}
ACTION=
PACKAGE=
DBUS_REQ_GET_APP_STATUS="dbus-send --system --print-reply --dest=com.axis.Streamer /com/axis/Streamer/RuleEngine com.axis.Streamer.RuleEngine.GetApplicationStatus"

croak() {
	logger -t $TAG -p INFO "$1"
	[ "$DEBUG" = 0 ] || echo "$TAG: $1"
	exit 1
}

echo_ssivar_licensekey() {
	echo -n "<!--#set var=\"license_key\" value=\""
	cat $PACKAGE_DIRECTORY/$1/$LICENSEFILE  2>/dev/null || echo -n "none"
	echo "\" -->"
}

status() {
	local dbus_resp status status_resp="Not Found"

	if [ -r $PACKAGE_DIRECTORY/$1/$ADPPACKCFG ]; then
		. $PACKAGE_DIRECTORY/$1/$ADPPACKCFG || :
		if [ "$APPTYPE" = lua ]; then
			if [ "$LUASTANDALONE" = yes ]; then
				FILENAME=$PACKAGE_DIRECTORY/${APPNAME%.*}/is_running
				read status_resp < $FILENAME
				unset LUASTANDALONE
			else
				dbus_resp=$($DBUS_REQ_GET_APP_STATUS string:$PACKAGE_DIRECTORY/$1/$1.xml 2>/dev/null)
				status=${dbus_resp#*string }
				if [ "$status" = \"Running\" ]; then
					status_resp=Running
				elif [ "$status" = \"Idle\" ]; then
					status_resp=Idle
				else
					status_resp="Not Running"
				fi
			fi
		else
			if [ "$(pidof $APPNAME)" ]; then
				status_resp=Running
			else
				status_resp="Not Running"
			fi
		fi
	fi
	echo $status_resp
}

list() {
	local listpack= found= p= status= licensed= dbus_resp=

	if [ -d $PACKAGE_DIRECTORY ]; then
		cd $PACKAGE_DIRECTORY
		listpack=$(echo *)
		[ "$listpack" != '*' ] || listpack=
		found=0
		for p in $listpack; do
			if [ -r $p/$ADPPACKCFG ]; then
				found=1
				. $p/$ADPPACKCFG || :

				if [ "$APPTYPE" = lua ]; then
					if [ "$LUASTANDALONE" = yes ]; then
						FILENAME=$PACKAGE_DIRECTORY/${APPNAME%.*}/is_running
						read status < $FILENAME
						APPNAME=${APPNAME%.*}
						unset LUASTANDALONE
					else
						dbus_resp=$($DBUS_REQ_GET_APP_STATUS string:$PACKAGE_DIRECTORY/${APPNAME%.*}/$APPNAME 2>/dev/null)
						if [ "${dbus_resp#*string }" = \"Running\" ]; then
							status=Running
						elif [ "${dbus_resp#*string }" = \"Idle\" ]; then
							status=Idle
						else
							status="Not Running"
						fi
						APPNAME=${APPNAME%.*}
					fi
				else
					if [ "$(pidof $APPNAME)" ]; then
						status=Running
					else
						status="Not Running"
					fi
				fi

				if [ -r "$p/$LICENSEFILE" ]; then
					licensed=yes
				elif [ "$LICENSEPAGE" = none ]; then
					licensed="-"
				else
					licensed=no
				fi
				cat <<EOF
document.write('<option value="$APPNAME">' + getPaddedString("<!--#language id="applist_packagenameTxt" text="$PACKAGENAME" -->", width1, true) + '&nbsp;&nbsp;' +
getPaddedString("<!--#language id="applist_versionTxt" text="$APPMAJORVERSION.$APPMINORVERSION" -->", width2, true) + '&nbsp;&nbsp;' +
getPaddedString("<!--#language id="applist_vendorTxt" text="$VENDOR" -->", width3, true) + '&nbsp;&nbsp;' +
getPaddedString("<!--#language id="applist_statusTxt" text="$status" -->", width4, true) + '&nbsp;&nbsp;' +
getPaddedString("<!--#language id="applist_licensedTxt" text="$licensed" -->", width5, true) +
'&nbsp;</option>\n');
EOF
			fi
		done
		if [ $found -eq 0 ]; then
			cat <<EOF
document.write('<option>' + getPaddedString("<!--#language id="servers_NameTxt" text="&nbsp;" -->", 22, true) + '&nbsp;&nbsp;' +
 getPaddedString("<!--#language id="servers_protTxt" text="&nbsp;" -->", 22, true) + '&nbsp;&nbsp;' +
 getPaddedString("<!--#language id="servers_UsrNameTxt" text="&nbsp;" -->", 22, true) + '&nbsp;</option>\n');
EOF
		fi
	fi
}

menulist() {
	local listpack= p= name=

	if [ -d $PACKAGE_DIRECTORY ]; then
		cd $PACKAGE_DIRECTORY
		listpack=$(echo *)
		[ "$listpack" != '*' ] || listpack=
		for p in $listpack; do
			if [ -r $p/$ADPPACKCFG ]; then
				. $p/$ADPPACKCFG || :
				[ "$APPTYPE" != lua ] || APPNAME=${APPNAME%.*}
				if [ "$MENUNAME" ]; then
					name=$MENUNAME
				else
					name=$PACKAGENAME
				fi
				cat <<-EOF
					["$name", "/app_params.shtml", "app=$APPNAME&", hostA, <!--#if expr="\$activeMenu1 = $APPNAME" -->true<!--#else -->false<!--#endif -->, null,
						[
						["Settings", "/app_params.shtml", "app=$APPNAME&", hostA, <!--#if expr="\$activePage = param_$APPNAME" -->true<!--#else -->false<!--#endif -->, null, []],
				EOF
				LICENSEPAGE=$(echo $LICENSEPAGE |tr '[:upper:]' '[:lower:]')
				if [ -z "$LICENSEPAGE" ] || [ "$LICENSEPAGE" = axis ]; then
					cat <<-EOF
						["License", "/app_license.shtml", "app=$APPNAME&", hostA, <!--#if expr="\$activePage = license_$APPNAME" -->true<!--#else -->false<!--#endif -->, null, []],
					EOF
				fi
				if [ "$LICENSEPAGE" = custom ] && [ -r "$HTMLROOT/local/$APPNAME/license.inc" ]; then
					cat <<-EOF
						["License", "/app_license_custom.shtml", "app=$APPNAME&", hostA, <!--#if expr="\$activePage = custom_$APPNAME" -->true<!--#else -->false<!--#endif -->, null, []],
					EOF
				fi
				if [ -r "$HTMLROOT/local/$APPNAME/about.inc" ]; then
					cat <<-EOF
						["About", "/app_index.shtml", "app=$APPNAME&", hostA, <!--#if expr="\$activePage = $APPNAME" -->true<!--#else -->false<!--#endif -->, null, []],
					EOF
				fi
				cat <<-EOF
					[]]],
				EOF
				# Clear variables that may not exist
				# in the next package listed.
				MENUNAME=
			fi
		done
	fi

}

mainpagelink() {
	local indexpage="/local/$1/index.html"

	[ ! -r "$HTMLROOT$indexpage" ] ||
		echo "<a align=\"left\" href=\"javascript:openPopUp('$indexpage', 'AppPopUp', 635, 580)\">Main page</a>"
}

settingslink() {
	local settingspagepath=/local/$1

	if [ -r "$PACKAGE_DIRECTORY/$1/$ADPPACKCFG" ]; then
		. "$PACKAGE_DIRECTORY/$1/$ADPPACKCFG" || :
		if [ "$SETTINGSPAGETEXT" ] && [ "$SETTINGSPAGEFILE" ] &&
			 [ -r "$HTMLROOT$settingspagepath" ]; then
			echo "<a align=\"left\" href=\"$settingspagepath/$SETTINGSPAGEFILE\" target=\"_blank\">$SETTINGSPAGETEXT</a>"
		fi
	fi
}

confvariable() {
	local val=

	if [ -r "$PACKAGE_DIRECTORY/$1/$ADPPACKCFG" ]; then
		. "$PACKAGE_DIRECTORY/$1/$ADPPACKCFG" || :
		eval val=\$$2
		echo $val
	fi
}

load_auto_inst_form() {
	local response=

	response=$(shttpclient -sT 4 -o /dev/stdout http://www.axis.com/techsup/compatible_applications/cam_form.php)

	if [ "$response" ]; then
		echo '<script type="text/javascript">'
		echo "$response"
		echo '</script>'
	else
		echo '<tr><td colspan="4"><div id="internetConnectionStatus">'
		echo '<script type="text/javascript">print_no_conn_text()</script>'
		echo '</div></td></tr>'
	fi
}

ACTION=$1
PACKAGE=$2
VARIABLE=

[ "$ACTION" ]  || croak "No ACTION provided"

case $ACTION in
	list)
		list
		;;
	status)
		[ "$PACKAGE" ] || croak "No PACKAGE provided"
		status "$PACKAGE"
		;;
	menulist)
		menulist
		;;
	mainpagelink)
		[ "$PACKAGE" ] || croak "No PACKAGE provided"
		mainpagelink "$PACKAGE"
		;;
	settingslink)
		[ "$PACKAGE" ] || croak "No PACKAGE provided"
		settingslink "$PACKAGE"
		;;
	confvariable)
		[ "$PACKAGE" ] || croak "No PACKAGE provided"
		VARIABLE=$3
		[ "$VARIABLE" ] || croak "No variable name provided"
		confvariable "$PACKAGE" "$VARIABLE"
		;;
	echo_ssivar_licensekey)
		[ "$PACKAGE" ] || croak "No PACKAGE provided"
		echo_ssivar_licensekey "$PACKAGE"
		;;
	load_auto_inst_form)
		load_auto_inst_form
		;;
	*)
		croak "Action $ACTION not recognized"
		;;
esac

exit 0
