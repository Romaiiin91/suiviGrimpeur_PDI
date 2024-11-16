#!/bin/sh -e




. /usr/html/axis-cgi/lib/functions.sh

action=$(__qs_getparam action || :)

plcfgtab_out() {
	local tzv="$1" ltv="$2" cv="$3"

	echo "	  <OPTION VALUE=\"$tzv\" <!--#if expr=\"\$root_Time_POSIXTimeZone = $tzv\" --> SELECTED<!--#set var=\"selected_tz\" val=\"$tzv\" --><!--#endif -->>$cv</OPTION>"
}

datetab_out() {
	local tzv="$1" ltv="$2" cv="$3"

	if [ "$ltv" ]; then
		echo "    <OPTION VALUE=\"$tzv\" <!--#if expr=\"\$root_Time_POSIXTimeZone = $tzv\" --> SELECTED<!--#set var=\"selected_tz\" val=\"$tzv\" --><!--#endif -->><!--#language id=\"$ltv\" text=\"$cv\" --></OPTION>"
	else
		echo "    <OPTION VALUE=\"$tzv\" <!--#if expr=\"\$root_Time_POSIXTimeZone = $tzv\" --> SELECTED<!--#set var=\"selected_tz\" val=\"$tzv\" --><!--#endif -->>$cv</OPTION>"
	fi
}

case $action in
	int_plcfg_table)
		prod_out=plcfgtab_out
		;;
	int_table)
		prod_out=datetab_out
		;;
	*)
		exit 1
		;;
esac

echo "<!--#set var=\"selected_tz\" val=\"User_configured\" -->"

old_ifs=$IFS
IFS=\;
while read -r TZ_VAL ZN_VAL LT_VAL COM_VAL; do
	[ "${TZ_VAL%%[#]*}" ] || continue

	[ "$TZ_VAL" ] || continue
	[ "$ZN_VAL" ] || continue
	[ "$COM_VAL" ] || continue

	$prod_out "$TZ_VAL" "$LT_VAL" "$COM_VAL"
done </usr/share/timezone/zones.conf
IFS=$old_ifs


case $action in
	int_plcfg_table)
		echo "<!--#if expr=\"\$selected_tz = User_configured\" -->"
		echo "	  <OPTION VALUE=\"<!--#echo var=\"root_Time_POSIXTimeZone\" option=\"encoding:html\" -->\" SELECTED><!--#echo var=\"root_Time_POSIXTimeZone\" option=\"encoding:html\" --> (Manually configured)</OPTION>"
		echo " <!--#endif -->"
		;;
	int_table)
		echo "<!--#if expr=\"\$selected_tz = User_configured\" -->"
		echo "    <OPTION VALUE=\"<!--#echo var=\"root_Time_POSIXTimeZone\" option=\"encoding:html\" -->\" SELECTED><!--#echo var=\"root_Time_POSIXTimeZone\" option=\"encoding:html\" --> <!--#language id=\"dst_tz_userval\" text=\"(Manually configured)\" --></OPTION>"
		echo " <!--#endif -->"
		;;
esac

exit 0
