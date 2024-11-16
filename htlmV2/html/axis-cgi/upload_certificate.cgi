#!/bin/sh -e

# Takes care of uploading certificate files. The cgi method must be POST.
#
# Edits the wpa_supplicant configuration file.
#
# Knows about the following QUERY_STRING parameters:
#
#   generate_header	{yes|no} (default yes)
#   type		{ca_certificate|client_certificate|private_key}
#   interface		{ethernet|wlan}
#			  defaults to ethernet
#   return_page		URL encoded return path

#exec 2>>/tmp/${0##*/}.log
#set -x
#set >&2

. /usr/html/axis-cgi/lib/functions.sh
. /lib/rcscripts/sh/files.sh

local nof_vars__ cgi_vars__ real_vars__
local tmpvar__ cgi_interface__ cgi_interface_name__

tmpvar__=$(__qs_getparam interface) || :
nof_vars__=1
cgi_vars__=ethernet
real_vars__="I0"
cgi_interface_name__=ethernet
cgi_interface__=I0
if [ "$tmpvar__" ]; then
	cgi_interface__=$(__cgi2internal_name $tmpvar__ $nof_vars__ \
				"$cgi_vars__" "$real_vars__") || {
		__cgi_errhd 400 "Invalid interface;" \
			"available {"$(echo $cgi_vars__ | tr ' ' ',')"}"
		exit 1
	}
	cgi_interface_name__=$tmpvar__
fi

# Resources.
local ph_param_name__=Network.Interface.${cgi_interface__}.SystemDevice iface__
iface__=$(parhandclient --nocgi get $ph_param_name__ - RAW) || {
	__cgi_errhd 500 "Interface name not available."
	exit 1
}

local return_page__ cgi_hdgen__ type__ cert_file__ fpath__
tmpvar__=$(__qs_getparam return_page) &&
	[ "$tmpvar__" ] && return_page__=$tmpvar__
tmpvar__=

nof_vars__=3
cgi_vars__="ca_certificate client_certificate private_key"
real_vars__="WPA_SUPPLICANT_CA_CERT WPA_SUPPLICANT_CLIENT_CERT \
WPA_SUPPLICANT_PRIVATE_KEY"

# Sourcing conf-files.
__load_conf /etc/dot1x/$iface__/global || exit 1

if [ -z "$return_page__" ]; then
	return_page__="/admin/8021x.shtml?id=$(date +%H%M%S)"
fi
cgi_hdgen__=yes

tmpvar__=$(__qs_getparam generate_header) &&
	[ "$tmpvar__" ] && cgi_hdgen__=$tmpvar__
tmpvar__=

# 'type' param
tmpvar__=$(__qs_getparam type) &&
	[ "$tmpvar__" ] && type__=$tmpvar__
[ "$type__" ] || {
	__cgi_errhd 400 "Missing type parameter."
	exit 1
}
type__=$(__cgi2internal_name \
		$type__ $nof_vars__ "$cgi_vars__" "$real_vars__") || {
	__cgi_errhd 400 "Invalid certificate type;" \
		"available {"$(echo "$cgi_vars__" | tr ' ' ',')"}"
	exit 1
}
eval cert_file__=$(echo \$$type__)

# cover for missing variable
[ "$CONTENT_LENGTH" ] || CONTENT_LENGTH=0

fpath__=$(file_upload -s $CONTENT_LENGTH) || {
	forwardurl "/admin/8021x.shtml?uploaderror=1" 0
	exit 1
}

if [ "$fpath__" ]; then
	if [ $type__ != WPA_SUPPLICANT_CA_CERT ] && exists certinfo; then
		timevalid=$(certinfo --timevalid "$fpath__" 2>&1) || {
			:
		}
		timevalid=${timevalid#*: }
		case $timevalid in
			not-yet-active)
				error="&error=1"
				forwardurl $return_page__$error 0
				exit 1
				;;
			expired)
				error="&error=2"
				forwardurl $return_page__$error 0
				exit 1
				;;
			active)
				;;
			*)
				if [ "$timevalid" = "Could not find certificate." ]; then
					logger -s -t${0##*/} -pinfo "Could not verify certificate in uploaded file."
				else
					__cgi_errhd 500 "Error checking certificate valid time."
					exit 1
				fi
				;;
		esac
	fi

	if \mv -f "$fpath__" $cert_file__; then
		logger "Certificate file '$cert_file__' in place"
	else
		__cgi_errhd 500 "Moving '$fpath__' -> '$cert_file__' failed."
		exit 1
	fi
else
	__cgi_errhd 500 "Missing upload file path."
	exit 1
fi

if [ "$cgi_interface_name__" = ethernet ]; then
	# Update the supplicant's conf-file.
	/usr/sbin/mangle-wpa_supplicant-conf.sh $iface__
else
	# Notify WLAN wpa_supplicant.
	:
fi

if [ "$cgi_hdgen__" = yes ]; then
	__cgi_errhd 200
else
	forwardurl $return_page__
fi
