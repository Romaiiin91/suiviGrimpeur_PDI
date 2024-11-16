# Language cgis that only requires view access

. /usr/html/axis-cgi/lib/language/language_common.sh

__list_language() {
	local f lang
	__cgi_errhd 200

	for f in $DEFAULT_DIRECTORY/default/$LANGUAGE_INFO 		$BUILTIN_DIRECTORY/*/$LANGUAGE_INFO 		$UPLOAD_DIRECTORY/*/$LANGUAGE_INFO 		$MINI_DIRECTORY/*/$LANGUAGE_INFO; do
		[ -f $f ] && {
			. $f || {
				$LOGWARN "Failed to source $f."
				continue
			}
			echo $product_language_name
		}
	done
}

__type() {
	local product_language_type=unknown
	. $SELECTED_INFO || $LOGWARN "Could not retrieve language type."

	__cgi_errhd 200
	echo $product_language_type
}

__info() {
	local default= builtin_languages= uploaded_languages= mini_languages=
	local pictures= lang= flag tmp f ret

	for f in $UPLOAD_DIRECTORY/*/$LANGUAGE_INFO; do
		if [ -f "$f" ]; then
			ret=0
			product_language_name=
			. $f || ret=1
			flag=$(echo $product_language_name | urldecode -e |
					tr A-Z a-z)
			if [ -f "$DEFAULT_DIRECTORY/$flag.gif" ]; then
				pictures=/language/$flag.gif,$pictures
			elif [ -f "$LOCAL_DIRECTORY/$flag.gif" ]; then
				pictures=/local/$flag.gif,$pictures
			else
				pictures=,$pictures
			fi
			if [ $ret -ne 0 ]; then
				uploaded_languages=ERROR
			else
				uploaded_languages=$product_language_name,$uploaded_languages
			fi
		fi
	done
	for f in $MINI_DIRECTORY/*/$LANGUAGE_INFO; do
		[ -f $f ] || continue
		ret=0
		product_language_name=
		. $f || ret=1
		if [ $ret -ne 0 ]; then
			mini_languages=ERROR
		else
			mini_languages=$product_language_name,$mini_languages
		fi
	done
	for f in $BUILTIN_DIRECTORY/*/$LANGUAGE_INFO; do
		[ -f $f ] || continue
		ret=0
		product_language_name=
		. $f || {
			ret=1
			$LOGWARN "Could not source $f."
		}
		tmp=$(echo $product_language_name)
		if [ $ret -eq 0 ]; then

			builtin_languages=$tmp,$builtin_languages

			flag=$(echo $tmp | urldecode -e | tr A-Z a-z)

			if [ -f "$DEFAULT_DIRECTORY/$flag.gif" ]; then
				pictures=/language/$flag.gif,$pictures
			else
				pictures=,$pictures
			fi
		fi
	done

	ret=0
	. $DEFAULT_DIRECTORY/default/$LANGUAGE_INFO || {
		$LOGWARN "Could not retrieve default language info."
		ret=1
	}
	lang=$(echo $product_language_name | tr A-Z a-z)

	if [ $ret -eq 0 ] && [ -f "$DEFAULT_DIRECTORY/$lang.gif" ]; then
		pictures=/language/$lang.gif,$pictures
	else
		pictures=,$pictures
	fi

	echo "builtinlang=\"$builtin_languages\";
uploadedlang=\"$uploaded_languages\";
minilang=\"$mini_languages\";
langpics=\"$pictures\";"
}

