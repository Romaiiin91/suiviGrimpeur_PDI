# When copying data between file systems, use a buffer big enough to not loose
# performance. Some implementations of cp doesn't do this automatically so dd
# is used below.
BLOCK_SIZE=8192

. /usr/html/axis-cgi/lib/language/language_common.sh


__error() {
	# Arguments:
	#	$1 error=<nr>
	#	$2 exit status
	local cmd="forwardurl $FWDURL"

	[ $# -eq 2 ] && [ "$2" ] || exit 1
	case $2 in
		*[!0-9]*)
			exit 1
			;;
	esac
	[ -z "$1" ] || cmd="$cmd?$1"
	$cmd 0
	[ $2 -eq 0 ] || exit $2
}

__language_match() {
	# Arguments:
	#	$1 Langauge file to test
	# $2 Language code or name to match. In lower case.
	local country_code language_name

	[ $# -eq 2 ] || exit 1

	[ "$1" ] && [ -f "$1" ] && [ -r "$1" ] || return 1
	. $1 || {
		$LOGWARN "Could not source $2."
		return 1
	}

	country_code=$(echo $www_country_code | tr A-Z a-z)
	[ "$2" != "$country_code" ] || return 0
	language_name=$(echo $product_language_name | tr A-Z a-z)
	[ "$2" = "$language_name" ]
}

__language_match_dir() {
	# Arguments:
	# $1 Directory to check.
	#    If no language.info is located in directory look in
	#    $1/[a-z][a-z]
	# $2 language code or name to match. Assumed to be lower case.
	# return 0 if matched or 1 if no match.
	local match_name f

	[ $# -eq 2 ] || exit 1
	match_name=$(echo $2 | tr A-Z a-z)

	[ -d "$1" ] ||  return 1
	for f in $1/*/$LANGUAGE_INFO; do
		[ -f $f ] || continue
		! __language_match $f $match_name || {
			echo ${f%/*}
			return 0
		}
	done
	return 1
}

__verify_hwid() {
	# Arguments:
	#	$1 hwid from uploaded language file
	local rest_lf_hwid

	[ $# -eq 1 ] || exit 1

	# If HWID from bootblock contains '.' it belongs to a range and we must
	# only use the characters before the '.' for comparison.
	BBT_HWID=$(bootblocktool -nocgi -x HWID) || {
		LF_HWID=
		$LOGCRIT "Could not get boot block parameters."
		return 0
	}

	BBT_HWID=${BBT_HWID%%.*}
	LF_HWID=${1%%:*}
	rest_lf_hwid=${1#$LF_HWID}
	rest_lf_hwid=${rest_lf_hwid#:}

	# The hwid from the language file may have several HWIDs separated by
	# colon.
	while [ "$LF_HWID" ] && [ "$LF_HWID" != "$BBT_HWID" ]; do
		LF_HWID=${rest_lf_hwid%%:*}
		rest_lf_hwid=${rest_lf_hwid#$LF_HWID}
		rest_lf_hwid=${rest_lf_hwid#$:}
	done
}

__restore_old_langfiles() {
	local f

	if [ "$BACKUP_DIRECTORY" ] &&
	   ls $BACKUP_DIRECTORY/* >/dev/null 2>&1; then

		rm -rf "$UPLOAD_DIRECTORY/*" || :

		for f in $BACKUP_DIRECTORY/*; do
			if [ -f "$f" ]; then
				dd if=$f 				   of=$UPLOAD_DIRECTORY/${f##*/} 				   bs=$BLOCK_SIZE 2>/dev/null ||
					$LOGWARN "Could not copy $f."
			fi
		done

	fi

	rm -rf "$BACKUP_DIRECTORY" || :
}

__copy_uploaded_language_files() {
	# Arguments:
	#	$1 Path to uploaded directory.
	#	$2 Temporary directory to upload to.
	#	$3 Country code.
	local directory f dstdir

	[ $# -eq 3 ] || exit 1
	directory=$1
	dstdir=$2

	# Backup current language file to make it possible to restore if
	# EXIT is received.
	BACKUP_DIRECTORY=$(mktemp -d /tmp/language.XXXXXX) ||
		$LOGWARN "Could not create a backup directory."
	trap __restore_old_langfiles EXIT
	[ -z "$BACKUP_DIRECTORY" ] || {
		cp -pr "$UPLOAD_DIRECTORY" "$BACKUP_DIRECTORY" ||
			$LOGWARN "Could not copy backup files."
	}

	mkdir -p $UPLOAD_DIRECTORY/$3 || {
		$LOGCRIT "Could not create directory for language file."
		return 1
	}
	chmod 0755 $UPLOAD_DIRECTORY/$3 || $LOGWARN "Could not set permissions"
	cp -r $directory/* $dstdir || $LOGCRIT "Failed to copy language file."
	for f in $directory/*.css; do
		if [ -f "$f" ]; then
			dd if=$f 			   of=$dstdir/${f##*/} 			   bs=$BLOCK_SIZE 2>/dev/null ||
				$LOGWARN "Failed to copy css file."
		fi
	done

	for f in $directory/*.gif; do
		[ -f "$f" ] || continue
		cp $f $LOCAL_DIRECTORY || $LOGWARN "Failed to copy flag."
	done

	chmod 0755 $dstdir || $LOGWARN "Could not set permission for $dstdir."
	chmod 0644 $dstdir/* || $LOGWARN "Could not set permission in $dstdir."
}

__set_default_language() {
	local s

	rm $SELECTED_DIRECTORY || :
	ln -sf "$DEFAULT_DIRECTORY/default" "$SELECTED_DIRECTORY" ||
		$LOGCRIT "Failed to link language files."
	# Clean up the style sheet directory before setting new links.
	rm -rf $LANGUAGE_DIRECTORY/css/*.css || :

	for s in $DEFAULT_DIRECTORY/css/*.css; do
		if [ -f "$s" ]; then
			ln -sf "$s" "$LANGUAGE_DIRECTORY/css/${s##*/}" ||
				$LOGWARN "Could not link $s."
		fi
	done
}

__set_builtin_language() {
	# Arguments:
	#	$1 Path to builtin language directory.
	local sheet code s

	[ $# -eq 1 ] || exit 1

	rm $SELECTED_DIRECTORY/ || :
	ln -sf "$1" "$SELECTED_DIRECTORY" || {
		$LOGCRIT "Failed to link language files."
		ln -sf "$DEFAULT_DIRECTORY/default" "$SELECTED_DIRECTORY" ||
			$LOGCRIT "Could not link default language."
	}

	# Get the country abbreviation also known as
	# country code from the filename.
	. $1/$LANGUAGE_INFO || {
		www_country_code=
		$LOGWARN "Failed to source $1/$LANGUAGE_INFO."
	}
	code=$www_country_code

	# Clean up the style sheet directory before setting new links.
	rm -rf $LANGUAGE_DIRECTORY/css/*.css || :

	for s in $DEFAULT_DIRECTORY/css/*.css; do
		sheet=${s##*/}
		if [ -f "$1/css_${code}/$sheet" ]; then
			ln -sf "$1/css_${code}/$sheet" 			       "$LANGUAGE_DIRECTORY/css/${code}_$sheet" ||
				$LOGWARN "Failed to link $s."
		elif [ -f "$1/$sheet" ]; then
			ln -sf "$1/$sheet" 			       "$LANGUAGE_DIRECTORY/css/$sheet" ||
				$LOGWARN "Failed to link $s."
		else
			ln -sf "$s" 			       "$LANGUAGE_DIRECTORY/css/$sheet" ||
				$LOGWARN "Failed to link $s."
		fi
	done
}

__upload_language() {
	local ret=0 default fail_msg="set_language=no" tmp_file
	local lf_info info_file memory_free lang_code=

	[ $# -lt 1 ] || [ -z "$1" ] || FWDURL=$1

	# Don't fill tmpfs. Save at least 200kB for unpacking the tar file.
	tmp_file=$(file_upload -S $CONTENT_LENGTH -n 204800) || ret=$?
	[ $ret -eq 0 ] || {
		$LOGWARN "Failed to upload language file."
		__error "error=3" $ret
	}

	. /usr/share/axis-release/variables ||
		$LOGWARN "Failed to source axis variables."

	# Put files from the uploaded language tar file in a
	# temporary directory.
	tmp_unpacked=$(mktemp -d /tmp/language.XXXXXX) || {
		$LOGWARN "Failed to create temporary language file."
		rm -f tmp_file || :
		__error "error=1" 1
	}
	chmod -R 0755 $tmp_unpacked || :
	cd $tmp_unpacked
	ret=0
	tar -xf $tmp_file || ret=$?
	rm -f $tmp_file || :

	[ $ret -eq 0 ] || {
		cd -
		rm -fr $tmp_unpacked || :
		$LOGWARN "Failed to unpack language file."
		__error "error=1" $ret
	}

	lang_code=$(ls -1) || ret=$?
	[ $ret -eq 0 ] || {
		cd -
		rm -fr $tmp_unpacked || :
		__error "error=1" $ret
	}
	cd -
	. $tmp_unpacked/$lang_code/language.info || ret=$?
	[ $ret -eq 0 ] || {
		$LOGWARN "Could not retrieve language info from uploaded file."
		rm -fr $tmp_unpacked || :
		__error "error=2" $ret
	}

	__verify_hwid "$product_hwid" || ret=$?

	if [ "$BBT_HWID" = "$LF_HWID" ] && [ $ret -eq 0 ]; then
		. $SELECTED_INFO || {
			$LOGWARN "Could source source selected langauged."
			product_language_type=
			www_country_code=
		}
		if [ "$product_language_type" = mini ] &&
		   [ "$www_country_code" != "$lang_code" ]; then
			# If this is a call from language_selection2, type=mini,
			# only accept the language already set.
			rm -rf $tmp_unpacked || :
			__error "error=6" 1
		fi
		fail_msg="set_language=yes"

		tmp_etc=$(mktemp -d $UPLOAD_DIRECTORY/language.XXXXXX) || {
			$LOGCRIT "Failed to create temporary language directory."
			rm -rf $tmp_unpacked || :
			rm -rf $BACKUP_DIRECTORY || :
			__error "error=1" 1
		}
		__copy_uploaded_language_files "$tmp_unpacked/$lang_code" 			"$tmp_etc" "$lang_code" || ret=1

		# Make sure 64kB is still available in the destination
		# filesystem. We need to do this test this late because the
		# destination file system may compress the files.
		memory_free=$(df -k "$UPLOAD_DIRECTORY" |
				sed -e '1d;s/.*[[:blank:]]\([0-9]\+\)[[:blank:]]\+[0-9]\+%[[:blank:]]\+.*/\1/') || memory_free=0
		if [ $memory_free -lt 64 ] || [ $ret -ne 0 ]; then
			$LOGINFO "No room for language."
			rm -rf $tmp_etc || :
			fail_msg="error=3"
		else
			[ ! -d $UPLOAD_DIRECTORY/$lang_code ] || {
				# Remove old language files.
				rm -rf $UPLOAD_DIRECTORY/$lang_code || :
			}
			mv $tmp_etc/ $UPLOAD_DIRECTORY/$lang_code ||
				$LOGWARN "Failed to move temporary file."
			rm -rf $BACKUP_DIRECTORY || :
			trap - EXIT
		fi

		product_release=
		product_build=
		. $UPLOAD_DIRECTORY/$lang_code/$LANGUAGE_INFO ||
			$LOGWARN "Failed to source uploaded language info."
		if [ "$RELEASE" = "$product_release" ] &&
		   [ "$BUILD" = "$product_build" ]; then
			# Fully compatible.
			fail_msg=$fail_msg
		elif [ "$RELEASE" != "$product_release" ] ||
		     [ "$BUILD" != "$product_build" ]; then
			# Partly compatible.
			fail_msg="error=5&$fail_msg"
		else
			# Something is wrong.
			rm -fr $UPLOAD_DIRECTORY/$lang_code || :
			fail_msg="error=2"
		fi
	else
		fail_msg="error=2"
	fi

	rm -rf $tmp_unpacked || :
	rm -rf $BACKUP_DIRECTORY || :

	if [ -z $fail_msg ]; then
		forwardurl "$FWDURL" 0
	else
		__error "$fail_msg" $ret
	fi

	unset RELEASE
	unset BUILD
	unset BBT_HWID
	unset LF_HWID
	unset BACKUP_DIRECTORY
}

__set_language() {
	# $1 Language to be set.
	# $2 Page to return (optional).
	local language notify_language_handler builtin_languages f tmp tmp_top
	local return_page= found=0 ret=0

	[ $# -ge 1 ] || exit 1

	language=$(echo "$1" | tr A-Z a-z)
	[ $# -lt 2 ] || return_page=$2

	if [ -z "$language" ]; then
		[ "$HTTP_ACCEPT_LANGUAGE" ] || {
			__cgi_errhd 503
			return 1
		}
		language=$HTTP_ACCEPT_LANGUAGE
		[ ${#language} -le 2 ] || {
			tmp=${language#??}
			language=${language%$tmp}
		}
		language=$(echo $language | tr A-Z a-z)
		[ "$language" != en ] || language=default

		. $SELECTED_INFO || $LOGWARN "Failed to source $SELECTED_INFO"
		[ "$product_language_type" != uploaded ] || {
			# Do not set language if we already have set it once.
			if [ "$return_page" ]; then
				forwardurl "$return_page" 2
			else
				__cgi_errhd 200
			fi
			return 0
		}
		if language_dir=$(__language_match_dir $MINI_DIRECTORY $language); then
			__set_builtin_language $language_dir
		else
			__set_default_language
		fi
		found=1
	fi
	if [ "$language" = default ]; then
		__set_default_language
		found=1
	elif [ $found -eq 0 ]; then
		# Check for uploaded language file.
		! language_dir=$(__language_match_dir $UPLOAD_DIRECTORY $language) || {
			found=1
			__set_builtin_language $language_dir
		}

		# If no match for the uploaded language try builtin languages.
		[ $found -ne 0 ] || {
			! language_dir=$(__language_match_dir $BUILTIN_DIRECTORY $language) || {
				found=1
				__set_builtin_language $language_dir
			}
		}
		# Last try the default language.
		[ $found -ne 0 ] || {
			! __language_match $DEFAULT_DIRECTORY/default/$LANGUAGE_INFO $language || {
				found=1
				__set_default_language
			}
		}
	fi
	if [ $found -eq 1 ]; then
		if [ "$return_page" ]; then
			forwardurl "$return_page" 2
		else
			__cgi_errhd 200
		fi
		ret=0
		. $SELECTED_INFO || ret=1
		tmp_top=$(mktemp $LANGUAGE_DIRECTORY/top_tmp.XXXXXX) || ret=1
		if [ $ret -eq 0 ]; then
			sed -re 's/(^[[:blank:]]*<meta.*[[:blank:]]charset=).*(".*)/\1'$www_charset'\2/' 				$DEFAULT_DIRECTORY/default/top_incl.shtml >$tmp_top || ret=1
			sed -i -re 's/(^[[:blank:]]*<meta[[:blank:]]+http-equiv="Content-language".*CONTENT=\").*(\".*)/\1'$www_country_code'\2/' $tmp_top || ret=1
			if [ $ret -eq 0 ]; then
				mv $tmp_top $LANGUAGE_DIRECTORY/top_incl.shtml || :
			else
				$LOGWARN "Could not create top inlcude file."
				cp $DEFAULT_DIRECTORY/default/top_incl.shtml 					$LANGUAGE_DIRECTORY/top_incl.shtml ||
						$LOGWARN "Could not create top include file."
			fi
			ret=0
			sed -re 's/(^[[:blank:]]*<meta.*[[:blank:]]charset=).*(".*)/\1'$www_charset'\2/' 				$DEFAULT_DIRECTORY/default/top_incl_popup.shtml >$tmp_top || ret=1
			sed -i -re 's/(^[[:blank:]]*<meta[[:blank:]]+http-equiv="Content-language".*CONTENT=\").*(\".*)/\1'$www_country_code'\2/' $tmp_top || ret=1
			if [ $ret -eq 0 ]; then
				mv $tmp_top $LANGUAGE_DIRECTORY/top_incl_popup.shtml || :
			else
				$LOGWARN "Could not create top inlcude file."
				cp $DEFAULT_DIRECTORY/default/top_incl_popup.shtml 					$LANGUAGE_DIRECTORY/top_incl_popup.shtml ||
						$LOGWARN "Could not create top include file."

			fi
			ret=0
			sed -re 's/(^[[:blank:]]*<meta.*[[:blank:]]charset=).*(".*)/\1'$www_charset'\2/' 				$DEFAULT_DIRECTORY/default/page_header.shtml >$tmp_top || ret=1
			sed -i -re 's/(^[[:blank:]]*<meta[[:blank:]]+http-equiv="Content-language".*CONTENT=\").*(\".*)/\1'$www_country_code'\2/' $tmp_top || ret=1
			if [ $ret -eq 0 ]; then
				mv $tmp_top $LANGUAGE_DIRECTORY/page_header.shtml || :
			else
				$LOGWARN "Could not create head inlcude file."
				cp $DEFAULT_DIRECTORY/default/page_header.shtml 					$LANGUAGE_DIRECTORY/page_header.shtml ||
						$LOGWARN "Could not create head include file."

			fi
			ret=0
			sed -re 's/(^[[:blank:]]*<meta.*[[:blank:]]charset=).*(".*)/\1'$www_charset'\2/' 				$DEFAULT_DIRECTORY/default/popup_header.shtml >$tmp_top || ret=1
			sed -i -re 's/(^[[:blank:]]*<meta[[:blank:]]+http-equiv="Content-language".*CONTENT=\").*(\".*)/\1'$www_country_code'\2/' $tmp_top || ret=1
			if [ $ret -eq 0 ]; then
				mv $tmp_top $LANGUAGE_DIRECTORY/popup_header.shtml || :
			else
				$LOGWARN "Could not create head inlcude file."
				cp $DEFAULT_DIRECTORY/default/popup_header.shtml 					$LANGUAGE_DIRECTORY/popup_header.shtml ||
						$LOGWARN "Could not create head include file."

			fi
			chmod 0644 $LANGUAGE_DIRECTORY/top_incl.shtml ||
				$LOGWARN "Failed to set top include mode."
			chmod 0644 $LANGUAGE_DIRECTORY/top_incl_popup.shtml ||
				$LOGWARN "Failed to set popup include mode."
			chmod 0644 $LANGUAGE_DIRECTORY/page_header.shtml ||
				$LOGWARN "Failed to set head include mode."
			chmod 0644 $LANGUAGE_DIRECTORY/popup_header.shtml ||
				$LOGWARN "Failed to set head popup include mode."
		fi
	else
		if [ "$return_page" ]; then
			forwardurl "$return_page" 0
		else
			__cgi_errhd 503
		fi
	fi
}

__remove_language() {
	# Arguments:
	#	$1 Language to be remove.
	#	$2 Page to return (optional).
	local to_remove= language found=0 in_use f return_page=

	[ $# -ge 1 ] || exit 1

	language=$(echo "$1" | tr A-Z a-z)
	[ -z "$2" ] || return_page=$2

	if [ "$language" = default ]; then
		found=1
	else
		# Try uploaded.
		for f in $UPLOAD_DIRECTORY/*/$LANGUAGE_INFO; do
			[ -f $f ] || continue
			. $f ||  {
				$LOGWARN "Could not source $f."
				product_language_name=
			}
			[ "$(echo $product_language_name | tr A-Z a-z)" != "$language" ] || {
				found=2
				to_remove=${f%/*}
				break
			}
		done

		# Builtin language files are located in
		# /usr/html/language/builtin/XX/.
		if [ $found -eq 0 ]; then
			for f in $BUILTIN_DIRECTORY/*/$LANGUAGE_INFO; do
				[ -f $f ] || continue
				. $f || {
					$LOGWARN "Could not source $f."
					product_language_name=
				}
				[ "$(echo $product_language_name | tr A-Z a-z)" != "$language" ] || {
					found=1
					break
				}
			done
		fi
	fi

	if [ $found -eq 1 ]; then
		if [ "$return_page" ]; then
			__error "error=4" 0
		else
			__cgi_errhd 403
		fi
	elif [ $found -eq 2 ]; then
		in_use=$(readlink $SELECTED_DIRECTORY) ||
			$LOGWARN "Could not determine current language."
		if [ "$in_use" = "$to_remove" ]; then
			__set_language default "$return_page"
			rm -rf "$to_remove" || :
		else
			rm -rf "$to_remove" || :

			if [ "$return_page" ]; then
				forwardurl "$return_page" 0
			else
				__cgi_errhd 200
			fi
		fi
	else
		if [ "$return_page" ]; then
			forwardurl "$return_page" 0
		else
			__cgi_errhd 404
		fi
	fi
}
