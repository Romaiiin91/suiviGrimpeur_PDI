# Collection of functions available to all other cgi-scripts (sourced).

__whoami() {
	printf "%s" "${0##*/}"
}

__cgi_hdgen() {
	# Arguments:
	#	$1	generate header if arg value is 'yes'
	#	$2	content type (default text/plain)
	#	$3	content length (if specified)
	local c_type__

	[ $# -lt 1 ] || [ "$1" != yes ] || {
		c_type__=text/plain

		[ -z "$2" ] || c_type__=$2
		printf "Cache-Control: no-cache\r
Pragma: no-cache\r
Expires: Thu, 01 Dec 1994 16:00:00 GMT\r
Content-Type: %s\r\n" "$c_type__"

		# If the calling cgi returns only a header (no body),
		# content length _must be present_ and set to 0.
		[ -z "$3" ] || printf "Content-Length: %d\r\n" $3

		# The final touch.
		printf "\r\n"
	}
}

__cgi_errcode() {
	# Arguments:
	#	$1	variable name for message text return
	#	$2	error code
	local ret__ mess__

	ret__=$1
	shift

	mess__="*** UNKNOWN ERROR CODE '$1' ***"
	case $1 in
		# Successful 2XX
		200)
			mess__=OK
			;;
		201)
			mess__=Created
			;;
		202)
			mess__=Accepted
			;;
		204)
			mess__="No Content"
			;;
		2??)
			mess__=OK
			;;

		# Redirection 3xx
		300)
			mess__="Multiple Choices"
			;;
		301)
			mess__="Moved Permanently"
			;;
		302)
			mess__="Moved Temporarily"
			;;
		304)
			mess__="Not Modified"
			;;
		3??)
			mess__="Multiple Choices"
			;;

		# Client Error 4xx
		400)
			mess__="Bad Request"
			;;
		401)
			mess__=Unauthorized
			;;
		403)
			mess__=Forbidden
			;;
		404)
			mess__="Not Found"
			;;
		409)
			mess__=Conflict
			;;
		413)
			mess__="Request Entity Too Large"
			;;
		4??)
			mess__="Bad Request"
			;;

		# Server Error 5xx
		500)
			mess__="Internal Server Error"
			;;
		501)
			mess__="Not Implemented"
			;;
		502)
			mess__="Bad Gateway"
			;;
		503)
			mess__="Service Unavailable"
			;;
		5??)
			mess__="Internal Server Error"
			;;
	esac
	eval $ret__=\$mess__
}

__cgi_content() {
	# Arguments:
	#	$1	error code
	#	$2	error message
	#	$3	content type
	#	$4..$n	optional text
	printf "Status: %d %s\r\n" $1 "$2"
	__cgi_hdgen yes $3
	[ $1 -lt 400 ] || printf "%d %s\r\n\r\n" $1 "$2"
	shift 3
	[ $# -lt 1 ] || printf "%s\r\n" "$*"
}

__cgi_errhd_ct() {
	# Arguments:
	#	$1	error code (integer) for the generated error header
	#       $2      content type
	#	$3..$n	optional text
	local code__ ctmess__ c_type__

	code__=$1

	# At least 2 arguments required
	[ $# -ge 2 ] || code__=500

	c_type__=$2

	__cgi_errcode ctmess__ $code__
	shift 2
	__cgi_content $code__ "$ctmess__" $c_type__ $*
}

__cgi_errhd() {
	# Arguments:
	#	$1	error code (integer) for the generated error header
	#	$2..$n	optional text
	local hdmess__ code__=$1

	__cgi_errcode hdmess__ $code__
	shift
	__cgi_content $code__ "$hdmess__" text/plain $*
}

__load_conf() {
	# $1 - resource file path

	[ "$1" ] || {
		__cgi_errhd 500 "__load_conf: missing argument."
		return 1
	}

	[ -f "$1" ] || {
		__cgi_errhd 500 "__load_conf: Resource file '$1' not found."
		return 1
	}

	[ -r "$1" ] || {
		__cgi_errhd 500 "__load_conf: Resource file '$1' not readable."
		return 1
	}

	. $1
}

__title() {
	# Arguments:
	#	$1	generate a title if arg value is 'yes'
	#	$2	the title string

	[ "$1" != yes ] || {
		local title_str__="### missing title string ###"

		[ -z "$2" ] || title_str__=$2
		printf "\r\n----- %s -----\r\n\n" "$title_str__"
	}
}

__qs_getparam() {
	# Arguments:
	#	$1	parameter name to search the QUERY_STRING for
	local reterr__=1 retval__= qs__="$QUERY_STRING"

	if [ "$1" ]; then
		local inparam__=$1 IFS='&' param__
		local decoded_param__ decoded_val__

		for param__ in $qs__; do
			decoded_param__=${param__%=*}
			decoded_param__=$(printf "%s\n" "$decoded_param__" | urldecode)
			[ "$decoded_param__" != $inparam__ ] || {
				decoded_val__=${param__#*=}
				if [ "$decoded_val__" ]; then
					retval__=$(printf "%s\n" "$decoded_val__" | urldecode)
				else
					# This param doesn't have a value.
					retval__=
				fi
				reterr__=0
				break
			}
		done
	else
		retval__="__qs_getparam: missing argument."
	fi

	printf "%s" "$retval__"

	return $reterr__
}

__cgi2internal_name() {
	# Maps one cgi-parameter or value name (located in array $3) to the
	# corresponding internal name (located in array $4).
	# Note: when calling this function, use _exactly_ 4 arguments and
	#	enclose arguments 3 and 4 in quotes, especially if they are
	#	lists of values.
	# $1 - cgi-parameter or value name (to be converted)
	# $2 - total number of cgi-parameter or value names
	# $3 - list of cgi-parameter or value names
	# $4 - list of internal names
	local invar__ nof_params__ outvar__= n__ tmpvar__

	[ $# -eq 4 ] || {
		__cgi_errhd 500 "__convert_var_name: missing argument(s)."
		exit 1
	}
	invar__=$1; shift
	nof_params__=$1; shift

	set -- $*

	n__=1
	while [ $n__ -le $nof_params__ ]; do
		# Pick up one name among $1..$n.
		eval tmpvar__=$(echo \$$n__) || :

		[ "$invar__" != "$tmpvar__" ] || {
			eval outvar__=$(echo \$$(($n__ + $nof_params__)))
			break
		}
		n__=$(($n__ + 1))
	done

	if [ "$outvar__" ]; then
		printf "%s" $outvar__
	else
		__cgi_errhd 500 "__convert_var_name: invalid name '$invar__'."
		exit 1
	fi
}

__rootpath() {
	# Only for very well motivated use.
	# May be called in two ways.
	# Recommanded (does not spill the PATH variable to the script env.):
	#	eval "$(__rootpath) <priviledged command and arguments>"
	# NOT recommanded (obvious what this does):
	#	export $(__rootpath)
	echo "PATH=/usr/sbin:/sbin:$PATH"
}

__escape_single_quotes() {
	# Breaks out single quotes, escapes them and concatenates to a
	# string made up of multiple single quoted strings. This can be
	# used to make sure that all special characters can be used in
	# a string (for exable a user defined pass phrase) without risk
	# of execution of constructions in the form $(command arg) or
	# `command arg`
	# Any ' found in the string is replaced with: '\''
	# The result itself is enclosed within single quotes.
	# Example: test'pass -> 'test'\''pass'
	echo "$1" | sed -re "s/'/'\\\\''/g" -e "s/^/'/" -e "s/$/'/"
}
