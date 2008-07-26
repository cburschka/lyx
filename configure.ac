dnl Process with autoconf to generate configure script   -*- sh -*-

AC_INIT(LyX,1.5.7svn,[lyx-devel@lists.lyx.org],[lyx])
AC_SUBST(LYX_DATE, ["Sun, Jul 27, 2008"])
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(src/main.cpp)
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_AUX_DIR(config)

# First check the version
LYX_CHECK_VERSION
LYX_VERSION_SUFFIX
# Check how the files should be packaged
AC_CANONICAL_TARGET
LYX_USE_PACKAGING
# We need to define these variables here and the no-define option of
# AM_INIT_AUTOMAKE above because we alter $PACKAGE in LYX_USE_PACKAGING.
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])

dnl default maintainer mode to true for development versions
if test "${enable_maintainer_mode+set}" != set; then
  enable_maintainer_mode=$lyx_devel_version
fi
AM_MAINTAINER_MODE

save_PACKAGE=$PACKAGE
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-define 1.5])
PACKAGE=$save_PACKAGE

### Set the execute permissions of the various scripts correctly
for file in config/install-sh config/mkinstalldirs ; do
  chmod 755 ${srcdir}/${file}
done

### Check for programs
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AC_PROG_AWK
test "$AWK" = gawk && AWK="gawk --posix"

#AC_PROG_RANLIB
AC_CHECK_PROG(KPSEWHICH, kpsewhich, kpsewhich, :)
if test "x$KPSEWHICH" = xkpsewhich ; then
    AC_DEFINE(HAVE_KPSEWHICH, 1,
    [Define this if you have the kpsewhich program working on your system.])
fi
AC_CHECK_PROGS(M4, gm4 gnum4 m4, m4)

# Check for installed python
AM_PATH_PYTHON(2.3,, :)

# Work around a problem in automake 1.4: when invoking install-strip,
# INSTALL_PROGRAM is changed to 'install -s', and since
# INSTALL_SCRIPT==INSTALL_PROGRAM, we get errors with fileutils-4.0
# which returns an error condition when stripping fails.
INSTALL_SCRIPT='${INSTALL}'

### we will also need a C compiler to compile GNU gettext
AC_PROG_CC

### check for special systems
AC_ISC_POSIX
AC_AIX

### we need to know the byte order for unicode conversions
AC_C_BIGENDIAN

### check which frontend we want to use
LYX_USE_FRONTENDS

### Check for a C++ compiler
LYX_PROG_CXX
### Some checks on what the C++ compiler can(not) do
AC_LANG(C++)
dnl we do not need that currently (and probably all our supported
dnl compiler allow that)
dnl LYX_CXX_PARTIAL
LYX_CXX_EXPLICIT
LYX_CXX_GLOBAL_CSTD
LYX_STD_COUNT
dnl we disable rtti for now
dnl LYX_CXX_RTTI
AC_CHECK_HEADERS(ostream istream sstream locale limits ios)
LYX_CXX_STL_MODERN_STREAMS

### and now some special lyx flags.
AC_ARG_ENABLE(assertions,
  AC_HELP_STRING([--enable-assertions],[add runtime sanity checks in the program]),,
  [if test $lyx_devel_version = yes -o $lyx_prerelease = yes ; then
	enable_assertions=yes;
    else
	enable_assertions=no;
    fi;])
if test "x$enable_assertions" = xyes ; then
   lyx_flags="assertions $lyx_flags"
   AC_DEFINE(ENABLE_ASSERTIONS,1,
    [Define if you want assertions to be enabled in the code])
fi

### Library Files
AC_CHECK_LIB(m, sin)

### Add extra directories to check for libraries.
LYX_WITH_DIR([extra-lib],[extra library directory],extra_lib, NONE)
LYX_LOOP_DIR($lyx_cv_extra_lib,LYX_ADD_LIB_DIR(lyx_ldflags,$dir))
test ! x"$lyx_ldflags" = x && LDFLAGS="$lyx_ldflags $LDFLAGS"

### Add extra directories to check for include files.
LYX_WITH_DIR([extra-inc],[extra include directory],extra_inc, NONE)
LYX_LOOP_DIR($lyx_cv_extra_inc,LYX_ADD_INC_DIR(lyx_cppflags,$dir))
test ! x"$lyx_cppflags" = x && CPPFLAGS="$lyx_cppflags $CPPFLAGS"

### Add both includes and libraries
LYX_WITH_DIR([extra-prefix],[extra lib+include directory],extra_prefix, NONE, ${prefix})
LYX_LOOP_DIR($lyx_cv_extra_prefix,[
LYX_ADD_INC_DIR(CPPFLAGS,$dir/include)
  LYX_ADD_LIB_DIR(LDFLAGS,$dir/lib)])

### These are needed in windows
AC_CHECK_LIB(shlwapi, main, [LIBSHLWAPI=-lshlwapi])
AC_SUBST(LIBSHLWAPI)
AC_CHECK_LIB(gdi32, main)

AC_ARG_WITH(aiksaurus,
  [  --without-aiksaurus     do not use the Aiksaurus library],
  [lyx_use_aiksaurus=$withval])
if test x$lyx_use_aiksaurus != xno; then
AC_CHECK_LIB(Aiksaurus, main,
	[AC_DEFINE(HAVE_LIBAIKSAURUS,1,[Define this if you have the AikSaurus library])
	 AIKSAURUS_LIBS="-lAiksaurus"
	 lyx_flags="aiksaurus $lyx_flags"
	])
AC_CHECK_HEADER(Aiksaurus.h,[
  ac_cv_header_aiksaurus_h=yes
  lyx_cv_aiksaurus_h_location="<Aiksaurus.h>"])
AC_CHECK_HEADER(Aiksaurus/Aiksaurus.h,[
  ac_cv_header_aiksaurus_h=yes
  lyx_cv_aiksaurus_h_location="<Aiksaurus/Aiksaurus.h>"])
AC_DEFINE_UNQUOTED(AIKSAURUS_H_LOCATION,$lyx_cv_aiksaurus_h_location,[Location of Aiksaurus.h])
fi
AC_SUBST(AIKSAURUS_LIBS)

LYX_USE_INCLUDED_BOOST

# Needed for our char_type
AC_CHECK_SIZEOF(wchar_t)

### Setup libtool
dnl Dirty trick ahead: disable libtool checking for a fortran compiler
dnl see http://permalink.gmane.org/gmane.comp.gnu.libtool.general/6699
dnl Note that this will reportedly not be needed with libtool 2.0
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])
AC_DISABLE_SHARED
AC_LIBTOOL_WIN32_DLL
#AM_PROG_LIBTOOL
LYX_PROG_LIBTOOL

### We need iconv for unicode support (Qt4 frontend requires it too)
AM_ICONV
if test "$am_cv_func_iconv" = no; then
  LYX_ERROR([Cannot find required library iconv])
else
  LIBS="$LIBS $LIBICONV"
fi

### check for compression support
AC_CHECK_HEADERS(zlib.h,
 [AC_CHECK_LIB(z, gzopen, [LIBS="$LIBS -lz"], LYX_LIB_ERROR(libz,zlib))],
 [LYX_LIB_ERROR(zlib.h,zlib)])


### Check for X libraries
AC_PATH_XTRA
case $have_x in
  yes) LIBS="$X_PRE_LIBS $LIBS $X_LIBS -lX11 $X_EXTRA_LIBS"
       CPPFLAGS="$CPPFLAGS $X_CFLAGS";;
  no) LYX_ERROR(dnl
[Cannot find X window libraries and/or headers. Check your installation.
  If you use a Linux system, check that you have installed
  the development tools.]);;
  disable) ;;
esac

### check which frontend we want to use

dnl The code below is not in a macro, because this would cause big
dnl problems with the AC_REQUIRE contained in QT_DO_IT_ALL.
for frontend in $FRONTENDS ; do
  case "$frontend" in
    qt4)
	  QT4_DO_IT_ALL
	  FRONTENDS_PROGS="$FRONTENDS_PROGS lyx-qt4\$(EXEEXT)"
	  FRONTENDS_SUBDIRS="$FRONTENDS_SUBDIRS qt4"
	  RPM_FRONTEND="qt4"
	  FRONTEND_INFO="${FRONTEND_INFO}\
  Qt 4 Frontend:\n\
      Qt 4 version:\t\t${QT4_VERSION}\n"
dnl qt 4 build will fail without moc or uic
	  if test -z "$MOC4"; then
	    LYX_ERROR([moc 4 binary not found !])
	  fi
	  if test -z "$UIC4"; then
	    LYX_ERROR([uic 4 binary not found !])
	  fi
	  if test -z "$QT4_LIB"; then
	    LYX_ERROR([qt 4 library not found !])
	  fi
      ;;
    *)
	  LYX_ERROR(Unknown frontend '$frontend');;
  esac
done

# fix the value of the prefixes.
test "x$prefix" = xNONE && prefix=$default_prefix
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
if echo $prefix |grep ' ' >/dev/null 2>/dev/null ; then
  LYX_WARNING([The installation prefix \"${prefix}\" contains a space, which
   causes problems with the Makefiles. The installation will be done in
   directory \"`pwd`/installprefix\" instead. Please move its contents to
   the right place after installation.])
  prefix=`pwd`/installprefix
fi

### Setup GNU gettext
dnl GNU gettext is written in C
AC_LANG_PUSH(C)
AM_GNU_GETTEXT
AC_LANG_POP(C)

# some standard header files
AC_HEADER_DIRENT
AC_HEADER_MAJOR
AC_CHECK_HEADERS(sys/time.h sys/types.h sys/select.h strings.h locale.h utime.h sys/utime.h io.h process.h NewAPIs.h)

# some standard structures
AC_HEADER_STAT
AC_HEADER_TIME

# some standard types
AC_CHECK_TYPE(mode_t,[AC_DEFINE(HAVE_MODE_T, 1, [Define this to 1 if your compiler supports the mode_t type.])])
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_UID_T

AC_CHECK_FUNCS(strerror)
LYX_CHECK_DECL(istreambuf_iterator, iterator)
LYX_CHECK_DECL(mkstemp,[unistd.h stdlib.h])

# Check the form of mkdir()
AC_FUNC_MKDIR

dnl This is a slight hack: the tests generated by autoconf 2.52 do not
dnl work correctly because of some conflict with stdlib.h with g++ 2.96
dnl We aim to remove this eventually, since we should test as much as
dnl possible with the compiler which will use the functions (JMarc)
AC_LANG_PUSH(C)
AC_CHECK_FUNCS(chmod close _close getpid _getpid lstat mkfifo mkstemp mktemp open _open pclose _pclose popen _popen readlink)
AC_LANG_POP(C)

LYX_CHECK_SPELL_ENGINES

lyx_client_subdir=true
AC_LANG_PUSH(C)
dnl LIBS already contains some X extra libs that may interfere.
save_LIBS="$LIBS"
LIBS=
AC_CHECK_FUNCS(fcntl,
  [AC_SEARCH_LIBS([gethostbyname], [nsl])
   AC_SEARCH_LIBS([socket], [socket], [],
     [AC_CHECK_LIB([socket], [socket], [LIBS="-lsocket -lnsl $LIBS"],
		[], [-lnsl])])],
  [lyx_client_subdir=false])
AC_SUBST(SOCKET_LIBS,$LIBS)
LIBS="$save_LIBS"
AM_CONDITIONAL(BUILD_CLIENT_SUBDIR, $lyx_client_subdir)
AC_LANG_POP(C)

lyx_win_res=false;
case ${host} in
    *mingw*|*cygwin*) lyx_win_res=true;;
esac
AM_CONDITIONAL(LYX_WIN_RESOURCE, $lyx_win_res)
LYX_SET_VERSION_INFO

AC_FUNC_SELECT_ARGTYPES

### Some information on what just happened
real_bindir=`eval "echo \`eval \"echo ${bindir}\"\`"`
real_pkgdatadir=`eval "echo \`eval \"echo \\\`eval \\\"echo ${pkgdatadir}\\\"\\\`\"\`"`
real_localedir=`eval "echo \`eval \"echo ${datadir}/locale\"\`"`
VERSION_INFO="Configuration\n\
  Host type:                    ${host}\n\
  Special build flags:          ${lyx_flags}\n\
  C   Compiler:                 ${CC} ${CC_VERSION}\n\
  C   Compiler LyX flags:       ${AM_CPPFLAGS} ${AM_CFLAGS}\n\
  C   Compiler flags:           ${CPPFLAGS} ${CFLAGS}\n\
  C++ Compiler:                 ${CXX} ${CXX_VERSION}\n\
  C++ Compiler LyX flags:       ${AM_CPPFLAGS} ${AM_CXXFLAGS}\n\
  C++ Compiler flags:           ${CPPFLAGS} ${CXXFLAGS}\n\
  Linker flags:                 ${AM_LDFLAGS}\n\
  Linker user flags:            ${LDFLAGS}\n\
${FRONTEND_INFO}\
  Packaging:                    ${lyx_use_packaging}\n\
  LyX binary dir:               ${real_bindir}\n\
  LyX files dir:                ${real_pkgdatadir}\n"

MSYS_AC_CANONICAL_PATH(LYX_ABS_TOP_SRCDIR, ${srcdir})
MSYS_AC_CANONICAL_PATH(LYX_ABS_INSTALLED_LOCALEDIR, ${real_localedir})
MSYS_AC_CANONICAL_PATH(LYX_ABS_INSTALLED_DATADIR, ${real_pkgdatadir})

AC_SUBST(VERSION_INFO)
AC_SUBST(RPM_FRONTEND)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(LYX_ABS_TOP_SRCDIR)
AC_SUBST(LYX_ABS_INSTALLED_LOCALEDIR)
AC_SUBST(LYX_ABS_INSTALLED_DATADIR)

## Some config.h stuff

AH_TOP([
/* -*- C++ -*- */
/*
 * \file config.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * This is the compilation configuration file for LyX.
 * It was generated by autoconfs configure.
 * You might want to change some of the defaults if something goes wrong
 * during the compilation.
 */

#ifndef _CONFIG_H
#define _CONFIG_H
])

AH_BOTTOM([
/************************************************************
 ** You should not need to change anything beyond this point */

#ifndef HAVE_STRERROR
#if defined(__cplusplus)
extern "C"
#endif
char * strerror(int n);
#endif

#ifdef HAVE_MKSTEMP
#ifndef HAVE_DECL_MKSTEMP
#if defined(__cplusplus)
extern "C"
#endif
int mkstemp(char*);
#endif
#endif

#if defined(HAVE_OSTREAM) && defined(HAVE_LOCALE) && defined(HAVE_SSTREAM)
#  define USE_BOOST_FORMAT 1
#else
#  define USE_BOOST_FORMAT 0
#endif

#define BOOST_USER_CONFIG <config.h>

#if !defined(ENABLE_ASSERTIONS)
#  define BOOST_DISABLE_ASSERTS 1
#endif
#define BOOST_ENABLE_ASSERT_HANDLER 1

#define BOOST_DISABLE_THREADS 1
#define BOOST_NO_WREGEX 1
#define BOOST_NO_WSTRING 1

#ifdef __CYGWIN__
#  define BOOST_POSIX 1
#  define BOOST_POSIX_API 1
#  define BOOST_POSIX_PATH 1
#endif

#if defined(HAVE_NEWAPIS_H)
#  define WANT_GETFILEATTRIBUTESEX_WRAPPER 1
#endif

/*
 * the FreeBSD libc uses UCS4, but libstdc++ has no proper wchar_t
 * support compiled in:
 * http://gcc.gnu.org/onlinedocs/libstdc++/faq/index.html#3_9
 * And we are not interested at all what libc
 * does: What we need is a 32bit wide wchar_t, and a libstdc++ that
 * has the needed wchar_t support and uses UCS4. Whether it
 * implements this with the help of libc, or whether it has own code
 * does not matter for us, because we don't use libc directly (Georg)
*/
#if defined(HAVE_WCHAR_T) && SIZEOF_WCHAR_T == 4 && !defined(__FreeBSD__) && !defined(__FreeBSD_kernel__)
#  define USE_WCHAR_T
#endif

#endif
])

AC_DEFINE_UNQUOTED([LYX_MAJOR_VERSION],$lyx_major,[Major version number])
AC_DEFINE_UNQUOTED([LYX_MINOR_VERSION],$lyx_minor,[Minor version number])
AC_DEFINE_UNQUOTED([LYX_RELEASE_LEVEL],$lyx_release,[Release version number])
AC_DEFINE_UNQUOTED([LYX_RELEASE_PATCH],$lyx_patch,[Patch version number])

### Finish the work.
AC_CONFIG_FILES([Makefile
       boost/Makefile \
       boost/libs/Makefile \
       boost/libs/filesystem/Makefile \
       boost/libs/filesystem/src/Makefile \
       boost/libs/iostreams/Makefile \
       boost/libs/iostreams/src/Makefile \
       boost/libs/regex/Makefile \
       boost/libs/regex/src/Makefile \
       boost/libs/signals/Makefile \
       boost/libs/signals/src/Makefile \
       config/Makefile \
       development/Makefile \
       development/MacOSX/Makefile \
       development/MacOSX/Info.plist \
       development/MacOSX/lyxrc.dist \
       development/MacOSX/spotlight/Makefile \
       development/lyx.spec \
       intl/Makefile \
       lib/Makefile \
       lib/doc/Makefile \
       lib/lyx2lyx/lyx2lyx_version.py \
       lib/lyx2lyx/Makefile \
       m4/Makefile \
       po/Makefile.in \
       sourcedoc/Doxyfile \
       sourcedoc/Makefile \
       src/client/Makefile \
       src/Makefile \
       src/version.cpp-tmp:src/version.cpp.in \
       src/tex2lyx/Makefile \
       src/mathed/Makefile \
       src/graphics/Makefile \
       src/insets/Makefile \
       src/support/Makefile \
       src/support/tests/Makefile \
       src/frontends/Makefile \
       src/frontends/controllers/Makefile \
       src/frontends/controllers/tests/Makefile \
       src/frontends/qt4/Makefile \
       src/frontends/qt4/ui/Makefile \
])

AC_OUTPUT
# show version information
echo
printf "$VERSION_INFO"
echo

# Display a final warning if there has been a LYX_ERROR
LYX_CHECK_ERRORS
