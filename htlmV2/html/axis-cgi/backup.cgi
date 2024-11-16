#!/bin/sh
#
# Send a backup of the JFFS filesystem either by HTTP or FTP.
# The backup contains the serial number of the unit and is unit specific.
# The backup contains a file which lists all files in /etc.
# HTTP: requires REQUEST_METHOD=GET
# FTP: requires FTP_METHOD=GET from the virtual target execution.
#

#
# Switch to runlevel 3 on exit.
#
trap "init 3" EXIT

#
# Trap sigpipe (13), letting the above defined trap switch to runlevel 3.
# Sigpipe will be received whenever the user cancels a backup.
#
trap "exit 0" 13

#
# Set PATH
#
PATH=/bin:/sbin:/usr/bin:/usr/sbin

#
# Test return values
#
ret=0

#
# Error messages
#
errmsg=""

#
# Send error message
#
panic()
{
    if test ! -z ${REQUEST_METHOD}
    then
	echo -e "Content-Type: text/html\r"
	echo -e "\r"
	echo -e "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\r"
	echo -e "<html><head><title>Fatal error</title></head><body>\r"
    fi

    echo -e ${errmsg} ${ret} "\r"

    if test ! -z ${REQUEST_METHOD}
    then
	echo -e "</body></html>\r"
    fi
}

#
# Call only by HTTP or FTP
#
if test -z ${REQUEST_METHOD} && test -z ${FTP_METHOD}
then
    errmsg="Environment variables REQUEST_METHOD or FTP_METHOD must be: "
    ret="GET"
    panic
    exit 1
fi

#
# HTTP GET required to obtain a backup file
#
if test ! -z ${REQUEST_METHOD} && test ${REQUEST_METHOD} != "GET"
then
    errmsg="Invalid request method for application/x-tar: "
    ret=${REQUEST_METHOD}
    panic
    exit 1
fi

#
# FTP GET required to obtain a backup file
#
if test ! -z ${FTP_METHOD} && test ${FTP_METHOD} != "GET"
then
    errmsg="Invalid request method for application/x-tar: "
    ret=${FTP_METHOD}
    panic
    exit 1
fi

#
# Init single user mode
#
init 4 || ret=$?
if [ ${ret} != 0 ]
then
    errmsg="init 4 failed: "
    panic
    exit ${ret}
fi
sleep 3

#
# Create a tar tf compatible list of all files in /etc
#
find /etc/* -type f 2> /dev/null > /tmp/ETC.origx || ret=$?
if [ ${ret} != 0 ]
then
    errmsg="find failed: "
    panic
    exit ${ret}
fi

sed -e 's/^\/\(.*\)/\1/' 2> /dev/null /tmp/ETC.origx > /tmp/ETC.orig || ret=$?
if [ ${ret} != 0 ]
then
    errmsg="sed failed: "
    panic
    exit ${ret}
fi

#
# Work in /tmp
#
cd /tmp || ret=$?
if [ ${ret} != 0 ]
then
    errmsg="cd failed: "
    panic
    exit ${ret}
fi

#
# Identifies the backup as belonging to this device by the serial number
# stored in the boot block sector, _note_ -nocgi _prevents_ the output
# of Content-Type: text/plain...argh!
#
bootblocktool -nocgi -x SERNO > SERNO.orig || ret=$?
if [ ${ret} != 0 ]
then
    errmsg="bootblocktool failed: "
    panic
    exit ${ret}
fi

#
# Check if language support is present and save the current language link.
#
language_linkfile=/etc/language/langfile.dat

if test -f ${language_linkfile}
then
    language_file="LANGUAGE.orig"
    ls -l ${language_linkfile} | sed -e 's/.*-> \(.*\)/\1/' > ${language_file}
else
    language_file=""
fi

#
# This is the suggested filename the client stores the file by
#
filename="backup_"`cat SERNO.orig`".tar"

#
# Send the backup to the client
#
if test ! -z ${REQUEST_METHOD}
then
    echo -e "Content-Type: application/x-tar\r"
    echo -e "Content-Disposition: attachment; filename="${filename}
    echo -e "\r"
fi

if tar cf - SERNO.orig ETC.orig ${language_file} /etc/*
then
    true
else
    errmsg="tar failed: "
    panic
    exit ${ret}
fi

#
# Remove the created files
#
rm -f SERNO.orig ETC.origx ETC.orig ${language_file} || ret=$?
if [ ${ret} != 0 ]
then
    echo -e "rm failed: " ${ret} "\r" > /dev/stderr
fi

exit 0
