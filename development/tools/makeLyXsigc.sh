#!/bin/sh
## Author: Allan Rae <rae@lyx.org>
## With bug fixes and improvements by Karl Nelson.
##
## Severely hacked version of the libsigc++/win32/makevc.sh script
## Builds the 'mini' libsigc++ distribution for use with LyX.
## Hopefully the generated distribution is generic enough for use by
## other apps also.
##
if test ! -f .cvsignore ; then
  ## could also test for acconfig.h since we need that too.
  echo "Must be run in a CVS controlled source directory"
  echo "You can't build the lyx-sigc distribution from a tarball"
  ## This just makes my life considerably easier.
  exit 1
fi

if test ! -f config.status ; then
  echo "Must be run in configured source directory"
  exit 1
fi

version=`grep SIGC_VERSION config.status | awk -F% '{print $3}'`
package="libsigc++-$version"
tarfile="$package.tar.gz"
configure="$package/configure.in"

if test ! -f $tarfile ; then
  echo "Must have dist file $tarfile"
  echo "Try:  make dist"
  exit 1
fi

echo "Unpack distribution."
if test -d $package; then
rm -R $package
fi
tar xzf $tarfile --exclude CVS



echo "Removing irrelevent or regeneratable files."
find $package -name "Makefile.in" | xargs rm
find $package -name "*.Makefile" | xargs rm
( cd $package;
  cp ../acconfig.h sigc++;
  mv [ACFT]* doc;
  rm -f doc/riscos doc/ChangeLog;
  mv  ltdll.c sigc-config.in doc sigc++;
  rm -Rf lyx win32 scripts tests riscos djgpp borland examples;
  rm -Rf aclocal.m4 libsigc++.spec*;
  cd sigc++/ ;
    find macros -name "*.h.m4" | sed -e 's,macros/\(.*\)\.m4,\1,' | xargs rm;
    rm -rf README scripts/README generator.h config )


echo "Make autogen.sh for testing"
( cd $package/sigc++;
  cat > autogen.sh << EOF
#!/bin/sh
#
# You shouldn't need to use this script in your application unless it doesn't
# have its own equivalent.
#
libtoolize --automake
aclocal $ACLOCAL_FLAG
autoheader
automake --add-missing --foreign
autoconf
EOF
  chmod a+x autogen.sh 
)


echo "Limiting to Signal2"
# we only need up to Signal2
# NOTE: sigc++ needs 2 slots higher than the number of signals (see bind.h)
# Anyone feeling keen?
# How would you like to make this a parameter-controlled stripping of
# the headers instead?
#
for i in `ls $package/sigc++/macros/*.h.m4`; do
  case $i in
    $package/sigc++/macros/slot.h.m4)
        sed < $i > $i.tmp \
		-e '/.*ARGS(P,[56789]).*/d'
        ;;
    *)
        sed < $i > $i.tmp \
		-e '/.*ARGS(P,[3456789]).*/d'
        ;;
  esac
  rm -f $i
  mv $i.tmp $i
done


echo "Changing <sigc++config.h> to <sigc++/sigc++config.h>"
#
# This change means that if the compiler can find <sigc++/slot.h> it can also
# find the sigc++config.h header which is internal to the library.
#
for i in `ls $package/sigc++/macros/*.h.m4 $package/sigc++/*.h`; do
  sed < $i > $i.tmp \
	-e 's%\(\(sigc++\)config.h\)%\2/\1%'
  rm -f $i
  mv $i.tmp $i
done


echo "Building application level config/"
mkdir $package/config
mv $package/sigc++.m4 $package/config


echo "Creating custom sigc++/configure.in"
## My sed doesn't seem to want to match newlines in scripts. As a result
## the first two scripts haven't been merged into one.
## These separate scripts are ordered approximately in the order in which
## they match in the unmodified configure.in.
##
## Note that you have to be very careful about quoting.  Look at the second
## script for example: '\'', \\\ and \$
##
## Now for some explanation of what each script is supposed to change: 
##
#       -e 's/\(Nelson\)/\1\
### Modified by makeLyXsigc.sh (Allan Rae)/'
#
#  Credit where credit is due.
#
#    -e 's%\(AC_INIT\)(sigc++)%\1(slot.cc)\
#\
#AC_DEFUN(LYX_FIX_MAKEFILE_IN,[\
### modify Makefile.in if need be -- since we still use automake\
#for dir in \$srcdir \$srcdir/macros ; do\
#( cd \$dir ;\
#  sed < Makefile.in > Makefile_tmp \\\
#        -e '\''s/all\: all-redirect/all: all-@USE_INCLUDED_SIGC@\\\
#all-yes: all-redirect\\\
#all-no:/'\'' ;\
#  mv Makefile_tmp Makefile.in )\
#done\
#])\
#\
#LYX_FIX_MAKEFILE_IN%'
#
# Change which file autoconf demands to be in the current directory.
# Add a way to not make anything in the sigc++ package when using an
# externally supplied library.
#
#    -e 's%\(AUX_DIR(\)scripts%\1../config%'
#
# Use the applications auxilliary directory. Assumed to be ../config. 
#
#    -e 's%config/\(sigc++config\.h\)%\1%'
#
# Move sigc++config.h out of the now removed config/.
#
#    -e '/.*\.spec.*/d' \
#    -e '\%.*sigc++/Makefile.*%d' \
#    -e '\%.*config/Makefile.*%d'
#
# Remove unnecessary files from the AC_OUTPUT list.
#
#    -e 's,sigc++/,,g'
#
# No subdir sigc++.  This also results in sigc++/macros -> macros.
#
#    -e '\%.*tests/.*%d' \
#    -e '\%.*examples/.*%d' \
#    -e '\%.*scripts/.*%d'
#
# These directories have been completely removed and aren't relevent.
#
#    -e 's/.*\(AM_DISABLE_\)STATIC/\1SHARED/' \
#
# We want to default to static libs.  Although we still need to ensure that
# configure is forced to only produce static libs.
#
#    -e 's/AM_MAINTAINER_MODE/AC_ARG_WITH([included-libsigc],\
#  [  --without-included-libsigc\
#                             Use the libsigc++ installed on the system\
#                             Used when libsigc++ is bundled with applications],\
#  [sigc_use_included_libsigc=$withval],\
#  [sigc_use_included_libsigc=yes])\
#if test x$sigc_use_included_libsigc = xno; then\
#  ## just change the setting for the Makefile\
#  USE_INCLUDED_SIGC=no\
#  AC_SUBST(USE_INCLUDED_SIGC)\
#else\
####\
#### This else..fi goes till the end of the file!\
####\
#  USE_INCLUDED_SIGC=yes\
#  AC_SUBST(USE_INCLUDED_SIGC)/'
#
# Kill two birds with the one stone.  We want maintainer access as standard
# and we want to be able to choose between using this package or the full
# library installed on the local system.  If using the latter option we don't
# have to run most of the configure script so we hide it in a conditional.
#
#    -e 's/\(AC_OUTPUT.*\)/\
####\
#### This is the end of the massive if..then..else..fi\
####\
#fi\
#\
#\1/'
#
# Close the conditional introduced in the previous script.
#
( cd $package;
  sed < configure.in > sigc++/configure.in \
    -e 's/\(Nelson\)/\1\
## Modified by makeLyXsigc.sh (Allan Rae)/' \
    -e 's%\(AC_INIT\)(sigc++)%\1(slot.cc)\
\
AC_DEFUN(LYX_FIX_MAKEFILE_IN,[\
## modify Makefile.in if need be -- since we still use automake\
for dir in \$srcdir \$srcdir/macros ; do\
( cd \$dir ;\
  sed < Makefile.in > Makefile_tmp \\\
        -e '\''s/all\: all-redirect/all: all-@USE_INCLUDED_SIGC@\\\
all-yes: all-redirect\\\
all-no:/'\'' ;\
  mv Makefile_tmp Makefile.in )\
done\
])\
\
LYX_FIX_MAKEFILE_IN%' \
    -e 's%\(AUX_DIR(\)scripts%\1../config%' \
    -e 's%config/\(sigc++config\.h\)%\1%' \
    -e '/.*\.spec.*/d' \
    -e '\%.*sigc++/Makefile.*%d' \
    -e '\%.*config/Makefile.*%d' \
    -e 's,sigc++/,,g' \
    -e '\%.*tests/.*%d' \
    -e '\%.*examples/.*%d' \
    -e '\%.*scripts/.*%d' \
    -e 's/.*\(AM_DISABLE_\)STATIC/\1SHARED/' \
    -e 's/AM_MAINTAINER_MODE/AC_ARG_WITH([included-libsigc],\
  [  --without-included-libsigc\
                             Use the libsigc++ installed on the system\
                             Used when libsigc++ is bundled with applications],\
  [sigc_use_included_libsigc=$withval],\
  [sigc_use_included_libsigc=yes])\
if test x$sigc_use_included_libsigc = xno; then\
  ## just change the setting for the Makefile\
  USE_INCLUDED_SIGC=no\
  AC_SUBST(USE_INCLUDED_SIGC)\
else\
###\
### This else..fi goes till the end of the file!\
###\
  USE_INCLUDED_SIGC=yes\
  AC_SUBST(USE_INCLUDED_SIGC)/' \
    -e 's/\(AC_OUTPUT.*\)/\
###\
### This is the end of the massive if..then..else..fi\
###\
fi\
\
\1/' )

echo "Modifying sigc++/Makefile.am"
#
#    -e 's%\(SUBDIRS =\)\(.*\)config\(.*\)%\1\2\3\
#DISTCLEANFILES= *.orig *.rej *~ *.bak core \
#MAINTAINERCLEANFILES= \$(srcdir)/Makefile.in \$(srcdir)/configure \$(srcdir)/sigc++config* \$(srcdir)/stamp* \$(srcdir)/aclocal.m4 \$(srcdir)/acinclude.m4 \
#ETAGS_ARGS = --lang=c++%'
#
# We've removed the sigc++/config/ above now we just make sure automake knows.
# We also want maintainer-clean to actually remove stuff. Same for distclean.
#
#    -e '/^INCLUDES/{s%\(dir)\)%\1/..%;}'
#
# All the headers are included with <sigc++/xxx.h> so we need an adjustment.
#
#    -e '/^M4_DIR/{s/sigc++\///;}'
#
# In the mini-dist the macros are directly under the top_srcdir which in this
# case is sigc++/.
#
#    -e '/^libsigc.*dir/d'
#
# Not installing the headers or the library.
#
#    -e 's/^libsigcinclude/noinst/'
#
# Don't install the headers.
#
#    -e 's/generator.h//'
#
# We don't need generator.h.
#
#    -e 's/lib_//' \
#    -e 's/\(LTLIB\)/noinst_\1/'
#
# These two lines make sure we don't install the library.
#
#    -e '/EXTRA_DIST.*/d';
#
# We don't want the extra bits that libsigc++ does.
#
( cd $package/sigc++;
  sed < Makefile.am > Makefile.tmp \
    -e 's%\(SUBDIRS =\)\(.*\)config\(.*\)%\1\2\3\
DISTCLEANFILES= *.orig *.rej *~ *.bak core \
MAINTAINERCLEANFILES= \$(srcdir)/Makefile.in \$(srcdir)/configure \$(srcdir)/sigc++config* \$(srcdir)/stamp* \$(srcdir)/aclocal.m4 \$(srcdir)/acinclude.m4 \
ETAGS_ARGS = --lang=c++%' \
    -e '/^INCLUDES/{s%\(dir)\)%\1/..%g;}' \
    -e '/^M4_DIR/{s/sigc++\///;}' \
    -e '/^libsigc.*dir/d' \
    -e 's/^libsigcinclude/noinst/' \
    -e 's/generator.h//' \
    -e 's/lib_//' \
    -e 's/\(LTLIB\)/noinst_\1/' \
    -e '/EXTRA_DIST.*/d' \
    -e 's/-version-info.*ION)//' \
    -e 's/-release.*EASE)//';
  rm -f Makefile.am ;
  mv Makefile.tmp Makefile.am )


echo "Modifying sigc++/macros/Makefile.am"
#
#    -e '/^m4includedir/d' \
#    -e 's/^m4include/noinst/'
#
# We're not installing any of this.
#
#    -e '$a\
#MAINTAINERCLEANFILES=\$(srcdir)/Makefile.in'
#
# Clean up.
#
( cd $package/sigc++/macros;
  sed < Makefile.am > Makefile.tmp \
    -e '/^m4includedir/d' \
    -e 's/^m4include/noinst/' \
    -e '$a\
MAINTAINERCLEANFILES=\$(srcdir)/Makefile.in' ;
  rm -f Makefile.am ;
  mv Makefile.tmp Makefile.am )


echo "Adding various .cvsignore"
( cd $package/sigc++;
  cp ../../sigc++/.cvsignore . ;
  cp ../../sigc++/macros/.cvsignore macros;
  cp ../../doc/.cvsignore doc;
  cat >> .cvsignore <<EOF
aclocal.m4
config.log
config.status
configure
libtool
sigc-config
stamp*
sigc++config.h
sigc++config.h.in
EOF
)


echo "Adding sigc++/DO_NOT_MODIFY_THESE_FILES"
( cd $package/sigc++;
  cat > DO_NOT_MODIFY_THESE_FILES <<EOF
These files are all collected together from a checked out copy of the
libsigc++ CVS repository by the ../development/tools/makeLyXsigc.sh file.
Some of these files are modified by that script.  If you need to fix a bug in
the code then you should try to get the latest CVS libsigc++ and regenerate
this entire subpackage. If the problem still persists it may be necessary to
modify the generation script.

If an error is found while porting to a new platform read the doc/requirements
file.

Remember to contact the LyX developers list if you are having problems.
Allan. (ARRae)
EOF
)


echo "Packing files."
( cd $package ;
  chmod -R a+r sigc++ config ;
  GZIP=--best tar chozf lyx-sigc++-$version.tar.gz sigc++ config )
mv $package/lyx*gz .
#rm -Rf $package

chmod a+r lyx-sigc++-$version.tar.gz
