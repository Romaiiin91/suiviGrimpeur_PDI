#!/bin/sh

if [ x$1 != xnoheader ]; then
	echo -e "Cache-Control: no-cache\r"
	echo -e "Pragma: no-cache\r"
	echo -e "Expires: Thu, 01 Dec 1994 16:00:00 GMT\r"
	echo -e "Content-Type: text/plain\r"
	echo -e "\r"
fi

echo

cat /usr/share/axis-release/variables

echo -e "\r\n"

echo "---- Parameter list ----"

sec=$(echo "$QUERY_STRING" | sed 's/^.*\(sections\).*$/\1/')

if [ "$sec" = sections ]; then
	parhandclient getgroup root - NAMEVALUESECTIONS |
		sed -e 's/\([[:blank:]]*passwd[[:blank:]]*=[[:blank:]]*\[\)[[:print:]]*\([[:blank:]]*\]\)/\1 "******" \2/g;s/\(^.*[Pp][Aa][Ss][Ss][Ww][Oo][Rr][Dd][[:blank:]]*=\).*$/\1 "*****"/g;s/\(^.*[Pp][Aa][Ss][Ss][[:blank:]]*=\).*$/\1 "*****"/g;s/\(^Passphrase[[:blank:]]*=\).*$/\1 "*****"/g;s/\(^Key[1234 ][[:blank:]]*=\).*$/\1 "*****"/g'
else
	parhandclient getgroup root - NAMEVALUE |
		sed -e 's/\(^.*[Pp][Aa][Ss][Ss][Ww][Oo][Rr][Dd]=\).*$/\1"*****"/g;s/\(^.*[Pp][Aa][Ss][Ss]=\).*$/\1"*****"/g;s/\(^.*Passphrase=\).*$/\1"*****"/g;s/\(^.*Wireless.*\.Key[1234]*=\).*$/\1"*****"/g'
fi
