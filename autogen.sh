#!/bin/sh

ACLOCAL="aclocal -I ${PWD}/m4"
AUTOHEADER="autoheader"
AUTOMAKE="automake --add-missing --copy --foreign"
AUTOCONF="autoconf"
ACINCLUDE_FILES="lyxinclude.m4 libtool.m4 xforms.m4 qt.m4 gtk--.m4 gnome--.m4 gnome.m4 spell.m4 pkg.m4"

# Discover what version of automake we are using.
automake_version=`$AUTOMAKE --version 2>/dev/null | head -n 1`

test "$automake_version" != "" && {
    echo "Using $automake_version"
} || {
    echo "LyX requires automake >= 1.9"
    exit 1
}

case $automake_version in
    *' '1.9*|*' '1.10*)
	;;
    *)
    
	echo "This automake version is not supported by LyX."
        echo "LyX only supports automake 1.9 and 1.10."
        exit 1
        ;;
esac
							
# Discover what version of autoconf we are using.
autoversion=`$AUTOCONF --version 2>/dev/null | head -n 1`

test "$autoversion" != "" && {
    echo "Using $autoversion"
} || {
    echo "LyX requires autoconf >= 2.52"
    exit 1
}
	    
case $autoversion in
    *' '2.5[2-9]|*' '2.60[ab]|*' '2.6[0-1])
	EXTRA_ACINCLUDE_FILES="lyxinclude25x.m4"
	;;
    *)
	echo "This autoconf version is not supported by LyX."
	echo "LyX only supports autoconf 2.52-2.61."
	exit 1
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
	exit 1
else
	echo `which $GNUM4`
fi

# Delete old cache directories.
# automake will stop if their contents was created by an earlier version.
rm -rf autom4te.cache

# Generate acinclude.m4
echo -n "Generate acinclude.m4... "
rm -f acinclude.m4
(cd config ; cat ${ACINCLUDE_FILES} ${EXTRA_ACINCLUDE_FILES} >../acinclude.m4)
echo "done."

# Generate the Makefiles and configure files
if ( $ACLOCAL --version ) < /dev/null > /dev/null 2>&1; then
	echo "Building macros..."
	for dir in . ; do
	    echo "        $dir"
	    ( cd $dir ; $ACLOCAL )
	done
	echo "done."
else
	echo "aclocal not found -- aborting"
	exit 1
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
	exit 1
fi

if ( $AUTOMAKE --version ) < /dev/null > /dev/null 2>&1; then
	echo "Building Makefile templates..."
	for dir in . ; do
	    echo "        $dir"
	    ( cd $dir ; $AUTOMAKE )
	done
	echo "done."
else
	echo "automake not found -- aborting"
	exit 1
fi

if ( $AUTOCONF --version ) < /dev/null > /dev/null 2>&1; then
	echo "Building configure..."
	for dir in . ; do
	    echo "       $dir"
	    ( cd $dir ; $AUTOCONF )
	done
	echo "done."
else
	echo "autoconf not found -- aborting"
	exit 1
fi

echo
echo 'run "./configure ; make"'
echo
