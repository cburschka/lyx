#!/bin/sh
# this is the default converter if no one other was
# defined by the user in edit->preferences->converter
#
# the user can also redefine this default converter
# with an own shell script in ~/.lyx/scripts
#
# converts an image from $1 to $2 format 
convert -depth 8 $1 $2 
exit 0
