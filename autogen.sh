#!/bin/sh

# Discover what version of autoconf we are using.
autoversion=`autoconf --version | head -n 1`

case $autoversion in
    *2.13)
	echo "2.13"
	cp config/acconfig.h .
	cp config/configure.in .
	cp config/relyx_configure.in lib/reLyX/configure.in
	;;
    *2.53)
	echo "2.53"
	cp config/configure.ac .
	cp config/relyx_configure.ac lib/reLyX/configure.ac
	;;
    *)
	echo "You are running a version of autoconf that"
	echo "we do not support."
	echo "LyX only supports autoconf 2.13 and 2.53."
	exit
	;;
esac


ACLOCAL=aclocal
AUTOHEADER="autoheader -W none -W obsolete"
AUTOMAKE="automake -a -c --foreign"
AUTOCONF="autoconf -W none -W obsolete"
GNUM4=

ACINCLUDE_FILES="lyxinclude.m4 libtool.m4 codeset.m4 gettext.m4 glibc21.m4 iconv.m4 isc-posix.m4 lcmessage.m4 progtest.m4 xforms.m4 qt2.m4 gtk--.m4 gnome--.m4 gnome.m4 pspell.m4 pkg.m4"
SIGCPP_ACINCLUDE_FILES="libtool.m4"

echo -n "Locating GNU m4... "
for prog in $M4 gm4 gnum4 m4 ; do
  case `$prog --version 2>&1` in
    *GNU*) ok=yes
	   GNUM4=$prog
	   echo "found: $GNUM4"
	   break ;;
    *) ;;
  esac
done
if test x$ok = xno ; then
    echo "not found."
fi

# Generate acinclude.m4
echo -n "Generate acinclude.m4... "
rm -f acinclude.m4
(cd config ; cat ${ACINCLUDE_FILES} >../acinclude.m4)
echo "done."

# Generate the Makefiles and configure files
if ( $ACLOCAL --version ) </dev/null > /dev/null 2>&1; then
	echo "Building macros..."
	for dir in . lib/reLyX ; do
	    echo "        $dir"
	    ( cd $dir ; $ACLOCAL )
	done
	echo "done."
else
	echo "aclocal not found -- aborting"
	exit
fi

if ( $AUTOHEADER --version ) </dev/null > /dev/null 2>&1; then
	echo "Building config header template..."
	for dir in . ; do
	    echo "        $dir"
	    ( cd $dir ; $AUTOHEADER )
	done
	echo "done."
else
	echo "autoheader not found -- aborting"
	exit
fi

if ( $AUTOMAKE --version ) </dev/null > /dev/null 2>&1; then
	echo "Building Makefile templates..."
	for dir in . lib/reLyX ; do
	    echo "        $dir"
	    ( cd $dir ; $AUTOMAKE )
	done
	echo "done."
else
	echo "automake not found -- aborting"
	exit
fi

if ( $AUTOCONF --version ) </dev/null > /dev/null 2>&1; then
	echo "Building configure..."
	for dir in . lib/reLyX ; do
	    echo "       $dir"
	    ( cd $dir ; $AUTOCONF )
	done
	echo "done."
else
	echo "autoconf not found -- aborting"
	exit
fi

# Autogenerate lib/configure.m4.
if test x$GNUM4 != x ; then
    echo -n "Building lib/configure ... "
    rm -f lib/configure
    $GNUM4 lib/configure.m4 > lib/configure
    chmod a+x lib/configure
    echo "done."
else
    echo "No GNU m4. Impossible to build lib/configure -- aborting"
    exit
fi

echo
echo 'run "./configure ; make"'
echo
