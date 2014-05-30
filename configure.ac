dnl Process with autoconf to generate configure script   -*- sh -*-

AC_INIT(LyX,2.0.8.1,[lyx-devel@lists.lyx.org],[lyx])
# Use ISO format only. The frontend needs to parse this
AC_SUBST(LYX_DATE, ["2014-05-30"])
AC_PREREQ(2.52)
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
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-define 1.8])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
PACKAGE=$save_PACKAGE

# Allow to build some parts of the code as one big chunk
m4_define([ALLPARTS],[boost,client,insets,mathed,core,tex2lyx,frontend_qt4])
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
AM_CONDITIONAL(MONOLITHIC_FRONTEND_QT4, test "x$enable_monolithic_frontend_qt4" = "xyes")

### Set the execute permissions of the various scripts correctly
for file in config/install-sh ; do
  chmod 755 ${srcdir}/${file}
done

# Check for installed python
AM_PATH_PYTHON(2.4.0,, :)

### we need to know the byte order for unicode conversions
AC_C_BIGENDIAN

### check which frontend we want to use
LYX_USE_FRONTENDS

### Check for a C++ compiler
LYX_PROG_CXX

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

# Needed for our char_type
AC_CHECK_SIZEOF(wchar_t)

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


### check which frontend we want to use

dnl The code below is not in a macro, because this would cause big
dnl problems with the AC_REQUIRE contained in QT4_DO_IT_ALL.
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
dnl Dirty trick ahead: disable macro AC_GNU_SOURCE because it triggers a bug with autoconf 2.62.
dnl this can be removed if gettext is been updated to avoid that.
m4_undefine([AC_GNU_SOURCE])
m4_defun([AC_GNU_SOURCE],[])
AM_GNU_GETTEXT([no-libtool])
AM_GNU_GETTEXT_VERSION([0.16.1])
AC_LANG_POP(C)

# some standard header files
AC_HEADER_MAJOR
AC_CHECK_HEADERS(sys/time.h sys/types.h sys/select.h locale.h utime.h sys/utime.h)

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

LYX_CHECK_DEF(PATH_MAX, limits.h, [int n = PATH_MAX;])

AC_CHECK_FUNCS(chmod close _close fork getpid _getpid lstat mkfifo open _open pclose _pclose popen _popen readlink strerror unsetenv)
# Check the form of mkdir()
AC_FUNC_MKDIR
AC_FUNC_SELECT_ARGTYPES

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


### Some information on what just happened
real_bindir=`eval "echo \`eval \"echo ${bindir}\"\`"`
real_pkgdatadir=`eval "echo \`eval \"echo \\\`eval \\\"echo ${pkgdatadir}\\\"\\\`\"\`"`
real_localedir=`eval "echo \`eval \"echo ${datadir}/locale\"\`"`
VERSION_INFO="Configuration\n\
  Host type:                    ${host}\n\
  Special build flags:         ${lyx_flags}\n\
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

AC_SUBST(VERSION_INFO)
AC_SUBST(RPM_FRONTEND)
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

/*
 * the FreeBSD libc uses UCS4, but libstdc++ has no proper wchar_t
 * support compiled in:
 * http://gcc.gnu.org/onlinedocs/libstdc++/faq/index.html#3_9
 * And we are not interested at all what libc
 * does: What we need is a 32bit wide wchar_t, and a libstdc++ that
 * has the needed wchar_t support and uses UCS4. Whether it
 * implements this with the help of libc, or whether it has own code
 * does not matter for us, because we do not use libc directly (Georg)
*/
#if defined(HAVE_WCHAR_T) && SIZEOF_WCHAR_T == 4 && !defined(__FreeBSD__) && !defined(__FreeBSD_kernel__)
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

# Display a final warning if there has been a LYX_ERROR
LYX_CHECK_ERRORS
