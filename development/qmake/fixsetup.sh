#!/bin/sh

# usage: fixsetup.sh <configured directory> <build directory>

cp "$1"/config.h  "$2"/

#echo "<!DOCTYPE RCC><RCC version='1.0'><qresource>" > $@
#find $(top_srcdir)/lib/images -name '*.png' \
#    | sed -e 's:$(top_srcdir)/lib/\(.*\):<file alias="\1">&</file>:' \
#    >> $@
#echo "</qresource></RCC>" >> $@

