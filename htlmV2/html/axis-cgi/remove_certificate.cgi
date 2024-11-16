#!/bin/sh -e

# Deletes certificate files. The cgi method must be POST.
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

local cgi_interface__ cgi_interface_name__
local nof_vars__ cgi_vars__ real_vars__
local tmpvar__

tmpvar__=$(__qs_getparam interface) || :
nof_vars__=1
cgi_vars__=ethernet
real_vars__=I0
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

local return_page__
tmpvar__=$(__qs_getparam return_page) &&
	[ "$tmpvar__" ] && return_page__=$tmpvar__
tmpvar__=
local cgi_hdgen__ type__ cert_file__ nof_vars__=3
local cgi_vars__="ca_certificate client_certificate private_key"
local real_vars__="WPA_SUPPLICANT_CA_CERT WPA_SUPPLICANT_CLIENT_CERT \
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

[ -f $cert_file__ ] && {
	if \rm -f $cert_file__; then
		logger "Certificate file '$cert_file__' removed"
	else
		__cgi_errhd 500 "Removing $cert_file__ failed."
		exit 1
	fi
}


# Update the supplicant's conf-file.
/usr/sbin/mangle-wpa_supplicant-conf.sh $iface__ || {
	__load_conf /etc/dot1x/$iface__/global || exit 1
	if [ "$WPA_SUPPLICANT_AUTH_METHOD" = NONE ]; then
		__cgi_errhd 200 "802.1X disabled after removing certificate."
	else
		__cgi_errhd 500 "Internal error while removing certificate."
	fi

	exit 0
}

if [ "$cgi_hdgen__" = yes ]; then
	__cgi_errhd 200
else
	forwardurl $return_page__
fi
