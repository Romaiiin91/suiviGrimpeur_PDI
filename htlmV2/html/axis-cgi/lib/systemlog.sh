__do_show_logfile () {
	# Arguments
	#	$1=[text|html]		required; presentation, default text
	#	$2=[yes|no]		required; show log file name,
	#				default yes
	#	$3, $4, ...		required; at least one of the log file
	#				names: messages, access

	local cmd__ lf_title__=yes lf__ oldlf__

	[ $# -ge 3 ] || {
		echo "__do_show_logfile: missing argument(s)"
		return 1
	}

	# Parse arguments
	cmd__=cat
	[ "$1" != html ] || cmd__="sed -e 's#<#\&lt;#g' -e 's#>#\&gt;#g'"
	shift

	[ "$1" != no ] || lf_title__=no
	shift

	for lf__ in $*; do
		[ -r $lf__ ] || {
			echo "__do_show_logfile: logfile '$lf__' not found"
			return 1
		}
		oldlf__=$lf__.old

		[ ! -r $oldlf__ ] || {
			[ "$lf_title__" != yes ] || __title $lf_title__ $oldlf__
			eval $cmd__ $oldlf__
		}

		[ "$lf_title__" != yes ] || __title $lf_title__ $lf__
		[ ! -r $lf__ ] || eval $cmd__ $lf__
	done

	return 0
}

__do_show_logfile_for_appname () {
	# Arguments
	#	$1=<appname>
	#	$2=[text|html]		required; presentation, default text
	#	$3=[yes|no]		required; show log file name,
	#				default yes
	#	$4=<logfile>

	local appname__ lf_title__=yes
	local edev__=1

	[ $# -ge 4 ] || {
		echo "__do_show_logfile_for_appname: missing argument(s)"
		return 1
	}

	# Parse arguments
	appname__=$1
	shift

	[ "$2" != no ] || lf_title__=no
	[ "$lf_title__" != yes ] || __title $lf_title__ "Contents of $3 for '$appname__'"

	if [ "$edev__" ] && __is_lua_application $appname__; then
		__do_show_logfile $* | grep "monolith\[[0-9]\+\]:[[:blank:]]\+$appname__:\|[[:blank:]]\+$appname__\[[0-9]\+"
	else
		__do_show_logfile $* | grep "[[:blank:]]\+$appname__\[[0-9]\+"
	fi

	return 0
}
