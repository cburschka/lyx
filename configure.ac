dnl Process with autoconf to generate configure script   -*- sh -*-

AC_INIT(lyx,1.4.0cvs,lyx-devel@lists.lyx.org)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(src/main.C)
AM_CONFIG_HEADER([src/config.h])

AC_CONFIG_AUX_DIR(config)

PACKAGE=lyx${program_suffix}
VERSION="1.4.0cvs"
LYX_CHECK_VERSION

dnl default maintainer mode to true for development versions
if test "${enable_maintainer_mode+set}" != set; then
  enable_maintainer_mode=$lyx_devel_version
fi
AM_MAINTAINER_MODE
AC_CANONICAL_TARGET

LYX_VERSION_SUFFIX

AM_INIT_AUTOMAKE($lyxname, $VERSION)

# The list of languages known to LyX
# This is needed by GNU gettext
ALL_LINGUAS="bg ca cs da de es eu fi fr he hu it nl nn no pl pt ro ru sk sl sv tr wa"

# fix the value of the prefixes.
test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

### Set the execute permissions of the various scripts correctly
for file in config/install-sh config/mkinstalldirs lib/configure ; do
  chmod 755 ${srcdir}/${file}
done

### Check for programs
AC_PROG_MAKE_SET
AC_PROG_INSTALL
#AC_PROG_RANLIB
AC_CHECK_PROG(KPSEWHICH, kpsewhich, kpsewhich, :)
if test "x$KPSEWHICH" = xkpsewhich ; then
    AC_DEFINE(HAVE_KPSEWHICH, 1,
    [Define this if you have the kpsewhich program working on your system.])
fi
AC_CHECK_PROGS(M4, gm4 gnum4 m4, m4)

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
LYX_CXX_CHEADERS
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
   lyx_flags="$lyx_flags assertions"
   AC_DEFINE(ENABLE_ASSERTIONS,1,
    [Define if you want assertions to be enabled in the code])
fi

### Library Files
dnl by testing these we check if it is ok to have
dnl -lc and -lm as args to the compiler
AC_CHECK_LIB(m, sin)
AC_CHECK_LIB(c, fopen)

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

AC_ARG_WITH(aiksaurus,
  [  --without-aiksaurus     do not use the Aiksaurus library],
  [lyx_use_aiksaurus=$withval])
if test x$lyx_use_aiksaurus != xno; then
AC_CHECK_LIB(Aiksaurus, main,
	[AC_DEFINE(HAVE_LIBAIKSAURUS,1,[Define this if you have the AikSaurus library])
	 AIKSAURUS_LIBS="-lAiksaurus -lbz2"
	],,"-lbz2")
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

### Setup libtool
AC_DISABLE_SHARED
AC_LIBTOOL_WIN32_DLL
#AM_PROG_LIBTOOL
LYX_PROG_LIBTOOL

### Check if we want spell libraries, prefer new aspell
CHECK_WITH_ASPELL
if test "$USING_ASPELL" != "yes"; then
	CHECK_WITH_PSPELL
fi

### Check for some Cygwin-specific details.
CHECK_WITH_CYGWIN

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

dnl if the gtk frontend has been specified, then xforms is needed too
if echo $FRONTENDS | grep gtk | grep -v xforms ; then
  FRONTENDS="$FRONTENDS xforms"
fi

dnl The code below is not in a macro, because this would cause big
dnl problems with the AC_REQUIRE contained in QT_DO_IT_ALL.
for frontend in $FRONTENDS ; do
  case "$frontend" in
    xforms)
	  XFORMS_DO_IT_ALL
	  FRONTENDS_PROGS="$FRONTENDS_PROGS lyx-xforms\$(EXEEXT)"
	  FRONTENDS_SUBDIRS="$FRONTENDS_SUBDIRS xforms"
	  RPM_FRONTEND="xforms"
	  RPM_FRONTEND_DEPS='libforms >= 1.0'
	  FRONTEND_INFO="${FRONTEND_INFO}\
  XForms Frontend:\n\
      libXpm version:\t\t${XPM_VERSION}\n\
      libforms version:\t\t${XFORMS_VERSION}\n"
	  ;;
  dnl   gnome)
  dnl     XFORMS_DO_IT_ALL
  dnl     PKG_CHECK_MODULES(GNOME_FRONTEND, gtkmm-2.0 libglademm-2.0)
  dnl     AC_SUBST(GNOME_FRONTEND_CFLAGS)
  dnl     AC_SUBST(GNOME_FRONTEND_LIBS)

  dnl     FRONTEND="xforms gnome"
  dnl     FRONTEND_GUILIB="gnome/*.lo"
  dnl     FRONTEND_LDFLAGS="${GNOME_FRONTEND_LIBS}"
  dnl     FRONTEND_INCLUDES="${GNOME_FRONTEND_CFLAGS}"
  dnl     FRONTEND_LIBS="@XPM_LIB@ @XFORMS_LIB@ ${GNOME_FRONTEND_LIBS}"
  dnl	;;
    gtk)
	  PKG_CHECK_MODULES(GTK_FRONTEND, gtkmm-2.4 libglademm-2.4)
	  FRONTENDS_PROGS="$FRONTENDS_PROGS lyx-gtk\$(EXEEXT)"
	  FRONTENDS_SUBDIRS="$FRONTENDS_SUBDIRS gtk"
	  RPM_FRONTEND="gtk"
	  RPM_FRONTEND_DEPS='gtkmm >= 2.4.0'
	  GTKMM_VERSION=`pkg-config --modversion gtkmm-2.4`
	  LIBGLADEMM_VERSION=`pkg-config --modversion libglademm-2.4`
	  FRONTEND_INFO="${FRONTEND_INFO}\
  GTK Frontend:\n\
    libgtkmm version:\t\t${GTKMM_VERSION}\n\
    libglademm version:\t\t${LIBGLADEMM_VERSION}\n"
	  ;;

    qt)
	  QT_DO_IT_ALL
	  FRONTENDS_PROGS="$FRONTENDS_PROGS lyx-qt\$(EXEEXT)"
	  FRONTENDS_SUBDIRS="$FRONTENDS_SUBDIRS qt2"
	  RPM_FRONTEND="qt"
	  RPM_FRONTEND_DEPS='qt >= 2.2.1'
	  FRONTEND_INFO="${FRONTEND_INFO}\
  Qt Frontend:\n\
      Qt version:\t\t${QT_VERSION}\n"
dnl qt build will fail without moc or uic
	  if test -z "$MOC"; then
	    LYX_ERROR([moc binary not found !])
	  fi
	  if test -z "$UIC"; then
	    LYX_ERROR([uic binary not found !])
	  fi
      ;;
    *)
	  LYX_ERROR(Unknown frontend '$frontend');;
  esac
done

### Setup GNU gettext
dnl GNU gettext is written in C
AC_LANG_PUSH(C)
# Some tests that may be useful for gettext
AC_C_CONST
AC_C_INLINE
# Do the real setup now
AM_GNU_GETTEXT
# a hack for those who try to change LyX, but do not have gettext installed
case  "${XGETTEXT}" in
 *:) XGETTEXT='cp ${srcdir}/lyx.pot ./${PACKAGE}.po ; :'
esac

AC_SUBST(LINGUAS)

AC_LANG_POP(C)

# some standard header files
AC_HEADER_DIRENT
AC_HEADER_MAJOR
AC_CHECK_HEADERS(sys/time.h sys/types.h sys/select.h strings.h locale.h)

# some standard structures
AC_HEADER_STAT
AC_HEADER_TIME

# some standard types
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_UID_T

AC_CHECK_FUNCS(snprintf vsnprintf strerror)
LYX_CHECK_DECL(snprintf, stdio.h)
LYX_CHECK_DECL(vsnprintf, stdio.h)
LYX_CHECK_DECL(istreambuf_iterator, iterator)
LYX_CHECK_DECL(mkstemp,[unistd.h stdlib.h])

AC_ARG_ENABLE(compression-support, AC_HELP_STRING([--enable-compression-support],[Support for compressed files.]),[
    case "${enableval}" in
	yes) use_compression=true ;;
	no) use_compression=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-compression-support) ;;
    esac
],[use_compression=true])
if test $use_compression=true ; then
	AC_CHECK_HEADERS(zlib.h, use_compression=true, use_compression=false)
	AC_CHECK_LIB(z, gzopen,[use_compression=true;LIBS="$LIBS -lz"], use_compression=false)
	if test $use_compression = true ; then
		AC_DEFINE(USE_COMRESSION, 1, [Define as 1 if you want to support compressed files.])
		lyx_flags="$lyx_flags compression"
	fi
fi
AM_CONDITIONAL(USE_COMPRESSION, test x$use_compression = xtrue)
AC_MSG_CHECKING([whether to support compressed files])
AC_MSG_RESULT($use_compression)


dnl This is a slight hack: the tests generated by autoconf 2.52 do not
dnl work correctly because of some conflict with stdlib.h with g++ 2.96
dnl We aim to remove this eventually, since we should test as much as
dnl possible with the compiler which will use the functions (JMarc)
AC_LANG_PUSH(C)
AC_CHECK_FUNCS(memmove memset strchr mkfifo mkstemp mktemp)
AC_LANG_POP(C)

AC_FUNC_SELECT_ARGTYPES

### Some information on what just happened
real_bindir=`eval "echo \`eval \"echo ${bindir}\"\`"`
real_datadir=`eval "echo \`eval \"echo ${datadir}/${PACKAGE}\"\`"`
VERSION_INFO="Configuration\n\
  Host type:                      ${host}\n\
  Special build flags:           ${lyx_flags}\n\
  C   Compiler:                   ${CC}\n\
  C   Compiler flags:             ${CFLAGS}\n\
  C++ Compiler:                   ${CXX} ${CXX_VERSION}\n\
  C++ Compiler flags:             ${CXXFLAGS}\n\
  Linker flags:                   ${LDFLAGS}\n\
${FRONTEND_INFO}\
  LyX binary dir:                 ${real_bindir}\n\
  LyX files dir:                  ${real_datadir}\n"


AC_SUBST(VERSION_INFO)
AC_SUBST(RPM_FRONTEND)
AC_SUBST(RPM_FRONTEND_DEPS)

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

#ifndef HAVE_STRCHR
# define strchr(a,b)    index(a,b)
#endif

#ifndef HAVE_MEMMOVE
# define memmove(a,b,c) bcopy(b,a,c)
#endif

#ifndef HAVE_STRERROR
#if defined(__cplusplus)
extern "C"
#endif
char * strerror(int n);
#endif

#ifdef BROKEN_HEADERS
#include "broken_headers.h"
#endif

#ifdef HAVE_MKSTEMP
#ifndef HAVE_DECL_MKSTEMP
#if defined(__cplusplus)
extern "C"
#endif
int mkstemp(char*);
#endif
#endif

#ifdef __EMX__
#include "support/os2_defines.h"
#endif

#if defined(HAVE_OSTREAM) && defined(HAVE_LOCALE) && defined(HAVE_SSTREAM)
#define USE_BOOST_FORMAT 1
#else
#define USE_BOOST_FORMAT 0
#endif

#define BOOST_USER_CONFIG <config.h>

#if defined(ENABLE_ASSERTIONS)
#define BOOST_ENABLE_ASSERT_HANDLER 1
#else
#define BOOST_DISABLE_ASSERTS 1
#endif

#define BOOST_DISABLE_THREADS 1
#define BOOST_NO_WREGEX 1
#define BOOST_NO_WSTRING 1

#endif
])

### Finish the work.
AC_CONFIG_SUBDIRS(lib lib/reLyX)
AC_CONFIG_FILES([Makefile  m4/Makefile \
       boost/Makefile \
       boost/libs/Makefile \
       boost/libs/filesystem/Makefile \
       boost/libs/filesystem/src/Makefile \
       boost/libs/regex/Makefile \
       boost/libs/regex/src/Makefile \
       boost/libs/signals/Makefile \
       boost/libs/signals/src/Makefile \
       config/Makefile \
       development/Makefile \
       development/lyx.spec \
       lib/Makefile \
       lib/doc/Makefile \
       intl/Makefile \
       po/Makefile.in \
       sourcedoc/Doxyfile \
       sourcedoc/Makefile \
       src/client/Makefile \
       src/Makefile \
       src/version.C-tmp:src/version.C.in \
       src/tex2lyx/Makefile \
       src/mathed/Makefile \
       src/graphics/Makefile \
       src/insets/Makefile \
       src/support/Makefile \
       src/frontends/Makefile \
       src/frontends/controllers/Makefile \
       src/frontends/xforms/Makefile \
       src/frontends/xforms/lyx_forms.h-tmp:src/frontends/xforms/lyx_forms.h.in \
       src/frontends/xforms/lyx_xpm.h-tmp:src/frontends/xforms/lyx_xpm.h.in \
       src/frontends/xforms/forms/Makefile \
       src/frontends/gtk/Makefile \
       src/frontends/gtk/gimages/Makefile \
       src/frontends/gtk/glade/Makefile \
       src/frontends/qt2/Makefile \
       src/frontends/qt2/moc/Makefile \
       src/frontends/qt2/ui/Makefile \
       src/frontends/qt2/ui/moc/Makefile \
       src/frontends/gnome/Makefile \
])

AC_OUTPUT
# show version information
echo
printf "$VERSION_INFO"
echo

# Display a final warning if there has been a LYX_ERROR
LYX_CHECK_ERRORS
