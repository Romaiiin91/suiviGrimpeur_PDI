#!/bin/sh -e

# CGI parameters                                default
#       generate_header=yes|no                  yes

. /usr/html/axis-cgi/lib/functions.sh

CGI_HDGEN=yes

croak() {
	[ $CGI_HDGEN != yes ] || __cgi_errhd 500 "$*"
	echo "$*" >&2
	exit 1
}

parse_port_addr() {
	# Function parses addresses:port on IPv4, IPV6 and IPv4-mapped IPv6
	# format and separates address and port.
	# $1 - Input
	# $2 - variable to save address
	# $3 - variable to save port
	local _addr=$1 _port

	_port=${_addr##*:} # Parse out local port
	_addr=${_addr%:*} # Remove trailing :port

	case $_addr in
		*:*.*)
			_addr=${_addr##*:} # Remove ::ffff:
			;;
	esac
	eval $2=\$_addr && eval $3=\$_port
}

get_connection_list() {
	# Getting connection information from netstat and presents it
	# Only looks at ESTABLISHED connections
	local prot recq sendq localaddr remaddr status pid_prog pid process
	local service_name ign pname nname portptot serv_f=/etc/services
	local stat_f print_header=1 progset=0
	#	      ip    proto service/port process
	local format='%-39s %-9s %-22s %-40s\n'

	netstat -Wnutp 2>/dev/null |
	while read prot recq sendq localaddr remaddr status pid_prog; do
		# Default values
		pid=-1
		process=unknown
		service_name=unknown

		# Check that status == ESTABLISHED
		[ "$status" = ESTABLISHED ] || continue

		# Parse port and addr to separate variables
		# and also remove ::ffff: prefix if it exists
		parse_port_addr $localaddr localaddr localport ||
			croak "Failed to parse local address and port"
		parse_port_addr $remaddr remaddr ign ||
			croak "Failed to parse remote address and port"

		# Get the pid from netstat
		case $pid_prog in
			*/*)
				pid=${pid_prog%/*}
				;;
		esac
		# guard against no permission to lookup pid
		if [ $pid -le 0 ]; then
			pid="-"
		else
			# OK netstat -p is bugged when reading program name,
			# it reads from /proc/pid/cmdline, no thx, parse
			# /proc/pid/stat instead
			stat_f=/proc/$pid/stat
			while read ign pname ign; do
				process=${pname##*(}
					process=${process%)*}
			done <$stat_f || croak "Couldn't read file '$stat_f'"
		fi

		# read /etc/services and map name to port/protocol
		while read nname portprot ign; do
			case $portprot in
				$localport/$prot)
					service_name=$nname
					break
					;;
			esac
		done <$serv_f || croak "Couldn't read file '$serv_f'"

		# Formatting of table, both header and values
		# Print header just once
		[ $print_header -ne 1 ] || {
			printf "$format" IP PROTOCOL "SERVICE(PORT)" "PID/PROCESS"
			print_header=0
		}

		# Fill the table
		printf "$format" 			$remaddr $prot "$service_name($localport)" "$pid/$process"
	done
}

! tmp=$(__qs_getparam generate_header) || [ -z "$tmp" ] || CGI_HDGEN=$tmp
[ $CGI_HDGEN != yes ] || __cgi_errhd 200
get_connection_list
