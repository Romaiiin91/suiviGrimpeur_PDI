#! /bin/sh

# This script is used to verify that user does not try to filter its
# own ip address. The script should output nothing to stdout.
#
# The script should exit with exit status 0 if filtering is disabled
# or if configuration does _not_ filter out REMOTE_ADDR.
#
# The script should exit with exit status 1 if filtering is enabled
# and config tries to filter out REMOTE_ADDR.

phc="parhandclient --nocgi"

set -e

if [ $# -eq 0 ]; then
	# Verify current configuration.
	policy=$($phc get Network.Filter.Input.Policy - RAW)
	addrs=$($phc get Network.Filter.Input.AcceptAddresses - RAW)
	enabled=$($phc get Network.Filter.Enabled - RAW)
else
	# Verify parameters on command line
	policy=$1
	shift
	enabled=$1
	shift
	addrs="$*"
fi
[ "$enabled" = yes ] || exit 0

if [ $policy = allow ]; then
	# this means, addresses in the list are ACCEPTED, the rest are DROPPED.
	# so r-addr should be on the list
	! cidrmatch "$REMOTE_ADDR" $addrs 127.0.0.0/8 ::1 || exit 0
elif [ $policy = deny ]; then
	# this means, addresses in the list are DROPPED, the rest are ACCEPTED.
	# so r-addr should NOT be on the list
	cidrmatch "$REMOTE_ADDR" $addrs || exit 0
fi

exit 1
