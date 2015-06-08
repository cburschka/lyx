#!/bin/sh

ACLOCAL="aclocal -I m4 -I config"
AUTOHEADER="autoheader"
AUTOMAKE="automake --add-missing --force-missing --copy --foreign"
AUTOCONF="autoconf"

# Discover what version of automake we are using.
automake_version=`$AUTOMAKE --version 2>/dev/null | head -n 1`

test "$automake_version" != "" && {
    echo "Using $automake_version"
} || {
    echo "LyX requires automake >= 1.8"
    exit 1
}

case $automake_version in
    *' '1.[8-9]*|*' '1.1[012345]*)
	;;
    *)
	echo "This automake version is not supported by LyX."
	echo "LyX only supports automake 1.8 to 1.14."
	exit 1
	;;
esac

# Discover what version of autoconf we are using.
autoversion=`$AUTOCONF --version 2>/dev/null | head -n 1`

test "$autoversion" != "" && {
    echo "Using $autoversion"
} || {
    echo "LyX requires autoconf >= 2.60"
    exit 1
}

case $autoversion in
    *' '2.60[ab]|*' '2.6[0-9])
	;;
    *)
	echo "This autoconf version is not supported by LyX."
	echo "LyX only supports autoconf 2.60-2.69."
	exit 1
	;;
esac

# Delete old cache directories.
# automake will stop if their contents was created by an earlier version.
rm -rf autom4te.cache

# Generate the Makefiles and configure files
echo "Building macros..."
if ( $ACLOCAL --version ) < /dev/null > /dev/null 2>&1; then
	$ACLOCAL
else
	echo "aclocal not found -- aborting"
	exit 1
fi

echo "Building config header template..."
if ( $AUTOHEADER --version ) < /dev/null > /dev/null 2>&1; then
	$AUTOHEADER
else
	echo "autoheader not found -- aborting"
	exit 1
fi

echo "Building Makefile templates..."
if ( $AUTOMAKE --version ) < /dev/null > /dev/null 2>&1; then
	$AUTOMAKE
else
	echo "automake not found -- aborting"
	exit 1
fi

echo "Building configure..."
if ( $AUTOCONF --version ) < /dev/null > /dev/null 2>&1; then
	$AUTOCONF
else
	echo "autoconf not found -- aborting"
	exit 1
fi

echo "Building po/POTFILES.in..."
if ( make -s -f po/Rules-lyx srcdir=po top_srcdir=. po/POTFILES.in ); then
    :
else
	echo "Building po/POTFILES.in failed -- aborting"
	exit 1
fi

echo
echo 'run "./configure && make"'
echo
