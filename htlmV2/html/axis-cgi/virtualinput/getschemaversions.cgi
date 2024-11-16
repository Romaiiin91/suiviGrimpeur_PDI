#!/bin/sh -e

. ./virtual_input_common.sh

get_schema_success() {
	local xml_resp xml_response_tag=VirtualInputResponse

	xml_resp="$XML_HEADER
<$xml_response_tag $XML_WOWO>
	<Success>
		<GetSchemaVersionsSuccess>
			<SchemaVersion>
				<VersionNumber>
					$XML_SCHEMA_MAJOR_VERSION.$XML_SCHEMA_MINOR_VERSION
				</VersionNumber>
				<Depricated/>
			</SchemaVersion>
		</GetSchemaVersionsSuccess>
	</Success>
</$xml_response_tag>"

	__cgi_content 200 OK text/xml $xml_resp
	exit 0
}

get_schema_success
