#! /bin/sh

# file fig_copy.sh
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author Angus Leeming
#
# Full author contact details are available in file CREDITS

# Usage:
# fig_copy.sh <from file> <to file>

# This script will copy an XFIG .fig file "$1" to "$2". In the process,
# it will modify the contents of the .fig file so that the names of any
# picture files that are stored as relative paths are replaced
# with the absolute path.

test $# -eq 2 || {
    echo "Usage: fig_copy.sh <from file> <to file>" >&2
    exit 1
}


test -r "$1" || {
    echo "Unable to read $1" >&2
    exit 1
}


# The work is trivial if "to" and "from" are in the same directory.
PRESENT_DIR=$PWD

cd `dirname "$1"` || exit $?
FROM_DIR=$PWD
cd "$PRESENT_DIR" || exit $?

cd `dirname "$2"` || exit $?
TO_DIR=$PWD
cd "$PRESENT_DIR" || exit $?

test "$FROM_DIR" = "$TO_DIR" && {
    'cp' -f "$1" "$2"
    exit $?
}


# Ok, they're in different directories. The .fig file must be modified.

# WS is a space and a tab character.
WS=' 	'

TRANSFORMATION="
# We're looking for a line of text that defines an entry of
# type '2' (a polyline), subtype '5' (an external picture file).
# The line has 14 other data fields.
/^[${WS}]*2[${WS}]\{1,\}5\([${WS}]\{1,\}[^${WS}]\{1,\}\)\{14\}/{

:loop
# If we're not on the last line, get the next line.
# It's this that defines the file itself.
$!{
N

# Does the new line contain any data?
# If not, loop
/\n[${WS}]*$/bloop

# Does the new line contain only a comment?
# If so, loop
/\n[${WS}]*#[^\n]*$/bloop

# The contents of the final line containing the file name
# are ' X <file name>', where X = 0 or 1.
# If the file name does not begin with '/', then insert the absolute path.
# Note that this test will work even if the file name contains spaces.
s@\(.*\n[${WS}]*[01][${WS}]\{1,\}\)\([^/]\)@\1${FROM_DIR}/\2@
}
}"

sed "${TRANSFORMATION}" "$1" > "$2"
exit $?
