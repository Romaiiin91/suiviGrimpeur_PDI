__do_uptime () {
	local llow__=50 lmed__=80 uptm__ llm__

	uptm__=$(expr "$(uptime)" : '.*up[[:space:]]\+\(.*,[[:space:]]\+load average:.*\)$')

	case "$1" in
		''|long)
			# This is the default.
			;;
		short)
			uptm__=$(expr "$uptm__" : '\(.*\),[[:space:]]\+load average:.*$')
			;;
		load)
			uptm__=$(expr "$uptm__" : '.*,[[:space:]]\+load average:[[:space:]]\+\(.*\)$')
			;;
		vload)
			# This is the load during the latest minute.
			uptm__=$(expr "$uptm__" : '.*,[[:space:]]\+load average:[[:space:]]\+\([^,]\+\),.*$')
			llm__="${uptm__%.*}${uptm__#*.}" # remove the dot
			if [ $llm__ -le $llow__ ]; then
				uptm__="Low"
			elif [ $llm__ -gt $llow__ ] && [ $llm__ -lt $lmed__ ]; then
				uptm__="Medium"
			else
				uptm__="High"
			fi
			;;
	esac

	echo -n "$uptm__"
	return 0
}
