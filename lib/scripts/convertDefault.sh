#! /bin/sh
# The default converter if no other has been defined by the user from the
# Conversion->Converter tab of the Preferences dialog.
#
# The user can also redefine this default converter, placing their
# replacement in ~/.lyx/scripts
#
# converts an image from $1 to $2 format
convert -depth 8 $1 $2
if [ $? -ne 0 ]; then
    exit $?
fi

# It appears that convert succeeded, but we know better than to trust it ;-)
# convert is passed strings in the form "FMT:FILENAME", so use the ':' to
# delimit the two parts.
FILE=`echo $2 | cut -d ':' -f 2`

# FSTATUS == 0 is the file exists and == 1 if it does not.
FSTATUS=0
test -f $FILE || FSTATUS=1

exit $FSTATUS
