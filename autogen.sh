#!/bin/sh

ACLOCAL="aclocal -I ${PWD}/m4"
AUTOHEADER="autoheader"
AUTOMAKE="automake -a -c --foreign"
AUTOCONF="autoconf"
ACINCLUDE_FILES="lyxinclude.m4 libtool.m4 xforms.m4 qt.m4 gtk--.m4 gnome--.m4 gnome.m4 aspell.m4 pspell.m4 cygwin.m4 pkg.m4"

# Discover what version of gettext we are using.
gettext_version=`gettext --version 2>/dev/null | head -n 1`

test "$gettext_version" != "" && {
    echo "Using $gettext_version"
} || {
    echo "LyX requires getttext >= 0.12"
    exit
}

case $gettext_version in
    *' '0.1[2-4]*)
	;;
    *)
	echo "This gettext version is not supported by LyX."
	echo "LyX supports only gettext 0.1[2-4]."
	exit
	;;
esac

# Discover what version of automake we are using.
automake_version=`$AUTOMAKE --version 2>/dev/null | head -n 1`

test "$automake_version" != "" && {
    echo "Using $automake_version"
} || {
    echo "LyX requires automake >= 1.5"
    exit
}

case $automake_version in
    *' '1.[5-7]*)
	;;
    *)
    
	echo "This automake version is not supported by LyX."
        echo "LyX only supports automake 1.[5-7]."
        exit
        ;;
esac
							
# Discover what version of autoconf we are using.
autoversion=`$AUTOCONF --version 2>/dev/null | head -n 1`

test "$autoversion" != "" && {
    echo "Using $autoversion"
} || {
    echo "LyX requires autoconf >= 2.52"
    exit
}
	    
case $autoversion in
    *' '2.5[2-9])
	EXTRA_ACINCLUDE_FILES="lyxinclude25x.m4"
	;;
    *)
	echo "This autoconf version is not supported by LyX."
	echo "LyX only supports autoconf 2.5[2-9]."
	exit
	;;
esac

echo -n "Locating GNU m4... "
GNUM4=
for prog in $M4 gm4 gnum4 m4; do
	# continue if $prog generates error (e.g. does not exist)
	( $prog --version ) < /dev/null > /dev/null 2>&1
	if test $? -ne 0 ; then continue; fi

	# /dev/null input prevents a hang of the script for some m4 compilers (e.g. on FreeBSD)
	case `$prog --version < /dev/null 2>&1` in
	*GNU*)	GNUM4=$prog
		break ;;
	esac
done
if test x$GNUM4 = x ; then
	echo "not found."
	exit
else
	echo `which $GNUM4`
fi

# Generate acinclude.m4
echo -n "Generate acinclude.m4... "
rm -f acinclude.m4
(cd config ; cat ${ACINCLUDE_FILES} ${EXTRA_ACINCLUDE_FILES} >../acinclude.m4)
echo "done."

# Generate the Makefiles and configure files
if ( $ACLOCAL --version ) < /dev/null > /dev/null 2>&1; then
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

if ( $AUTOHEADER --version ) < /dev/null > /dev/null 2>&1; then
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

if ( $AUTOMAKE --version ) < /dev/null > /dev/null 2>&1; then
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

if ( $AUTOCONF --version ) < /dev/null > /dev/null 2>&1; then
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
echo -n "Building lib/configure ... "
rm -f lib/configure
$GNUM4 lib/configure.m4 > lib/configure
chmod a+x lib/configure
echo "done."

echo
echo 'run "./configure ; make"'
echo
