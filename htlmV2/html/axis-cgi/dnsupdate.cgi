#!/bin/sh

. /usr/html/axis-cgi/lib/functions.sh

if [ ! "$QUERY_STRING"  ]
then
	__cgi_errhd 400 "No command issued"
	exit 1
fi

hdgen=__qs_getparam hdgen

while [ "$QUERY_STRING" ]; do

	cmd=${QUERY_STRING%%=*}
	QUERY_STRING=${QUERY_STRING#"$cmd"=}
	val=${QUERY_STRING%%&*}
	QUERY_STRING=${QUERY_STRING#"$val"}
	QUERY_STRING=${QUERY_STRING#&}

	[ "$cmd" -a "$val" ] || break

	case "$cmd" in
		add)
			if [ "$hdgen" = "yes" ]
			then
				if /usr/sbin/dnsupdate.script add "$val" all
				then
					__cgi_hdgen yes
					echo -e "Registered new address on dns.\r"
				else
					__cgi_errhd 400
					echo -e "Failed to register new address on dns.\r"
				fi
			else
				/usr/sbin/dnsupdate.script add "$val" all
			fi
			;;
		delete)
			if [ "$hdgen" = "yes" ]
			then
				if /usr/sbin/dnsupdate.script delete "$val"
				then
					__cgi_hdgen yes
					echo -e "Unregistered old address from dns.\r"
				else
					__cgi_errhd 400
					echo -e "Failed to unregister old address from dns.\r"
				fi
			else
				/usr/sbin/dnsupdate.script delete "$val"
			fi
			;;
		*)
			;;
	esac
done

exit 0
