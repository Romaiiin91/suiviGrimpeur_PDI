#!/bin/sh -e


! touch /dev/console 2>/dev/null || exec 2>/dev/console



debug=${1:-"debug"}

DBG_TEXT="Debug information factorydefaultsurvive.cgi\n\n"

debug_print() {
	[ "$debug" != debug ] || __cgi_errhd 200 $DBG_TEXT
}

debug_exit() {
	[ "$debug" != debug ] || __cgi_errhd 500 $DBG_TEXT
	exit 1
}


. /usr/html/axis-cgi/lib/functions.sh || {
	[ "$debug" != debug ] || echo "Failed to source functions.sh" >&2
	exit 1
}

bck_conf=/etc/bck/fd_survive.conf
. $bck_conf || {
	DBG_TEXT="${DBG_TEXT}Error while sourcing $bck_conf.\n"
	debug_exit
}

BCK_PARAMETERS_VAR=
BCK_FILES_VAR=
BCK_GROUPS_VAR=

[ ! -r $BCK_PARAMETERS_FILE ] || BCK_PARAMETERS_VAR=$(cat $BCK_PARAMETERS_FILE)
[ ! -r $BCK_FILES_FILE ] || BCK_FILES_VAR=$(cat $BCK_FILES_FILE)
[ ! -r $BCK_GROUPS_FILE ] || BCK_GROUPS_VAR=$(cat $BCK_GROUPS_FILE)

bck_parameters_file_size=${#BCK_PARAMETERS_VAR}
bck_files_file_size=${#BCK_FILES_VAR}
bck_groups_file_size=${#BCK_GROUPS_VAR}


no_valid_parameters() {
	if ! ([ "$parameters" ] || [ "$groups" ] || [ "$files" ]); then
		DBG_TEXT="${DBG_TEXT}Error: missing arguments for no_valid_parameters().\n"
		return 1
	fi

	return 0
}

modify_list() {
	[ $# -gt 2 ] || {
		DBG_TEXT="${DBG_TEXT}Error: missing arguments in modify_list().\n"
		return 1
	}

	local IFS entry arglist parhandreq pattern var list action=$2

	case $1 in
		files)
			var=BCK_FILES_VAR
			;;
		groups)
			var=BCK_GROUPS_VAR
			parhandreq=getgroup
			;;
		parameters)
			var=BCK_PARAMETERS_VAR
			parhandreq=get
			;;
		*)
			DBG_TEXT="${DBG_TEXT}Error: invalid arguments in modify_list().\n"
			return 1
			;;
	esac

	IFS="${IFS},"
	shift 2

	if [ $var = BCK_FILES_VAR ]; then
		arglist=$@
	else
		arglist=$(echo $@ | tr 'A-Z' 'a-z')
	fi

	eval list=\"\$$var \"

	for entry in $arglist; do
		if [ $var != BCK_FILES_VAR ]; then
			parhandclient --nocgi $parhandreq $entry \
				>/dev/null 2>&1 || continue
			entry=${entry#root.}
		fi

		case "$action" in
			add)
				pattern=${list##*[[:space:]]$entry[[:space:]]}
				[ ${#pattern} -ne ${#list} ] ||
					list="$list$entry "
				DBG_TEXT="${DBG_TEXT}add OK\n"
				;;
			remove)
				pattern="${list%%[[:space:]]$entry*} ${list##*$entry[[:space:]]}"
				if [ ${#list} -gt ${#pattern} ]; then
					list=$pattern
					DBG_TEXT="${DBG_TEXT}remove OK\n"
				else
					DBG_TEXT="${DBG_TEXT} # Error: resource not found\n"
				fi
				;;
			*)
				DBG_TEXT="${DBG_TEXT}Error: invalid action in modify_list.\n"
				return 1
				;;
		esac
	done

	eval $var=\${list% }

	return 0
}

list_files() {
	local entry

	DBG_TEXT="${DBG_TEXT}\nParameters to be saved during a factory default\n\n"
	for entry in $BCK_FILES_VAR; do
		DBG_TEXT="${DBG_TEXT}file:${entry}\n"
	done
	for entry in $BCK_GROUPS_VAR; do
		DBG_TEXT="${DBG_TEXT}group:${entry}\n"
	done
	for entry in $BCK_PARAMETERS_VAR; do
		DBG_TEXT="${DBG_TEXT}param:${entry}\n"
	done
}

reset_list() {
	BCK_PARAMETERS_VAR=
	BCK_FILES_VAR=
	BCK_GROUPS_VAR=
	DBG_TEXT="${DBG_TEXT}Remove all OK\n"
}

dump2file() {
	[ $# -eq 2 ] || {
		DBG_TEXT="${DBG_TEXT}dump2file: missing argument(s)\n"
		return 1
	}

	echo "$1" | tr ' ' '\n' | sort > $2 || {
		DBG_TEXT="${DBG_TEXT}dump2file: file $2 creation failed\n"
		return 1
	}

	chgrp admin $2 >/dev/null 2>&1
	chmod g+w $2 >/dev/null 2>&1
}

parameters=$(__qs_getparam param) || :
groups=$(__qs_getparam group) || :
files=$(__qs_getparam file) || :

if form_val=$(__qs_getparam action) && [ "$form_val" ]; then
	case "$form_val" in
		add)
			no_valid_parameters || debug_exit

			[ -z "$parameters" ] ||
				modify_list parameters add $parameters ||
				debug_exit

			[ -z "$groups" ] ||
				modify_list groups add $groups || debug_exit

			[ -z "$files" ] ||
				modify_list files add $files || debug_exit
			;;
		remove)
			no_valid_parameters || debug_exit

			[ -z "$parameters" ] ||
				modify_list parameters remove $parameters ||
				debug_exit

			[ -z "$groups" ] ||
				modify_list groups remove $groups || debug_exit

			[ -z "$files" ] ||
				modify_list files remove $files || debug_exit
			;;
		list)
			list_files
			;;
		removeall)
			reset_list
			;;
		*)
			DBG_TEXT="${DBG_TEXT}invalid action in main.\n"
			debug_exit
			;;
	esac
else
	DBG_TEXT="${DBG_TEXT}parameter action not found.\n"
	debug_exit
fi

[ ${#BCK_PARAMETERS_VAR} -eq $bck_parameters_file_size ] ||
	dump2file "$BCK_PARAMETERS_VAR" $BCK_PARAMETERS_FILE || debug_exit

[ ${#BCK_GROUPS_VAR} -eq $bck_groups_file_size ] ||
	dump2file "$BCK_GROUPS_VAR" $BCK_GROUPS_FILE || debug_exit

[ ${#BCK_FILES_VAR} -eq $bck_files_file_size ] ||
	dump2file "$BCK_FILES_VAR" $BCK_FILES_FILE || debug_exit

debug_print

exit 0
