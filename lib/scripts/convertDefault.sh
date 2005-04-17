#! /bin/sh

# file convertDefault.sh
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Herbert Voß

# Full author contact details are available in file CREDITS.

# The default converter if no other has been defined by the user from the
# Conversion->Converter tab of the Preferences dialog.

# The user can also redefine this default converter, placing their
# replacement in ~/.lyx/scripts

# converts an image from $1 to $2 format
convert -depth 8 "$1" "$2" || {
	echo "$0 ERROR" >&2
	echo "Execution of \"convert\" failed." >&2
	exit 1
}
