#!/usr/bin/env bash

# file header_check.sh
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Scott Kostyshak

# Full author contact details are available in file CREDITS

# Description:

# All .cpp and .h files in the current directory and subdirectories
# are checked to see which include statements could be omitted without
# causing a build error. Many of these omissions would not be desired.
# For example, currently if you don't include Undo.h in Undo.cpp, there
# is no error because Undo.h is included in Cursor.h which is included
# in Undo.cpp. But clearly we do want to include Undo.h in Undo.cpp.

# The results are stored in header_check.sh.log

set -u

LOG_FILE="$(basename $0).log"

# For only standard headers:
  PATTERN='^#include <'
# For all headers:
# PATTERN='^#include'

# Exclude common headers with regex
# (e.g. 'debug.h' will exclude 'support/debug.h')
EXCLUDE='\(debug.h\|cstdio\)'

function BUILD_FN ()
{
	# This is not a clean make.
	make -j$(grep "CPU" /proc/cpuinfo | wc -l)
}

echo "BUILD_FN exited without error after removing
the following include statements invididually:" > "${LOG_FILE}" \
|| { echo "ERROR: could not create log file, ${LOG_FILE}"; exit 1; }

find -regex ".*\(cpp\|h\)$" | \
while read FILE_
do
	FILE_COPY=$( tempfile )
	cp "${FILE_}" "${FILE_COPY}" \
		|| { echo "ERROR: bu copy failed" >&2; exit 1; }
	echo "processing ${FILE_}..."
	grep "${PATTERN}" "${FILE_}" | \
	while read INCLUDE
	do
		if echo "${INCLUDE}" | grep -q -v "${EXCLUDE}"; then
			cp "${FILE_COPY}" "${FILE_}" \
				|| { echo "ERROR: restore copy failed" >&2; exit 1; }
			sed -i "s@${INCLUDE}@@" "${FILE_}"
			( BUILD_FN ) &>/dev/null && echo "${FILE_}::${INCLUDE}" >> "${LOG_FILE}"
		fi
	done
	cp "${FILE_COPY}" "${FILE_}"
done
