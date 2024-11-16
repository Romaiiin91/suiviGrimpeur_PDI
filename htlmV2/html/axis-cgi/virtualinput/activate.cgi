#!/bin/sh -e

. ./virtual_input_common.sh

PORT=
SCHEMA_VERSION=

get_schemaversion SCHEMA_VERSION

get_port PORT

dbus_call $PORT $DBUS_MTD_ACTIVATE
