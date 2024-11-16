. /usr/html/axis-cgi/lib/functions.sh

[ ! -w /dev/console ] || exec 2> /dev/console

DBUS_DEST=com.axis.VirtualInput
DBUS_IFACE=com.axis.VirtualInput.Port
DBUS_OBJ=/com/axis/VirtualInput/Port/
DBUS_MTD_ACTIVATE=$DBUS_IFACE.Activate
DBUS_MTD_DEACTIVATE=$DBUS_IFACE.Deactivate

XML_HEADER='<?xml version="1.0" encoding="utf-8"?>'
XML_RESPONSE_TAG=VirtualInputResponse
XML_SCHEMA_MAJOR_VERSION=1
XML_SCHEMA_MINOR_VERSION=0
XML_SCHEMA_VERSION="SchemaVersion=\"$XML_SCHEMA_MAJOR_VERSION.$XML_SCHEMA_MINOR_VERSION\""
XML_XMLNS='xmlns="http://www.axis.com/vapix/http_cgi/virtualinput1"'
XML_XMLNS_XSI='xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"'
XML_NAME_SPACE=http://www.axis.com/vapix/http_cgi/virtualinput1
XML_XSI_SCHEMA_LOCATION="xsi:schemaLocation=\"$XML_NAME_SPACE $XML_NAME_SPACE\""
XML_WOWO="$XML_SCHEMA_VERSION $XML_XMLNS $XML_XMLNS_XSI $XML_XSI_SCHEMA_LOCATION"

MIN_PORT=1
MAX_PORT=32

panic() {
	logger -s -t${0##*/} -perr -- $*
	exit 1
}

xml_error_resp() {
	local xml_resp fname=xml_error_resp

	[ $# -ge 2 ] || panic "$fname: missing arguments"

	case $1 in
		*[!0-9]*)
			panic "$fname: error code must be a positive integer"
		;;
	esac

	xml_resp="$XML_HEADER
<$XML_RESPONSE_TAG $XML_WOWO>
	<Error>
		<GeneralError>
			<ErrorCode>
				$1
			</ErrorCode>
			<ErrorDescription>
				$2
			</ErrorDescription>
		</GeneralError>
	</Error>
</$XML_RESPONSE_TAG>"

	__cgi_content 200 OK "text/xml" "$xml_resp"
	exit 1
}

error() {
	[ $# -ge 2 ] || panic "error: missing arguments"

	xml_error_resp $1 "$2"
}

success_reply() {
	local xml_resp

	[ "$XML_RESPONSE_TAG" ] || error 40 "Internal server error: XML_RESPONSE_TAG not set"

	[ $# -eq 2 ] || error 40 "Internal server error: success() missing argument"

	case $2 in
		true|false)
		;;
		*)
			error 40 "Internal server error: Unrecognized state change"
		;;
	esac

	xml_resp="$XML_HEADER
<$XML_RESPONSE_TAG $XML_WOWO>
	<Success>
		<${1}Success>
			<StateChanged>
				$2
			</StateChanged>
		</${1}Success>
	</Success>
</$XML_RESPONSE_TAG>"

	__cgi_content 200 OK text/xml $xml_resp
	exit 0
}

missing_argument() {
	error 10 "Missing argument: $1"
}

invalid_port() {
	error 20 "Port must be an integer in the range [$MIN_PORT..$MAX_PORT]"
}

illegal_argument() {
	error 30 "Bad Request"
}

dbus_call() {
	local state_changed method

	[ $# -eq 2 ] || error 40 "Internal server error: dbus_call() missing argument"

	case $1 in
		*[!0-9]*)
			error 40 "Internal server error: port must be a positive integer"
			;;
	esac

	case $2 in
		$DBUS_MTD_ACTIVATE)
			method=Activate
		;;
		$DBUS_MTD_DEACTIVATE)
			method=Deactivate
		;;
		*)
			error 40 "Internal server error: dbus_call() invalid method"
		;;
	esac

	state_changed=$(gdbus call -y -d $DBUS_DEST -o $DBUS_OBJ$1 -m $2) ||
		error 40 "Internal server error: Unable to connect to Virtual Input Manager"

	case $state_changed in
		"(true,)")
			state_changed=true
		;;
		"(false,)")
			state_changed=false
		;;
		*)
			error 40 "Internal server error: Failed to change virtual input state"
		;;
	esac

	success_reply $method $state_changed
}

get_port() {
	local port

	[ $# -eq 1 ] || error 40 "Internal server error: get_port() missing argument"

	port=$(__qs_getparam port) || missing_argument port

	case $port in
		*[!0-9]*)
			invalid_port
		;;
		*)
			[ $port -ge $MIN_PORT ] && [ $port -le $MAX_PORT ] ||
				invalid_port
		;;
	esac

	eval $1=\$port
}

get_schemaversion() {
	local version

	[ $# -eq 1 ] || error 40 "Internal server error: get_schemaversion() missing argument"

	version=$(__qs_getparam schemaversion) || missing_argument schemaversion

	case $version in
		$XML_SCHEMA_MAJOR_VERSION)
		;;
		*)
			illegal_argument
		;;
	esac

	eval $1=\$version
}
