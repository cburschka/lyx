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
# LyX was compiled on exotic environments and these sometimes
# require headers not needed on win/linux. So check the logs before
# deleting "redundant" standard libraries or includes around various
# ifdefs...
EXCLUDE='\(debug.h\|cstdio\)'

NCORES=$(grep "CPU" /proc/cpuinfo | wc -l)

function BUILD_FN ()
{
	PREFIX=''

	# This is not a clean make.
	IFS='' ERROR_OUTPUT=$(make -j${NCORES} 2>&1 >/dev/null)
	ERROR_CODE=$?

	# Without the grep, ERROR_OUTPUT might contain messages such as:
	# 2885 translated messages, 2169 fuzzy translations, 1356 untranslated messages.
	ERROR_OUTPUT=$(echo "${ERROR_OUTPUT}" | grep -i "error")

	# The sed regex is more strict than it needs to be.
	if (( ERROR_CODE != 0 )); then
		cppORh=$(echo "${ERROR_OUTPUT}" | head -n 1 | \
			sed 's/.*\.\(cpp\|h\):[0-9]\+:[0-9]\+: error: .*/\1/')
		if [ "${cppORh}" = "cpp" ]; then
			PREFIX='suspicious: '
		elif [ "${cppORh}" != "h" ]; then
			echo -e "Warning: the error was not parsed correctly."\
				"\nThe following string was expected to be"\
				"'.cpp' or '.h': \n ${cppORh}" >&2
		fi
	fi
	return "${ERROR_CODE}"
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

			BUILD_FN
			BUILD_FN_RET=$?
			if [ "${BUILD_FN_RET}" = 0 ]; then
				echo "${FILE_}::${INCLUDE}" >> "${LOG_FILE}"
			elif [ -n "${PREFIX}" ]; then
				echo "${PREFIX}${FILE_}::${INCLUDE}" >> "${LOG_FILE}"
			fi
		fi
	done
	cp "${FILE_COPY}" "${FILE_}"
done
