#!/bin/sh

ACLOCAL=aclocal
AUTOHEADER=autoheader
AUTOMAKE="automake -a -c --foreign"
AUTOCONF=autoconf

# Generate the Makefiles and configure files
if ( aclocal --version ) </dev/null > /dev/null 2>&1; then
	echo "Building macros."
	$ACLOCAL ; (cd lib/reLyX; $ACLOCAL )
else
	echo "aclocal not found -- aborting"
	exit
fi

if ( autoheader --version ) </dev/null > /dev/null 2>&1; then
	echo "Building config header template"
	$AUTOHEADER
else
	echo "autoheader not found -- aborting"
	exit
fi

if ( $AUTOMAKE --version ) </dev/null > /dev/null 2>&1; then
	echo "Building Makefile templates"
	$AUTOMAKE ; (cd lib/reLyX ; $AUTOMAKE )
else
	echo "automake not found -- aborting"
	exit
fi

if ( $AUTOCONF --version ) </dev/null > /dev/null 2>&1; then
	echo "Building configure"
	$AUTOCONF ; ( cd lib/reLyX ; $AUTOCONF )
	echo 'run "./configure ; make"'
else
	echo "autoconf not found -- aborting"
	exit
fi

