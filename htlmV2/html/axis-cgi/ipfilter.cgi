#! /bin/sh
#
# Add, remove or list accepted IP addresses and/or enable/disable filter.
#
# Usage:
#	ipfilter.cgi?action={add|remove|update}
#		&ipaddress=<LIST>[&enable={yes|no}][&policy={allow|deny}]
#		[&verify={yes|no}]
#
#	ipfilter.cgi?action=removeall
#
#	ipfilter.cgi?action=list
#


. /usr/html/axis-cgi/lib/functions.sh

phc="parhandclient --nocgi"
phcns="$phc --nosync"
phcnl="$phc --nolog"

error() {
	__cgi_hdgen $CGI_HDGEN
	printf "$*\r\n"

	logger -p user.warn -t ${0##*/}[$$] "$*"
	exit 1
}

# Default values for parameters
remove_all_addrs=no
[ "$CGI_HDGEN" ] || CGI_HDGEN=yes

# Get parameters from env and store in variables
action=$(__qs_getparam action) && [ -n "$action" ] ||
	error 'Parameter "action" must  have a value!'
verify=$(__qs_getparam verify) && [ -z "$verify" ] &&
	error 'Parameter "verify" must  have a value!' || :
enable=$(__qs_getparam enable) && [ -z "$enable" ] &&
	error 'Parameter "enable" must  have a value!' || :
policy=$(__qs_getparam policy) && [ -z "$policy" ] &&
	error 'Parameter "policy" must  have a value!' || :
ipaddress=$(__qs_getparam ipaddress) && [ -z "$ipaddress" ] &&
	remove_all_addrs=yes || :

# Check values
[ "$verify" ] && [ "$verify" != yes ] && [ "$verify" != no ] &&
	error "Invalid value of verify: \"$verify\"" || :
[ "$enable" ] && [ "$enable" != yes ] && [ "$enable" != no ] &&
	error "Invalid value of enable: \"$enable\"" || :
[ "$policy" ] && [ "$policy" != allow ] && [ "$policy" != deny ] &&
	error "Invalid value of policy: \"$policy\"" || :
# Remove spaces in address list at beginning and end, and urldecode the others.
[ -z "$ipaddress" ] || ipaddress=$(echo $ipaddress | urldecode)

current_addrs=$($phcnl get Network.Filter.Input.AcceptAddresses - RAW || :)
current_policy=$($phcnl get Network.Filter.Input.Policy - RAW || :)
current_enabled=$($phcnl get Network.Filter.Enabled - RAW || :)

# Check that we got the ones we need for each action and
# generate new_addrs depending on action.
case "$action" in
	list)
		# No implicit verification for list
		[ "$verify" ] || verify=no

		new_addrs=$current_addrs
		;;
	add)
		[ "$ipaddress" ] || error "Missing parameter: 'ipaddress'"

		if [ "$current_addrs" ]; then
			new_addrs="$current_addrs $ipaddress"
		else
			new_addrs=$ipaddress
		fi
		;;
	remove)
		[ "$ipaddress" ] || error "Missing parameter: 'ipaddress'"

		case " $current_addrs " in
			*" $ipaddress "*)
				new_addrs=
				for addr in $current_addrs; do
					[ "$addr" = "$ipaddress" ] ||
						new_addrs="$new_addrs $addr"
				done
				new_addrs=${new_addrs# }
				[ "$new_addrs" ] || remove_all_addrs=yes
				;;
			*)
				error "Invalid ipadress: '$ipaddress' does" \
					"not exist in list of addresses to accept"
				;;
		esac
		;;
	removeall)
		new_addrs=
		remove_all_addrs=yes
		;;
	update)
		# Note remove_all_addrs is set at top
		if [ -z "$ipaddress" ] && [ "$remove_all_addrs" != yes ] &&
		   [ -z "$enable" ] && [ -z "$policy" ]; then
			error 'Missing parameter: "ipaddress" or "enable"'
		fi

		if [ "$ipaddress" ] || [ "$remove_all_addrs" = yes ]; then
			new_addrs=$ipaddress
		else
			new_addrs=$current_addrs
		fi
		;;
esac

# Disable firewall for empty list
[ "$remove_all_addrs" != yes ] || enable=no

# Perform verification
[ "$enable" ] && new_enabled=$enable || new_enabled=$current_enabled
[ "$policy" ] && new_policy=$policy || new_policy=$current_policy
[ "$new_enabled" != yes ] || [ "$verify" ] || verify=yes
if [ "$verify" = yes ]; then
	/usr/html/axis-cgi/verify_firewall.cgi "$new_policy" "$new_enabled" "$new_addrs" ||
		error "Verification failed: IP address \"$REMOTE_ADDR\"  is" \
			"not an accepted address"
fi

# Perform action
if [ "$action" = list ]; then
	__cgi_hdgen $CGI_HDGEN
	printf "Accept addresses: %s\r\n" "$current_addrs"
	printf "Input policy: %s\r\n" $current_policy
	printf "Enabled: %s\r\n" $current_enabled
else
	if [ "$new_addrs" ] || [ "$remove_all_addrs" = yes ]; then
		$phcns set Network.Filter.Input.AcceptAddresses \
			"$new_addrs" >/dev/null ||
			error "Failed to set parameter:" \
				"Network.Filter.Input.AcceptAddresses"
	fi
	if [ "$policy" ]; then
		$phcns set Network.Filter.Input.Policy "$policy" >/dev/null ||
			error "Failed to set parameter: Network.Filter.Input.Policy"
	fi
	if [ "$enable" ]; then
		$phcns set Network.Filter.Enabled "$enable" >/dev/null ||
			error "Failed to set parameter: Network.Filter.Enabled"
	fi
	$phc sync

	__cgi_hdgen $CGI_HDGEN
	printf "OK\r\n"
fi
