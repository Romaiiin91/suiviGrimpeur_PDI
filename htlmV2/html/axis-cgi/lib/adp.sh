#!/bin/sh
. /etc/init.d/functions.sh
. /lib/rcscripts/sh/cmpversions.sh

CLEANUP=1

PACKAGE_DIRECTORY=/usr/local/packages
HTML_FILE=${HTTP_REFERER#http*//*/}
if [ -z "$HTML_FILE" ]; then
	HTML_FILE=/devtools.shtml
else
	HTML_FILE=/${HTML_FILE%%\?*}
fi

TAG=${0##*/}
LOGINFO="logger -t $TAG -p INFO"
LOGWARN="logger -t $TAG -p WARNING"
LOGCRIT="logger -t $TAG -p CRIT"





ADPPACKCFG=package.conf
ELFCHECK=elflibcheck.sh
LICDIR=/var/lib/adptools
LICFILENAME=lic.xml
LICFILEPATH=$LICDIR/$LICFILENAME
VAR_RUN_DIR=/var/run/adptools
ADP_LISTCMD_ERROR_TYPE="Internal error"
ADP_LISTCMD_ERROR_MSG_UNKNOWN="Unknown error"
ADP_LISTCMD_ERROR_MSG_CONFIG="Error reading configuration file"
ADP_LISTCMD_ERROR_MSG_STATUS="Error getting running status"
ADP_LISTCMD_ERROR_MSG_LICENSE="Error reading license status"

INSTALLER=install-package.sh
PHC="parhandclient --nocgi"
DBUS_REQ_GET_APP_STATUS="dbus-send --system --print-reply --dest=com.axis.Streamer /com/axis/Streamer/RuleEngine com.axis.Streamer.RuleEngine.GetApplicationStatus"


__error() {
	local error_nbr=$1 pack=$2 exit_status=1 fwurl=

	[ "$error_nbr" ] || {
		$LOGCRIT "function __error() needs 1 parameter"
		exit $exit_status
	}

	if [ "$RELOAD_PAGE" = yes ]; then
		fwurl="$HTML_FILE?error=$error_nbr"
		[ -z "$pack" ] || fwurl="$fwurl&app=$pack"
		forwardurl "$fwurl"
	else
		__cgi_hdgen yes
		printf "Error: $error_nbr"
	fi


	exit $exit_status
}

__check_ambarella_config() {
local AMBAD_CONFIG_FILE=/etc/conf.d/ambad

[ ! -f $AMBAD_CONFIG_FILE ] || return 0

ldd $dest_dir/$APPNAME | grep -q libcapture || return 0

uname -m | grep -q armv6l || return 0

$LOGINFO "Created file $AMBAD_CONFIG_FILE, please reboot"
printf "%s\n" "AMBAD_RESERVE_BUFFER2=YES" > $AMBAD_CONFIG_FILE
chmod +r $AMBAD_CONFIG_FILE
}

__send_action_resp() {
	local return_page pack=$3

	return_page=$2

	if [ "$return_page" ]; then
		forwardurl "$return_page" 0
	elif [ "$RELOAD_PAGE" = yes ]; then
		local fwurl=$HTML_FILE
		case $1 in
			upload)
				fwurl="$fwurl?package_uploaded=yes"
				;;
			remove)
				fwurl="$fwurl?package_removed=yes"
				;;
			start)
				fwurl="$fwurl?package_started=yes"
				;;
			stop)
				fwurl="$fwurl?package_stopped=yes"
				;;
			*)
				[ -z "$pack" ] || fwurl="$fwurl?app=$pack"
				;;
		esac
		forwardurl "$fwurl" 0
	else
		__cgi_errhd 200 OK
	fi
}

__lua_application_is_running() {
	local xmlfile=$1

	[ "$xmlfile" ] ||	{
		$LOGCRIT "__lua_application_is_running needs para"
		return
	}

	if [ "$(dbus-send --system --print-reply --dest=com.axis.Streamer /com/axis/Streamer/RuleEngine com.axis.Streamer.RuleEngine.GetApplicationStatus string:$PACKAGE_DIRECTORY/$APPNAME/$xmlfile 2>/dev/null)" ]; then
		echo lua
	fi
}

__upload_adp_package() {
	local ret=0 tmp_file= tmp_ld= dest_dir= res= pid= action=upload
	local oldpwd=$PWD emb_cache_size= xmlfile= cache_size=
	local emb_default_cache_size= cache_size_to_set= size_7_5M=7864320

	[ -d $PACKAGE_DIRECTORY ] || {
		res=$(mkdir $PACKAGE_DIRECTORY 2>&1) || {
			$LOGCRIT "$res"
			__error 3
		}
		res=$(chmod 775 $PACKAGE_DIRECTORY 2>&1) || {
			$LOGCRIT "$res"
			__error 3
		}
	}

	tmp_ld=$(mktemp -d /tmp/pack.XXXXXX 2>&1) || {
		$LOGCRIT "failed: mktemp -d /tmp/pack.XXXXXX $tmp_ld"
		__error 3
	}

	tmp_file=$(file_upload -S $CONTENT_LENGTH -n 204800 2>&1) || {
		$LOGCRIT "Upload failed: '$tmp_file'"
		[ $CLEANUP -eq 0 ] || rm -fr $tmp_ld
		__error 3
	}

	[ -r "$tmp_file" ] || {
		$LOGCRIT "Cannot find file '$tmp_file'"
		[ $CLEANUP -eq 0 ] || rm -fr $tmp_ld
		__error 3
	}
	res=$(tar -xzf $tmp_file -C $tmp_ld 2>&1) || {
		local res2
		res2=$(tar -xf $tmp_file -C $tmp_ld 2>&1) || {
			$LOGCRIT "Failed to unpack $tmp_file: ($res) ($res2)"
			[ $CLEANUP -eq 0 ] || {
				rm -fr $tmp_ld
				rm -f $tmp_file
			}
			__error 1
		}
	}
	rm -f $tmp_file

	$LOGINFO "Uploaded '$tmp_file' size:$CONTENT_LENGTH"

	if [ -r $tmp_ld/$ADPPACKCFG ]; then
		. $tmp_ld/$ADPPACKCFG || :
	else
		$LOGCRIT "Could not source (1) '$ADPPACKCFG'"
		[ $CLEANUP -eq 0 ] || rm -fr $tmp_ld
		__error 2
	fi

	if EMBDEVVERSION=$($PHC get properties.EmbeddedDevelopment.Version 2>&1); then
		[ -z "$REQEMBDEVVERSION" ] && {
			REQEMBDEVVERSION="1.0"
		}
		compare_vers $REQEMBDEVVERSION le $EMBDEVVERSION || {
			$LOGCRIT "Invalid embedded development version $EMBDEVVERSION. \"$APPNAME\" require at least version $REQEMBDEVVERSION."
			__error 1
		}
	else
		$LOGCRIT "Failed to get embedded development version to check compatibility with application \"$APPNAME\"."
		__error 1
	fi

	[ -z "$(echo -n $APPNAME | sed -e 's/^[a-zA-Z][a-zA-Z0-9\.]*//')" ] || {
		$LOGCRIT "Invalid APPNAME \"$APPNAME\" (use only a-zA-Z0-9. Start with a letter)"
		__error 1
	}
	[ -z "$(echo $APPID | sed -e 's/[0-9]//g')" ] || {
		$LOGCRIT "Invalid APPID (use only 0-9)"
		__error 1
	}
	[ -z "$(echo $APPMAJORVERSION | sed -e 's/[0-9]//g')" ] || {
		$LOGCRIT "Invalid MAJOR (use only 0-9)"
		__error 1
	}
	[ -z "$(echo $APPMINORVERSION | sed -e 's/[0-9]//g')" ] || {
		$LOGCRIT "Invalid MINOR (use only 0-9)"
		__error 1
	}
	if [ "$APPTYPE" = lua ]; then
		xmlfile=$APPNAME
		APPNAME=${APPNAME%.*}
	else
		[ -z "$(echo -n $APPNAME | sed -e 's/^[a-zA-Z][a-zA-Z0-9]*//')" ] || {
			$LOGCRIT "Invalid APPNAME \"$APPNAME\" (use only a-zA-Z0-9 Start with a letter)"
			__error 1
		}
		if [ -x $tmp_ld/$APPNAME ]; then
			res=$($ELFCHECK $tmp_ld/$APPNAME 2>&1) || {
				$LOGWARN "$res"
				[ $CLEANUP -eq 0 ] || rm -fr $tmp_ld
				__error 5
			}
		else
			$LOGCRIT "'$APPNAME' not found or not executable"
			[ $CLEANUP -eq 0 ] || rm -fr $tmp_ld
			__error 2
		fi
	fi

	if [ "$APPTYPE" = lua ]; then
		pid=$(__lua_application_is_running $xmlfile)
	else
		pid=$(pidof $APPNAME) || :
	fi
	if [ "$pid" ]; then
		/etc/init.d/sdk$APPNAME stop >/dev/null ||
			$LOGWARN "Application $APPNAME[$pid] can not be stopped"
	fi

	dest_dir=$PACKAGE_DIRECTORY/$APPNAME

	if [ -d $dest_dir ]; then
		cd $dest_dir
		res=$(eval $(__rootpath) $INSTALLER uninstall) || $LOGWARN "$res"
		cd $oldpwd
	fi

	rm -fr $dest_dir
	res=$(mv $tmp_ld $dest_dir 2>&1) || {
		$LOGCRIT "Failed mv $tmp_ld $dest_dir: $res"
		[ $CLEANUP -eq 0 ] || rm -fr $tmp_ld
		__error 3
	}
	res=$(chmod 775 $dest_dir 2>&1) || {
		$LOGCRIT "$res"
		[ $CLEANUP -eq 0 ] || rm -fr $dest_dir
		__error 3
	}

	cd $dest_dir
	eval $(__rootpath) $INSTALLER install >/tmp/$$install.log 2>&1 || ret=$?
	cd $oldpwd

	if [ $ret -ne 0 ]; then
		$LOGCRIT -t install.log </tmp/$$install.log
		rm -f /tmp/$$install.log
		$LOGCRIT "Package $tmp_file installation FAILED"
		[ $CLEANUP -eq 0 ] || rm -fr $dest_dir
		__error 10
	else
		mv /tmp/$$install.log $dest_dir/install.log || :
	fi

	[ ! -r "$dest_dir/$POSTINSTALLSCRIPT" ] ||
		sh $dest_dir/$POSTINSTALLSCRIPT ||
			$LOGWARN "$APPNAME[0] Failed to run POSTINSTALLSCRIPT $POSTINSTALLSCRIPT"

	if emb_cache_size=$($PHC get properties.EmbeddedDevelopment.CacheSize - RAW) &&
		emb_default_cache_size=$($PHC get properties.EmbeddedDevelopment.DefaultCacheSize - RAW) &&
		cache_size=$($PHC get StreamCache.Size - RAW); then
		if [ "$APPTYPE" = lua ]; then
			if [ "$LUASTANDALONE" = yes ]; then
				cache_size_to_set=$cache_size
			else
				if [ $cache_size -ne $emb_cache_size ]; then
					cache_size_to_set=$(($emb_default_cache_size - $size_7_5M))
				else
					cache_size_to_set=$cache_size
				fi
			fi
		else
			cache_size_to_set=$emb_cache_size
		fi
		res=$($PHC set StreamCache.Size $cache_size_to_set) ||
			$LOGCRIT "Failed to set cache size: $res"
	else
		$LOGCRIT "Failed to get cache size: $emb_cache_size"
	fi

	__check_ambarella_config

	$LOGINFO "Package $tmp_file installation OK"

	__send_action_resp $action
}

__install_adp_license_key() {
	local pack=$1 action=uploadlicensekey packdir=



	packdir=$PACKAGE_DIRECTORY/$pack
	[ -d $packdir ] || {
		$LOGCRIT "Directory for package $pack is missing"
		__error 24 $pack
	}

	local resstr

	resstr=$(mv $LICFILEPATH $packdir/$LICFILENAME 2>&1) || {
		$LOGCRIT "$resstr"
		__error 22 $pack
	}

	$LOGINFO "Uploaded '$LICFILENAME' for $pack OK"

	__send_action_resp $action "" $pack
}

__upload_adp_license_key() {
	local pack=$1 action=uploadlicensekey ret=0 tmp_file= packdir= old_IFS=
	local licensekey= filename= serialnumber= expdateseconds= nowseconds=
	local applicationid= minmajorversion= minminorversion= maxmajorversion=
	local maxminorversion= expirationdate= deviceid=



	packdir=$PACKAGE_DIRECTORY/$pack
	[ -d $packdir ] || {
		$LOGCRIT "Directory for package $pack is missing"
		__error 24 $pack
	}
	if [ -r $packdir/$ADPPACKCFG ]; then
		. $packdir/$ADPPACKCFG || :
	else
		$LOGCRIT "Could not source (2) '$ADPPACKCFG'"
		[ $CLEANUP -eq 0 ] || rm -fr $tmp_ld
		__error 24 $pack
	fi
	tmp_file=$(file_upload -S $CONTENT_LENGTH -n 204800 2>&1) || ret=$?
	if [ $ret -ne 0 ]; then
		$LOGCRIT "Upload failed: '$tmp_file'"
		__error 22 $pack
	else
		[ -r "$tmp_file" ] || {
			$LOGCRIT "Cannot find file '$tmp_file'"
			__error 22 $pack
		}

		rm -f $LICFILEPATH
		mkdir -p $LICDIR || {
			$LOGCRIT "Failed to mkdir $LICDIR (1)"
			__error 22 $pack
		}
		chmod 700 $LICDIR || {
			$LOGCRIT "Failed to chmod $LICDIR(1)"
			__error 22 $pack
		}

		mv "$tmp_file" "$LICFILEPATH" || {
			$LOGCRIT cannot mv $tmp_file $LICFILEPATH
			__error 22 $pack
		}
		chmod 644 $LICFILEPATH || {
			$LOGCRIT "Failed to set permissions on created license file"
			__error 22 $pack
		}

		old_IFS=$IFS
		IFS=
		licensekey=$(tr -d '\n' <$LICFILEPATH |
			grep -e "<LicenseKey>.*</LicenseKey")

		IFS=$old_IFS
		if [ -z "$licensekey" ]; then
			$LOGWARN "Failed to parse license key(1)"
			rm -f $LICFILEPATH
			__error 21 $pack
		else
			expression=$(echo $licensekey | sed -rn 's/.*<ApplicationID>([[:digit:]]*)<\/ApplicationID>.*<MinimumMajorVersion>([-[:digit:]]*)<\/MinimumMajorVersion>.*<MinimumMinorVersion>([-[:digit:]]*)<\/MinimumMinorVersion>.*<MaximumMajorVersion>([-[:digit:]]*)<\/MaximumMajorVersion>.*<MaximumMinorVersion>([-[:digit:]]*)<\/MaximumMinorVersion>.*<ExpirationDate>([-[:digit:]]*)<\/ExpirationDate>.*<DeviceID>([0-9A-Fa-f]*)<\/DeviceID>.*/applicationid=\1 minmajorversion=\2 minminorversion=\3 maxmajorversion=\4 maxminorversion=\5 expirationdate=\6 deviceid=\7/p')

			if [ ${#expression} -eq 0 ]; then
				$LOGWARN "Failed to parse license key(2)"
				rm -f $LICFILEPATH
				__error 21 $pack
			fi

			eval $expression || :

			if [ -z "$applicationid" ] ||
			   [ -z "$minmajorversion" ] ||
			   [ -z "$minminorversion" ] ||
			   [ -z "$maxmajorversion" ] ||
			   [ -z "$maxminorversion" ] ||
			   [ -z "$expirationdate" ]; then
				$LOGWARN "Failed to parse license " \
					"\"$applicationid"\" \
					"\"$minmajorversion"\" \
					"\"$minminorversion"\" \
					"\"$maxmajorversion"\" \
					"\"$maxminorversion"\" \
					"\"$expirationdate"\"
				rm -f $LICFILEPATH
				__error 21 $pack
			fi
			if [ "$applicationid" != $APPID ]; then
				$LOGWARN "APPID=$APPID lic=$applicationid"
				__error 25 $pack
			fi
			if [ "$minmajorversion" -ne -1 ] &&
				[ "$APPMAJORVERSION" -lt "$minmajorversion" ]; then
				$LOGWARN "min major $minmajorversion $APPMAJORVERSION $minmajorversion"
				__error 26 $pack
			fi
			if [ "$minminorversion" -ne -1 ] &&
				[ "$APPMAJORVERSION" -eq "$minmajorversion" ] &&
				[ "$APPMINORVERSION" -lt "$minminorversion" ]; then
				$LOGWARN "min minor $APPMAJORVERSION $minmajorversion $APPMINORVERSION $minminorversion"
				__error 26 $pack
			fi
			if [ "$maxmajorversion" -ne -1 ] &&
				[ "$APPMAJORVERSION" -gt "$maxmajorversion" ]; then
				$LOGWARN "max major $maxmajorversion $APPMAJORVERSION $maxmajorversion"
				__error 26 $pack
			fi
			if [ "$maxminorversion" -ne -1 ] &&
				[ "$APPMAJORVERSION" -eq "$maxmajorversion" ] &&
				[ "$APPMINORVERSION" -gt "$maxminorversion" ]; then
				$LOGWARN "max minor $APPMAJORVERSION $maxmajorversion $APPMINORVERSION $maxminorversion"
				__error 26 $pack
			fi
			serialnumber=$($PHC get \
				properties.system.serialnumber - RAW |
				tr [:lower:] [:upper:])
			deviceid=$(echo $deviceid |tr '[:lower:]' '[:upper:]')
			[ "$serialnumber" = "$deviceid" ] || {
				$LOGWARN "$serialnumber != $deviceid"
				__error 30 $pack
			}

			if [ $expirationdate != "0" ]; then
				expdateseconds=$(
					date -d $expirationdate -D%F +%s)
				nowseconds=$(date +%s)
				[ "$expdateseconds" ] && [ "$nowseconds" ] || {
					$LOGWARN "Failed to parse expirationdate"
					__error 21 $pack
				}
				[ $expdateseconds -ge $nowseconds ] ||
					__error 31 $pack
			fi
		fi
		__install_adp_license_key $pack
	fi
}

__remove_adp_package() {
	local pack=$1 action=remove pid= packdir=
	local emb_cache_size= emb_default_cache_param= listpack= p= xmlfile=
	local c_app=0 lua_app=0 cache_size_to_set= size_7_5M=7864320



	packdir="$PACKAGE_DIRECTORY/$pack"
	if [ -r "$packdir/$ADPPACKCFG" ]; then
		. "$packdir/$ADPPACKCFG" || :
		if [ "$APPTYPE" = lua ]; then
			xmlfile=$APPNAME
			APPNAME=${APPNAME%.*}
			pid=$(__lua_application_is_running $xmlfile)
		else
			pid=$(pidof $APPNAME) || :
		fi

		[ -z "$pid" ] ||
			/etc/init.d/sdk$APPNAME stop >/dev/null || {
				$LOGWARN "Application $APPNAME[$pid] can not be stopped"
				__error 10 $pack
			}
		cd $packdir
		eval $(__rootpath) $INSTALLER uninstall >/dev/null || {
			$LOGWARN "Application $APPNAME[$pid] can not be uninstalled"
			__error  10 $pack
		}
	else
		__error 4 $pack
	fi

	if [ -d $PACKAGE_DIRECTORY ]; then
		cd $PACKAGE_DIRECTORY
		listpack=$(echo *)
		[ "$listpack" != '*' ] || listpack=
		for p in $listpack; do
			if [ -r $p/$ADPPACKCFG ]; then
				unset LUASTANDALONE
				. "$p/$ADPPACKCFG" || break
				if [ "$APPTYPE" = lua ]; then
					[ "$LUASTANDALONE" = yes ] ||
						lua_app=$(($lua_app + 1))
				else
					c_app=$(($c_app + 1))
				fi
			fi
		done
	fi

	emb_default_cache_size=$($PHC get properties.EmbeddedDevelopment.DefaultCacheSize - RAW) ||
		$LOGCRIT "Failed to get default cache size: $emb_default_cache_size"

	emb_cache_size=$($PHC get properties.EmbeddedDevelopment.CacheSize - RAW) ||
		$LOGCRIT "Failed to get emb cache size: $emb_cache_size"

	if [ $c_app -ne 0 ]; then
		cache_size_to_set=$emb_cache_size
	elif [ $lua_app -ne 0 ]; then
		cache_size_to_set=$(($emb_default_cache_size - $size_7_5M))
	else
		cache_size_to_set=$emb_default_cache_size
	fi

	res=$($PHC set StreamCache.Size $cache_size_to_set) ||
		$LOGCRIT "Failed to set cache size: $res"

	__send_action_resp $action "$2" $pack
}

__remove_adp_license_key() {
	local pack=$1 action=removelicensekey packdir= resstring=

	[ -d $PACKAGE_DIRECTORY/$pack ] || {
		$LOGCRIT "Directory for package $pack is missing"
		__error 24 $pack
	}
	packdir=$PACKAGE_DIRECTORY/$pack
	resstring=$(rm $packdir/$LICFILENAME 2>&1) || {
		$LOGCRIT "$resstring"
		__error 23
	}

	__send_action_resp $action "" $pack
}

__admin_adp_package() {
	local err= action= pack= return_page= pid= xmlfile= 
	local license_status license_exp_date


	action=$1
	pack=$2
	return_page=$3
	send_response=$4

	err=4
	packdir="$PACKAGE_DIRECTORY/$pack"
	runstate_conf=$packdir/conf/runstate.conf
	if [ -r "$packdir/$ADPPACKCFG" ]; then
		. "$packdir/$ADPPACKCFG" || :
		err=0
		if [ "$APPTYPE" = lua ]; then
			xmlfile=$APPNAME
			APPNAME=${APPNAME%.*}
			pid=$(__lua_application_is_running $xmlfile)
		else
			pid=$(pidof $APPNAME) || :
		fi
		case "$action" in
			start)
				if [ "$APPTYPE" = lua ] &&
				   [ "$LUASTANDALONE" = yes ]; then
					LICENSEFILE=lic.xml
					# Check license status
					 __get_license_status "$packdir/$LICENSEFILE" license_status license_exp_date 
					case "$license_status" in
					Missing)
						err=11
						$LOGWARN "Application '$APPNAME' can not be started without license installed"
						read running_status <$packdir/is_running || :
						if [ "$running_status" = Running ]; then
							echo "Not Running" >$packdir/is_running
						fi	
						;;
					Invalid)
					    err=21
						$LOGWARN "Invalid license. Application '$APPNAME' can not be started"
						read running_status <$packdir/is_running || :
						if [ "$running_status" = Running ]; then
							echo "Not Running" >$packdir/is_running
						fi	
						;;
					Valid|None|Custom)
						read running_status <$packdir/is_running || :
						if [ "$running_status" = Running ]; then
							err=6
							$LOGWARN "Application '$APPNAME' is running"
						else
							echo Running >$packdir/is_running
						fi
						;;
					*)
					    $LOGCRIT "Get license status failed!"
						;;
		            esac
				elif [ "$pid" ]; then
					err=6
					$LOGINFO "Application '$APPNAME' is running already pid=$pid"
				else
					if eval $(__rootpath) start-package-allowed.sh $packdir/$APPNAME $APPTYPE >/dev/null; then
						echo ENABLED=yes >$runstate_conf
						eval /etc/init.d/sdk$APPNAME start >/dev/null ||
							err=8
					else
						err=9
					fi
				fi
				;;
			stop)
				if [ "$APPTYPE" = lua ] &&
				   [ "$LUASTANDALONE" = yes ]; then
					read running_status <$packdir/is_running || :
					if [ "$running_status" = "Not Running" ]; then
						err=7
						$LOGINFO "Application '$APPNAME' is not running"
					else
						echo "Not Running" >$packdir/is_running
					fi
				elif [ "$pid" ]; then
					echo ENABLED=no >$runstate_conf
					/etc/init.d/sdk$APPNAME stop >/dev/null || :
					if [ "$APPTYPE" = lua ]; then
						pid=$(__lua_application_is_running $xmlfile)
					else
						pid=$(pidof $APPNAME) || :
					fi
					if [ "$pid" ]; then
						$LOGWARN "Application $APPNAME[$pid] can not be stopped"
						err=10
					fi
				else
					$LOGINFO "Application '$APPNAME' is not running"
					err=7
				fi
				;;
			*)
				err=10
				;;
		esac
	fi

	if [ $err -ne 0 ]; then
		__error $err
	elif [ "$send_response" -eq 1 ]; then
		__send_action_resp $action $return_page
	fi
}

__autoinstall_adp_license_key() {
	local action=autoinstalllicensekey
	local urn=/techsup/compatible_applications/api.php
	local pack= packdir= response= response_code= line=
	local old_IFS=$IFS within_xml= content= new_qs= param= err=


	pack=$1
	packdir="$PACKAGE_DIRECTORY/$pack"
	[ ! -r "$packdir/$ADPPACKCFG" ] || . "$packdir/$ADPPACKCFG" || :

	if [ -z "$APPID" ]; then
		$LOGWARN "No application ID set in package.conf"
		__error 29 $pack
	fi
	if [ -z "$APPMAJORVERSION" ]; then
		$LOGWARN "No major version set in package.conf"
		__error 29 $pack
	fi
	if [ -z "$APPMINORVERSION" ]; then
		$LOGWARN "No minor version set in package.conf"
		__error 29 $pack
	fi

	IFS="&"
	for param in $QUERY_STRING; do
		case "${param%%=*}" in
			action|reload_page|package)
				;;
			*)
				new_qs=$new_qs$param"&"
				;;
		esac
	done
	IFS=$old_IFS

	urn="$urn?${new_qs}app_id=$APPID&major_ver=$APPMAJORVERSION&minor_ver=$APPMINORVERSION"
	response=$(
		cat <<EOF | nc -w 75 www.axis.com 80 2>&1
GET $urn HTTP/1.1
Host: www.axis.com
Connection: close

EOF
		)
	if [ -z "$response" ]; then
		$LOGINFO "Failed to connect to the Axis web service"
		__error 27 $pack
	fi

	response_code=$(echo $response |
		sed -rn 's/HTTP\/1\..[[:blank:]]+([[:digit:]]*).*/\1/p' ||
		response_code=)

	if [ -z "$response_code" ]; then
		$LOGCRIT "Bad response received from the Axis web service $response"
		__error 28 $pack
	fi

	mkdir -p $LICDIR || {
		$LOGCRIT "Failed to mkdir $LICDIR (2)"
		__error 22 $pack
	}
	chmod 700 $LICDIR || {
		$LOGCRIT "Failed to chmod $LICDIR (2)"
		__error 22 $pack
	}

	if ! touch $LICFILEPATH >/dev/null; then
		$LOGCRIT "Failed to create temporary license file"
		__error 28 $pack
	fi

	IFS="
"
	rm -f $LICFILEPATH.data $LICFILEPATH.raw
	for line in $response; do
		[ -z "$within_xml" ] || echo $line >>$LICFILEPATH
		echo $line >>$LICFILEPATH.raw
		echo $line >>$LICFILEPATH.data
		if [ "$line" = "<LicenseKey>" ]; then
			echo $line >$LICFILEPATH
			within_xml=1
		fi

		cr=$(printf '\r')
		if [ "$line" = "$cr" ]; then
			>$LICFILEPATH.data || :
		fi
	done
	IFS=$old_IFS

	if [ $response_code -ne 200 ]; then
		content=$(cat $LICFILEPATH.data || :)
		if [ -z "$content" ]; then
			$LOGCRIT "Bad response received from the Axis web service(2) $response"
			__error 28 $pack
		fi
		$LOGINFO "Failed to retrieve license key: $content"
		err=${content%%:*}
		if [ "$(echo $err | tr -d [0-9])" ] ||
			[ $err -le 0 ] || \
			[ $err -gt 10 ]; then
			$LOGCRIT "Failed to parse error code r=\"$response\""
			$LOGCRIT "Failed to parse error code c=\"$content\""
			$LOGCRIT "Failed to parse error code e=\"$err\""
			err=28
		fi
		__error $err $pack
	fi

	chmod 644 $LICFILEPATH || {
		$LOGCRIT "Failed to set permissions on created license file"
		__error 28 $pack
	}
	__install_adp_license_key $pack
}

__is_lua_application() {
	# Arguments
	# $1=<appname>
	local packdir

	[ $# -eq 1 ] || {
		echo "__is_lua_application: wrong number of arguments"
		return 1
	}

	[ "$1" ] || {
		echo "__is_lua_application: illegal argument"
		return 1
	}

	packdir="$PACKAGE_DIRECTORY/$1"

	[ -r "$packdir/$ADPPACKCFG" ] || {
		echo "__is_lua_application: missing package"
		return 1
	}

	. "$packdir/$ADPPACKCFG" || {
		echo "__is_lua_application: failed to read package"
		return 1
	}

	[ "$APPTYPE" ] || {
		echo "__is_lua_application: missing APPTYPE in package"
		return 1
	}

	[ "$APPTYPE" = lua ]
}


__get_application_status() {

	local dbus_resp= app_status=

	if [ "$APPTYPE" = lua ]; then
		if [ "$LUASTANDALONE" = yes ]; then
				read app_status <"$PACKAGE_DIRECTORY/${APPNAME%.*}/is_running"
				unset LUASTANDALONE
		else
			dbus_resp=$($DBUS_REQ_GET_APP_STATUS string:$PACKAGE_DIRECTORY/${APPNAME%.*}/$APPNAME 2>/dev/null)
			dbus_resp=${dbus_resp#*string \"}
			dbus_resp=${dbus_resp%\"}
			case $dbus_resp in
				Running|Idle)
					app_status=$dbus_resp
					;;
				*)
					app_status=Stopped
					;;
			esac
		fi
		APPNAME=${APPNAME%.*}
	else
		if [ "$(pidof $APPNAME)" ]; then
			app_status=Running
		else
			app_status=Stopped
		fi
	fi

	eval $1=\$app_status
}

__get_license_status() {

	local license_file=$1 lic_status= lic_date= tag=


	case $LICENSEPAGE in
		[Aa][Xx][Ii][Ss])
			if [ -r "$license_file" ]; then
				lic_status=$(licensekey_cli -a $APPNAME -i $APPID -m $APPMAJORVERSION -n $APPMINORVERSION) || return 1
				tag=ExpirationDate
				while read line; do
					case $line in
						*$tag*)
							lic_date=${line#<$tag>}
							lic_date=${lic_date%</$tag*}
							[ "$lic_date" != 0 ] || lic_date=
							break
							;;
					esac
				done <"$license_file"
			else
				lic_status=Missing
			fi
			;;
		[Cc][Uu][Ss][Tt][Oo][Mm])
			lic_status=Custom
			;;
		*)
			lic_status=None
			if [ -r "$license_file" ]; then
				lic_status=$(licensekey_cli -a $APPNAME -i $APPID -m $APPMAJORVERSION -n $APPMINORVERSION) || return 1
				tag=ExpirationDate
				while read line ; do
					case $line in
						*$tag*)
							lic_date=${line#<$tag>}
							lic_date=${lic_date%</$tag*}
							[ "$lic_date" != 0 ] || lic_date=
							break
							;;
					esac
				done <"$license_file"
			fi
			;;
	esac

	eval $2=\$lic_status
	eval $3=\$lic_date
}


__list_adp_packages() {
	local listpack= license_status= license_exp_date= p= status= ret_error= error_msg=$ADP_LISTCMD_ERROR_MSG_UNKNOWN

	local list_resp=$(mktemp $VAR_RUN_DIR/tmp_resp.XXXXXX)

	# Make sure that the package directory exists and list the name of the
	# subdirectories of the PACKAGE_DIRECTORY.
	if [ -d $PACKAGE_DIRECTORY ]; then
		cd $PACKAGE_DIRECTORY
		listpack=$(echo *)
		[ "$listpack" != '*' ] || listpack=
		for p in $listpack; do
			if [ -r $p/$ADPPACKCFG ]; then

				$(. $p/$ADPPACKCFG 2>/dev/null) || {
					ret_error=yes
					error_msg=$ADP_LISTCMD_ERROR_MSG_CONFIG
					break
				}
				. $p/$ADPPACKCFG

				__get_application_status status || {
					ret_error=yes
					error_msg=$ADP_LISTCMD_ERROR_MSG_STATUS
					break
				}

				__get_license_status "$p/$LICFILENAME" license_status license_exp_date || {
					ret_error=yes
					error_msg=$ADP_LISTCMD_ERROR_MSG_LICENSE
					break
				}

				printf ' <application Name="%s"' "$APPNAME" >> $list_resp
				[ -z "$PACKAGENAME" ]      || printf ' NiceName="%s"' "$PACKAGENAME" >> $list_resp
				[ -z "$VENDOR" ]           || printf ' Vendor="%s"' "$VENDOR" >> $list_resp
				[ -z "$APPMAJORVERSION" ]  || [ -z "$APPMINORVERSION" ] || printf ' Version="%s"' "$APPMAJORVERSION.$APPMINORVERSION" >> $list_resp
				[ -z "$APPID" ]            || printf ' ApplicationID="%s"' "$APPID" >> $list_resp
				[ -z "$license_status" ]   || printf ' License="%s"' "$license_status" >> $list_resp
				[ -z "$license_exp_date" ] || printf ' LicenseExpirationDate="%s"' "$license_exp_date" >> $list_resp
				[ -z "$status" ]           || printf ' Status="%s"' "$status" >> $list_resp
				[ -z "$SETTINGSPAGEFILE" ] || printf ' ConfigurationPage="%s"' "local/$p/$SETTINGSPAGEFILE" >> $list_resp
				[ -z "$VALIDATIONURI" ]    || printf ' ValidationResult="%s"' "$VALIDATIONURI" >> $list_resp
				printf ' />\n' >> $list_resp

				APPNAME= PACKAGENAME= VENDOR= APPMAJORVERSION= APPMINORVERSION= APPID= license_status=
				license_exp_date= status= SETTINGSPAGEFILE= VALIDATIONURI= tag= ret_error= APPTYPE=
			fi
		done
	fi

	__cgi_hdgen yes text/xml

	if [ "$ret_error" ]; then
		printf '<reply result="error">\r\n'
		[ "$APPNAME" ] || {
			[ "$p" ] && APPNAME=$p || APPNAME=error
		}
		printf ' <error type="%s" message="%s %s" />\r\n' "$ADP_LISTCMD_ERROR_TYPE" "$APPNAME:" "$error_msg"
	else
		printf '<reply result="ok">\r\n'
		while read line ; do
			printf ' %s\r\n' "$line"
		done <"$list_resp"
	fi

	printf '</reply>\r\n'

	rm -f $list_resp
}

