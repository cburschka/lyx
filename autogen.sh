#!/bin/sh

ACLOCAL=aclocal
AUTOHEADER=autoheader
AUTOMAKE="automake -a -c --foreign"
AUTOCONF=autoconf
GNUM4=

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
rm -f acinclude.m4 sigc++/acinclude.m4 boost/acinclude.m4
touch acinclude.m4
for fil in config/lyxinclude.m4 config/libtool.m4 config/gettext.m4 config/lcmessage.m4 config/progtest.m4 config/sigc++.m4 config/kde.m4 config/qt2.m4 config/gtk--.m4 config/gnome--.m4 config/gnome.m4 config/pspell.m4; do
    cat $fil >> acinclude.m4
done
touch sigc++/acinclude.m4
for fil in config/libtool.m4 ; do
    cat $fil >> sigc++/acinclude.m4
done
touch boost/acinclude.m4
for fil in config/boost.m4 ; do
    cat $fil >> boost/acinclude.m4
done
echo "done."

# Generate the Makefiles and configure files
if ( aclocal --version ) </dev/null > /dev/null 2>&1; then
	echo "Building macros..."
	for dir in . lib/reLyX sigc++ boost ; do
	    echo -e "\t$dir"
	    ( cd $dir ; $ACLOCAL )
	done
#	$ACLOCAL ; ( cd lib/reLyX; $ACLOCAL ) ; ( cd sigc++; $ACLOCAL ) 
#	( cd boost; $ACLOCAL ) 
	echo "done."
else
	echo "aclocal not found -- aborting"
	exit
fi

if ( autoheader --version ) </dev/null > /dev/null 2>&1; then
	echo "Building config header template..."
	for dir in . sigc++ boost ; do
	    echo -e "\t$dir"
	    ( cd $dir ; $AUTOHEADER )
	done
#	$AUTOHEADER ; ( cd sigc++; $AUTOHEADER ) ; ( cd boost; $AUTOHEADER ) 
	echo "done."
else
	echo "autoheader not found -- aborting"
	exit
fi

if ( $AUTOMAKE --version ) </dev/null > /dev/null 2>&1; then
	echo "Building Makefile templates..."
	for dir in . lib/reLyX sigc++ boost ; do
	    echo -e "\t$dir"
	    ( cd $dir ; $AUTOMAKE )
	done
#	$AUTOMAKE ; ( cd lib/reLyX ; $AUTOMAKE ) ; ( cd sigc++; $AUTOMAKE ) 
#	( cd boost; $AUTOMAKE )
	echo "done."
else
	echo "automake not found -- aborting"
	exit
fi

if ( $AUTOCONF --version ) </dev/null > /dev/null 2>&1; then
	echo "Building configure..."
	for dir in . lib/reLyX sigc++ boost ; do
	    echo -e "\t$dir"
	    ( cd $dir ; $AUTOCONF )
	done
#	$AUTOCONF ; ( cd lib/reLyX ; $AUTOCONF ) ; ( cd sigc++; $AUTOCONF )
#	( cd boost; $AUTOCONF ) 
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
