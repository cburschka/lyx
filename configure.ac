dnl Process with autoconf to generate configure script   -*- sh -*-

AC_INIT(LyX,2.2.0dev,[lyx-devel@lists.lyx.org],[lyx])
AC_PRESERVE_HELP_ORDER
# Use ISO format only. The frontend needs to parse this
AC_SUBST(LYX_DATE, ["2014-04-14"])
AC_PREREQ(2.60)
AC_CONFIG_SRCDIR(src/main.cpp)
AC_CONFIG_HEADERS([config.h])

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

dnl Default maintainer mode to true for development versions
if test "${enable_maintainer_mode+set}" != set; then
  enable_maintainer_mode=$lyx_devel_version
fi
AM_MAINTAINER_MODE

save_PACKAGE=$PACKAGE
dnl The test for _AM_PROG_CC_C_O intends to test for automake 1.14 or newer,
dnl which are the versions where subdir-objects can beused.
dnl see: https://www.mail-archive.com/lyx-devel@lists.lyx.org/msg181023.html
m4_define([LYX_OBJECTS_LAYOUT], m4_ifdef([_AM_PROG_CC_C_O],[subdir-objects],))
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-define 1.8] LYX_OBJECTS_LAYOUT)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
PACKAGE=$save_PACKAGE

LYX_CHECK_GIT_COMMIT

# Allow to build some parts of the code as one big chunk
m4_define([ALLPARTS],[boost,client,insets,mathed,core,tex2lyx,frontend_qt])
AC_ARG_ENABLE(monolithic-build,
  AC_HELP_STRING([--enable-monolithic-build@<:@=LIST@:>@],
		[Use monolithic build for modules in LIST (default: ALLPARTS)]),
  [test "$enable_monolithic_build" = yes && enable_monolithic_build="ALLPARTS"
   test "$enable_monolithic_build" = no && enable_monolithic_build=
   IFS="${IFS=	}"; ac_save_ifs="$IFS"; IFS=" ,"
   for i in $enable_monolithic_build ; do
       eval "enable_monolithic_$i=yes"
   done
   IFS="$ac_save_ifs"],
  [enable_monolithic_build=])

AM_CONDITIONAL(MONOLITHIC_BOOST, test "x$enable_monolithic_boost" = "xyes")
AM_CONDITIONAL(MONOLITHIC_CLIENT, test "x$enable_monolithic_client" = "xyes")
AM_CONDITIONAL(MONOLITHIC_INSETS, test "x$enable_monolithic_insets" = "xyes")
AM_CONDITIONAL(MONOLITHIC_MATHED, test "x$enable_monolithic_mathed" = "xyes")
AM_CONDITIONAL(MONOLITHIC_CORE, test "x$enable_monolithic_core" = "xyes")
AM_CONDITIONAL(MONOLITHIC_TEX2LYX, test "x$enable_monolithic_tex2lyx" = "xyes")
AM_CONDITIONAL(MONOLITHIC_FRONTEND_QT, test "x$enable_monolithic_frontend_qt" = "xyes")

### Set the execute permissions of the various scripts correctly
for file in config/install-sh ; do
  chmod 755 ${srcdir}/${file}
done

AM_PATH_PYTHON(2.4.0,, :)
AC_PROG_RANLIB

### Check for a C++ compiler
LYX_PROG_CXX
AC_LANG(C++)

### Objective-C compiler
AC_PROG_OBJC
_AM_DEPENDENCIES([OBJC])

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
AC_CHECK_LIB(psapi, main, [LIBPSAPI=-lpsapi])
AC_SUBST(LIBPSAPI)
AC_CHECK_LIB(gdi32, main)

LYX_USE_INCLUDED_BOOST
LYX_USE_INCLUDED_MYTHES

### we need to know the byte order for unicode conversions
AC_C_BIGENDIAN

# Needed for our char_type
AC_CHECK_SIZEOF(wchar_t)

### We need iconv for unicode support (Qt4 frontend requires it too)
AM_ICONV
if test "$am_cv_func_iconv" = no; then
  AC_MSG_ERROR([cannot find required library iconv.])
else
  LIBS="$LIBS $LIBICONV"
fi

### check for compression support
AC_CHECK_HEADERS(zlib.h,
 [AC_CHECK_LIB(z, gzopen, [LIBS="$LIBS -lz"], LYX_LIB_ERROR(libz,zlib))],
 [LYX_LIB_ERROR(zlib.h,zlib)])

### check for file magic support (currently optional)
AC_CHECK_HEADERS(magic.h,
 [AC_CHECK_LIB(magic, magic_open, [LIBS="$LIBS -lmagic"],
  LYX_WARNING([cannot find libmagic. Please check that the libmagic library
   is correctly installed on your system.
   Falling back to builtin file format detection.]))],
 [LYX_WARNING([cannot find magic.h. Please check that the libmagic library
   is correctly installed on your system.
   Falling back to builtin file format detection.])])

### setup the qt frontend.
dnl The code below is not in a macro, because this would cause big
dnl problems with the AC_REQUIRE contained in QT_DO_IT_ALL.
QT_DO_IT_ALL([4.5.0])
AC_SUBST([FRONTENDS_SUBDIRS], [qt4])
FRONTEND_INFO="${FRONTEND_INFO}\
  Qt Frontend:\n\
      Qt version:\t\t${QTLIB_VERSION}\n"

# fix the value of the prefixes.
test "x$prefix" = xNONE && prefix=$default_prefix
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
if echo $prefix |grep ' ' >/dev/null 2>/dev/null ; then
  LYX_WARNING([the installation prefix \"${prefix}\" contains a space, which
   causes problems with the Makefiles. The installation will be done in
   directory \"`pwd`/installprefix\" instead. Please move its contents to
   the right place after installation.])
  prefix=`pwd`/installprefix
fi

### Setup po directory
AM_PO_SUBDIRS
AM_NLS
if test $USE_NLS = "yes" ; then
  AC_DEFINE(ENABLE_NLS, 1,
    [Define to 1 if translation of program messages to the user's native language
   is requested.])dnl'
fi

# some standard header files
AC_HEADER_MAJOR
AC_CHECK_HEADERS(sys/time.h sys/types.h sys/select.h locale.h utime.h sys/utime.h)

# some standard types
AC_CHECK_TYPE(mode_t,[AC_DEFINE(HAVE_MODE_T, 1, [Define this to 1 if your compiler supports the mode_t type.])])
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T
# Taken from gettext
AC_CACHE_CHECK([for wchar_t], [gt_cv_c_wchar_t],
   [AC_TRY_COMPILE([#include <stddef.h>
      wchar_t foo = (wchar_t)'\0';], ,
      [gt_cv_c_wchar_t=yes], [gt_cv_c_wchar_t=no])])
if test $gt_cv_c_wchar_t = yes; then
  AC_DEFINE([HAVE_WCHAR_T], [1], [Define if you have the 'wchar_t' type.])
fi

LYX_CHECK_DEF(PATH_MAX, limits.h, [int n = PATH_MAX;])

AC_CHECK_FUNCS(chmod close _close fork getpid _getpid lockf lstat mkfifo open _open pclose _pclose popen _popen readlink putenv setenv strerror unsetenv)
# Check the form of mkdir()
AC_FUNC_MKDIR
AC_FUNC_SELECT_ARGTYPES

LYX_CHECK_SPELL_ENGINES

lyx_client_subdir=true
dnl AC_LANG_PUSH(C)
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
dnl AC_LANG_POP(C)

lyx_win_res=false;
case ${host} in
    *mingw*|*cygwin*) lyx_win_res=true;;
esac
AM_CONDITIONAL(LYX_WIN_RESOURCE, $lyx_win_res)
LYX_SET_VERSION_INFO


### Some information on what just happened
real_bindir=`eval "echo \`eval \"echo ${bindir}\"\`"`
real_pkgdatadir=`eval "echo \`eval \"echo \\\`eval \\\"echo ${pkgdatadir}\\\"\\\`\"\`"`
real_localedir=`eval "echo \`eval \"echo ${datadir}/locale\"\`"`
VERSION_INFO="Configuration\n\
  Host type:                    ${host}\n\
  Special build flags:         ${lyx_flags}\n\
  C++ Compiler:                 ${CXX} ${CXX_VERSION}\n\
  C++ Compiler LyX flags:       ${AM_CPPFLAGS} ${AM_CXXFLAGS}\n\
  C++ Compiler flags:           ${CPPFLAGS} ${CXXFLAGS}\n\
  Linker flags:                 ${AM_LDFLAGS}\n\
  Linker user flags:            ${LDFLAGS}\n\
${FRONTEND_INFO}\
  Packaging:                    ${lyx_use_packaging}\n\
  LyX binary dir:               ${real_bindir}\n\
  LyX files dir:                ${real_pkgdatadir}\n"

AC_SUBST(VERSION_INFO)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(real_pkgdatadir)

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
#define BOOST_SIGNALS_NO_DEPRECATION_WARNING 1

// TR1 regex not supported in GCC <= 4.5
#ifndef LYX_USE_TR1
#  if __GNUC__ == 4 
#    define LYX_USE_TR1
#  endif
#endif

#ifdef __CYGWIN__
#  define NOMINMAX
#  define BOOST_POSIX 1
#  define BOOST_POSIX_API 1
#  define BOOST_POSIX_PATH 1
#endif

#ifdef __sparc__
#  ifndef __BIG_ENDIAN__
#    define __BIG_ENDIAN__ 1
#  endif
#endif

#if defined(HAVE_WCHAR_T) && SIZEOF_WCHAR_T == 4
#  define USE_WCHAR_T
#endif

#endif
])

MSYS_AC_CANONICAL_PATH(lyx_abs_top_srcdir, ${srcdir})
MSYS_AC_CANONICAL_PATH(lyx_abs_installed_localedir, ${real_localedir})
MSYS_AC_CANONICAL_PATH(lyx_abs_installed_datadir, ${real_pkgdatadir})

AC_DEFINE_UNQUOTED([LYX_ABS_TOP_SRCDIR],
	"${lyx_abs_top_srcdir}", [Top source directory])
AC_DEFINE_UNQUOTED([LYX_ABS_INSTALLED_LOCALEDIR],
	"${lyx_abs_installed_localedir}",[Hard coded locale directory])
AC_DEFINE_UNQUOTED([LYX_ABS_INSTALLED_DATADIR],
	"${lyx_abs_installed_datadir}",[Hard system support directory])
AC_DEFINE_UNQUOTED([PROGRAM_SUFFIX],
	"${version_suffix}",[Program version suffix])

AC_DEFINE_UNQUOTED([LYX_DATE],"$LYX_DATE",[Date of release])
AC_DEFINE_UNQUOTED([LYX_GIT_COMMIT_HASH], "$lyxgitcommit", [Git commit])
AC_DEFINE_UNQUOTED([VERSION_INFO],"$VERSION_INFO",[Full version info])
AC_DEFINE_UNQUOTED([LYX_DIR_VER],"$lyx_dir_ver",[Versioned env var for system dir])
AC_DEFINE_UNQUOTED([LYX_USERDIR_VER],"$lyx_userdir_ver",[Versioned env var for user dir])
AC_DEFINE_UNQUOTED([LYX_MAJOR_VERSION],$lyx_major,[Major version number])
AC_DEFINE_UNQUOTED([LYX_MINOR_VERSION],$lyx_minor,[Minor version number])
AC_DEFINE_UNQUOTED([LYX_RELEASE_LEVEL],$lyx_release,[Release version number])
AC_DEFINE_UNQUOTED([LYX_RELEASE_PATCH],$lyx_patch,[Patch version number])

AC_CONFIG_FILES([Makefile \
      lyx.1:lyx.1in \
      boost/Makefile \
      config/Makefile \
      development/Makefile \
      development/MacOSX/Makefile \
      development/MacOSX/Info.plist \
      development/MacOSX/lyxrc.dist \
      development/MacOSX/spotlight/Makefile \
      development/cygwin/Makefile \
      development/cygwin/lyxrc.dist \
      development/lyx.spec \
      lib/lyx.desktop-temp:lib/lyx.desktop.in
      lib/Makefile \
      lib/doc/Makefile \
      lib/lyx2lyx/lyx2lyx_version.py \
      lib/lyx2lyx/Makefile \
      m4/Makefile \
      po/Makefile.in \
      sourcedoc/Doxyfile \
      sourcedoc/Makefile \
      src/client/Makefile \
      src/client/lyxclient.1:src/client/lyxclient.1in \
      src/Makefile \
      src/tex2lyx/Makefile \
      src/tex2lyx/tex2lyx.1:src/tex2lyx/tex2lyx.1in \
      src/support/Makefile \
      src/frontends/Makefile \
      src/frontends/qt4/Makefile
])


AC_OUTPUT

# show version information
echo
printf "$VERSION_INFO"
echo

# Display a final warning if there has been a LYX_WARNING
LYX_CHECK_WARNINGS

cat <<EOF
Configuration of LyX was successful.
Type 'make' to compile the program,
and then 'make install' to install it.
EOF
