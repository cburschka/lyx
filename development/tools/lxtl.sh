#!/bin/sh
#
# Author:  Allan Rae <rae@lyx.org>
#
# Copy xtl files ready for internal use.

usage () {
	echo "usage:  lxtl.sh <version-number>">&2
	echo "        <version-number> is required and should be of the form" >&2
	echo "        'XTL-x.y.pl.z' where x, y and z are digits." >&2
	echo "" >&2
	echo "Run this from the root of the xtl source tree." >&2
	echo "Everything LyX needs will be moved into lxtl/" >&2
	echo "you then 'cp lxtl/* /path-to-lyx/src/xtl/'" >&2
	exit 0
}


if [ ! -d ./include/xtl ] ; then
	usage
fi

if [ -d ./lxtl ]; then
	echo "Uh oh!  There's a directory called lxtl here already." >&2
        echo "I'm going to delete it unless you press <Ctrl><C> right now!" >&2
        read aline
        rm -rf lxtl
fi

if [ $# -lt 1 ]; then
	usage
fi

mkdir lxtl

cp COPYING.LIB lxtl
cat > lxtl/README <<EOF
$1
This directory contains the subset of XTL required by LyX.
The full distribution includes extensive tests and documentation.
If you encounter compiler errors in xtl code when compiling LyX please
get a copy of the full distribution and run the regression tests on your
system. Report any problems found to the xtl mailing list.

See:  http://gsd.di.uminho.pt/jop/xtl/

-------------------------===========================----------------------

EOF

cat README >> lxtl/README

cat > lxtl/Makefile.am <<\EOF
AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core 
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
EXTRA_DIST = README COPYING.LIB
ETAGS_ARGS = --lang=c++
SOURCES = \
	autobuf.h \
	autoio.h \
	config.h \
	giop.h \
	graphio.h \
	macros.h \
	objio.h \
	text.h \
	vobjio.h \
	xdr.h
EOF

cat > lxtl/.cvsignore <<EOF
Makefile
Makefile.in
EOF

# strip the Id info otherwise it gets replaced with the CVS version instead
# making it even more confusing.
#
(
    cd include/xtl;
    for hfile in `ls`; do
            sed < $hfile > ../../lxtl/$hfile \
                -e '/Id:/s/\$//g'
    done
)

