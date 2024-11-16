#!/bin/sh -e

# Show usable certificate files. The cgi method must be GET.
#
# Knows about the following QUERY_STRING parameters:
#
#   generate_header	{yes|no} (default yes)
#   type		ca_certificate, client_certificate, private_key
#   interface		{ethernet|wlan}
#			  defaults to ethernet

#exec 2>>/tmp/${0##*/}.log
#set -x
#set >&2

. /usr/html/axis-cgi/lib/functions.sh

local nof_vars__ cgi_vars__ real_vars__
local tmpvar__ cgi_interface__

tmpvar__=$(__qs_getparam interface) || :
nof_vars__=1
cgi_vars__=ethernet
real_vars__=I0
if [ "$tmpvar__" ]; then
	cgi_interface__=$(__cgi2internal_name $tmpvar__ $nof_vars__ \
				"$cgi_vars__" "$real_vars__") || {
		__cgi_errhd 400 "Invalid interface;" \
			"available {"$(echo $cgi_vars__ | tr ' ' ',')"}"
		exit 1
	}
else
	cgi_interface__=I0
fi

# Resources.
local ph_param_name__=Network.Interface.${cgi_interface__}.SystemDevice iface__
iface__=$(parhandclient --nocgi get $ph_param_name__ - RAW) || {
	__cgi_errhd 500 "Interface name not available."
	exit 1
}

local wpa_conf__=/etc/dot1x/$iface__/global
[ -f $wpa_conf__ ] || {
	__cgi_errhd 500 "Resource file $wpa_conf__ not found."
	exit 1
}
. $wpa_conf__

local tmp__ wpaparam__= ret__=
local CGI_HDGEN__=yes
nof_vars__=3
cgi_vars__="ca_certificate client_certificate private_key"
real_vars__="WPA_SUPPLICANT_CA_CERT WPA_SUPPLICANT_CLIENT_CERT \
WPA_SUPPLICANT_PRIVATE_KEY"

__get_certparam () {
	[ "$1" ] || {
		__cgi_errhd 500 "__get_certparam: missing argument."
		exit 1
	}

	local val__
	eval val__=$(echo \$$1)

	echo -n "$val__"
	return 0
}

__show_certparam () {
	[ "$1" ] || {
		__cgi_errhd 500 "__show_certparam: missing argument."
		exit 1
	}

	local val__ vname__ vsiza__ vdate__

	val__=$(__get_certparam $1) || :
	if [ "$val__" ] && [ -f $val__ ]; then
		if [ -L $val__ ]; then
			val__=
		else
			val__=$(ls -le $val__ | \
				sed	-e 's|[[:space:]]\+| |g' \
					-e 's|^\([^/]*\)/.*/\([^/]\+\)$|\1\2|' | \
				cut -d' ' -f5-)
			vname__=$(expr "$val__" : '.*[[:space:]]\([^[:space:]]\+\)')
			vsize__=$(expr "$val__" : '\([0-9]\+\)[[:space:]].*')
			vdate__=$(expr "$val__" : '[0-9]\+[[:space:]][A-Za-z]\+[[:space:]]\([A-Za-z]\+[[:space:]][0-9]\+[[:space:]][0-9:[:space:]]\+\).*')
			val__="$vname__, $vsize__, $vdate__"
		fi
	else
		if [ $CGI_HDGEN__ = yes ]; then
			val__="Certificate file not found"
		else
			val__=
		fi
	fi

	echo -n "$val__"

	return 0
}

tmp__=$(__qs_getparam generate_header) && [ "$tmp__" ] && CGI_HDGEN__=$tmp__
tmp__=

if ! tmp__=$(__qs_getparam type) && [ -n "$tmp__" ]; then
	__cgi_errhd 400 "Certificate file type missing or empty;" \
		"available {"$(echo "$cgi_vars__" | tr ' ' ',')"}"
	exit 1
fi

if ! wpaparam__=$(__cgi2internal_name $tmp__ $nof_vars__ \
		    "$cgi_vars__" "$real_vars__") && [ "$wpaparam__" ]; then
	__cgi_errhd 400 "Invalid certificate file type;" \
		"available {"$(echo "$cgi_vars__" | tr ' ' ',')"}"
	exit 1
fi

ret__=$(__show_certparam $wpaparam__)

if [ "$CGI_HDGEN__" = yes ]; then
	__cgi_errhd 200 "$ret__"
else
	echo -n "$ret__"
fi
