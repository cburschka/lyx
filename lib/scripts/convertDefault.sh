#! /bin/sh
# The default converter if no other has been defined by the user from the
# Conversion->Converter tab of the Preferences dialog.
#
# The user can also redefine this default converter, placing their
# replacement in ~/.lyx/scripts
#
# converts an image from $1 to $2 format

convert -depth 8 $1 $2 || {
	echo "$0 ERROR"
	echo "Execution of \"convert\" failed."
	exit 1
}

# It appears that convert succeeded, but we know better than to trust it ;-)
# convert is passed strings in the form "FMT:FILENAME", so use the ':' to
# delimit the two parts.
# Note that Win32 filenames have the form 'C:\my\file',
# so use everything from the first ':' to the end of the line.
FILE=`echo $2 | cut -d ':' -f 2-`

test -f $FILE || {
	echo "$0 ERROR"
	echo "Unable to find file \"${FILE}\""
	exit 1
}

echo "$0 generated file \"${FILE}\" successfully."
