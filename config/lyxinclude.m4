dnl Some useful functions for LyXs configure.in                 -*- sh -*-
dnl Author: Jean-Marc Lasgouttes (Jean-Marc.Lasgouttes@inria.fr)
dnl         Lars Gullik Bjønnes (larsbj@lyx.org)
dnl         Allan Rae (rae@lyx.org)


dnl Usage LYX_CHECK_VERSION   Displays version of LyX being built and
dnl sets variable "build_type"
AC_DEFUN([LYX_CHECK_VERSION],[
echo "configuring LyX version" AC_PACKAGE_VERSION

AC_MSG_CHECKING([for build type])
lyx_devel_version=no
lyx_prerelease=no
AC_ARG_ENABLE(build-type,
  AC_HELP_STRING([--enable-build-type=TYPE],[set build setting according to TYPE=rel(ease), pre(release), dev(elopment), prof(iling), gprof]),
  [case $enableval in
    dev*) build_type=development;;
    pre*) build_type=prerelease;;
    prof*)  build_type=profiling;;
    gprof*) build_type=gprof;;
    rel*) build_type=release;;
    *) AC_MSG_ERROR([bad build type specification \"$enableval\". Please use one of rel(ease), pre(release), dev(elopment), prof(iling), or gprof]);;
   esac],
  [case AC_PACKAGE_VERSION in
    *dev*) build_type=development;;
    *pre*|*alpha*|*beta*|*rc*) build_type=prerelease;;
    *) build_type=release ;;
   esac])
AC_MSG_RESULT([$build_type])
lyx_flags="$lyx_flags build=$build_type"
case $build_type in
    development) lyx_devel_version=yes
                 AC_DEFINE(DEVEL_VERSION, 1, [Define if you are building a development version of LyX])
		 LYX_DATE="not released yet" ;;
    prerelease) lyx_prerelease=yes ;;
esac

AC_SUBST(lyx_devel_version)
])


dnl Define the option to set a LyX version on installed executables and directories
dnl
dnl
AC_DEFUN([LYX_VERSION_SUFFIX],[
AC_MSG_CHECKING([for version suffix])
dnl We need the literal double quotes in the rpm spec file
RPM_VERSION_SUFFIX='""'
AC_ARG_WITH(version-suffix,
  [AC_HELP_STRING([--with-version-suffix@<:@=VERSION@:>@], install lyx files as lyxVERSION (VERSION=-AC_PACKAGE_VERSION))],
  [if test "x$withval" = "xyes";
   then
     withval="-"AC_PACKAGE_VERSION
     ac_configure_args=`echo "$ac_configure_args" | sed "s,--with-version-suffix,--with-version-suffix=$withval,"`
   fi
   AC_SUBST(version_suffix,$withval)
   RPM_VERSION_SUFFIX="--with-version-suffix=$withval"])
AC_SUBST(RPM_VERSION_SUFFIX)
AC_SUBST(program_base_name,"lyx")
AC_MSG_RESULT([$withval])
])


dnl Check whether to configure for Qt5. Default is Qt4.
dnl
AC_DEFUN([LYX_CHECK_QT5],[
AC_MSG_CHECKING([whether Qt5 is requested])
AC_ARG_ENABLE([qt5],
  [  --enable-qt5            use Qt5 for building],
  USE_QT5=$enableval, USE_QT5=no)
AC_MSG_RESULT([$USE_QT5])
AC_SUBST([USE_QT5])
])


dnl Usage: LYX_WARNING(message)  Displays the warning "message" and sets the
dnl flag lyx_warning to yes.
AC_DEFUN([LYX_WARNING],[
lyx_warning_txt="$lyx_warning_txt
== $1
"
lyx_warning=yes])


dnl Usage: LYX_LIB_ERROR(file,library)  Displays an error message indication
dnl  that 'file' cannot be found because 'lib' may be uncorrectly installed.
AC_DEFUN([LYX_LIB_ERROR],[
AC_MSG_ERROR([cannot find $1. Please check that the $2 library
   is correctly installed on your system.])])


dnl Usage: LYX_CHECK_WARNINGS  Displays a warning message if a LYX_WARNING
dnl   has occurred previously.
AC_DEFUN([LYX_CHECK_WARNINGS],[
if test x$lyx_warning = xyes; then
cat <<EOF
=== The following minor problems have been detected by configure.
=== Please check the messages below before running 'make'.
=== (see the section 'Problems' in the INSTALL file)
$lyx_warning_txt

EOF
fi])


dnl LYX_SEARCH_PROG(VARIABLE-NAME,PROGRAMS-LIST,ACTION-IF-FOUND)
dnl
define(LYX_SEARCH_PROG,[dnl
for ac_prog in $2 ; do
# Extract the first word of "$ac_prog", so it can be a program name with args.
  set dummy $ac_prog ; ac_word=$[2]
  if test -z "[$]$1"; then
    IFS="${IFS=	}"; ac_save_ifs="$IFS"; IFS=":"
    for ac_dir in $PATH; do
      test -z "$ac_dir" && ac_dir=.
      if test -f [$ac_dir/$ac_word]; then
	$1="$ac_prog"
	break
      fi
    done
    IFS="$ac_save_ifs"
  fi

  if test -n "[$]$1"; then
    ac_result=yes
  else
    ac_result=no
  fi
  ifelse($3,,,[$3])
  test -n "[$]$1" && break
done
])dnl


dnl Usage: LYX_PROG_CLANG: set CLANG to yes if the compiler is clang.
AC_DEFUN([LYX_PROG_CLANG],
[AC_CACHE_CHECK([whether the compiler is clang],
               [lyx_cv_prog_clang],
[AC_TRY_COMPILE([], [
#ifndef __clang__
	    this is not clang
#endif
],
[lyx_cv_prog_clang=yes ; CLANG=yes], [lyx_cv_prog_clang=no ; CLANG=no])])
])


dnl Usage: LYX_CXX_CXX11: set lyx_use_cxx11 to yes if the compiler implements
dnl the C++11 standard.
AC_DEFUN([LYX_CXX_CXX11],
[AC_CACHE_CHECK([whether the compiler implements C++11],
               [lyx_cv_cxx_cxx11],
 [save_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$AM_CPPFLAGS $CPPFLAGS"
  save_CXXFLAGS=$CXXFLAGS
  CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
  AC_LANG_PUSH(C++)
  AC_TRY_COMPILE([], [
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
	    this is a c++11 compiler
#endif
  ],
  [lyx_cv_cxx_cxx11=no], [lyx_cv_cxx_cxx11=yes ; lyx_flags="$lyx_flags c++11"])
 AC_LANG_POP(C++)
 CXXFLAGS=$save_CXXFLAGS
 CPPFLAGS=$save_CPPFLAGS])
 lyx_use_cxx11=$lyx_cv_cxx_cxx11
])


dnl Usage: LYX_CXX_USE_REGEX(cxx11_flags)
dnl decide whether we want to use std::regex and set the
dnl LYX_USE_STD_REGEX accordingly.
dnl the extra cxx11 flags have to be passed to the preprocessor. They are
dnl not plainly added to AM_CPPFLAGS because then the objc compiler (mac)
dnl would fail.
AC_DEFUN([LYX_CXX_USE_REGEX],
[lyx_std_regex=no
 if test $lyx_use_cxx11 = yes; then
   save_CPPFLAGS=$CPPFLAGS
   # we want to pass -std=c++11 to clang/cpp if necessary
   CPPFLAGS="$AM_CPPFLAGS $1 $CPPFLAGS"
   save_CXXFLAGS=$CXXFLAGS
   CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   AC_LANG_PUSH(C++)
   AC_CHECK_HEADER([regex], [lyx_std_regex=yes], [lyx_std_regex=no])
   AC_LANG_POP(C++)
   CXXFLAGS=$save_CXXFLAGS
   CPPFLAGS=$save_CPPFLAGS
   if test x$GXX = xyes && test $lyx_std_regex = yes ; then
     AC_MSG_CHECKING([for correct regex implementation])
     if test x$CLANG = xno || test $lyx_cv_lib_stdcxx = yes; then
       dnl <regex> in gcc is unusable in versions less than 4.9.0
       dnl see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
       case $gxx_version in
         4.0*|4.1*|4.2*|4.3*|4.4*|4.5*|4.6*|4.7*|4.8*) lyx_std_regex=no ;;
         *) ;;
       esac
     fi
   fi
   AC_MSG_RESULT([$lyx_std_regex])
 fi

 if test $lyx_std_regex = yes ; then
  lyx_flags="$lyx_flags std-regex"
  AC_DEFINE([LYX_USE_STD_REGEX], 1, [define to 1 if std::regex should be preferred to boost::regex])
 fi
 AM_CONDITIONAL([LYX_USE_STD_REGEX], test $lyx_std_regex = yes)
])


dnl Usage: LYX_LIB_STDCXX: set lyx_cv_lib_stdcxx to yes if the STL library is libstdc++.
AC_DEFUN([LYX_LIB_STDCXX],
[AC_CACHE_CHECK([whether STL is libstdc++],
               [lyx_cv_lib_stdcxx],
[AC_TRY_COMPILE([#include<vector>], [
#if ! defined(__GLIBCXX__) && ! defined(__GLIBCPP__)
	    this is not libstdc++
#endif
],
[lyx_cv_lib_stdcxx=yes], [lyx_cv_lib_stdcxx=no])])
])


dnl Usage: LYX_LIB_STDCXX_CXX11_ABI: set lyx_cv_lib_stdcxx_cxx11_abi to yes
dnl        if the STL library is GNU libstdc++ and the C++11 ABI is used.
AC_DEFUN([LYX_LIB_STDCXX_CXX11_ABI],
[AC_CACHE_CHECK([whether STL is libstdc++ using the C++11 ABI],
               [lyx_cv_lib_stdcxx_cxx11_abi],
[AC_TRY_COMPILE([#include<vector>], [
#if ! defined(_GLIBCXX_USE_CXX11_ABI) || ! _GLIBCXX_USE_CXX11_ABI
	    this is not libstdc++ using the C++11 ABI
#endif
],
[lyx_cv_lib_stdcxx_cxx11_abi=yes], [lyx_cv_lib_stdcxx_cxx11_abi=no])])
])


AC_DEFUN([LYX_PROG_CXX],
[AC_REQUIRE([AC_PROG_CXX])
AC_REQUIRE([AC_PROG_CXXCPP])

AC_LANG_PUSH(C++)
LYX_PROG_CLANG
LYX_LIB_STDCXX
LYX_LIB_STDCXX_CXX11_ABI
AC_LANG_POP(C++)

if test $lyx_cv_lib_stdcxx = "yes" ; then
  if test $lyx_cv_lib_stdcxx_cxx11_abi = "yes" ; then
    AC_DEFINE(USE_GLIBCXX_CXX11_ABI, 1, [use GNU libstdc++ with C++11 ABI])
  else
    AC_DEFINE(STD_STRING_USES_COW, 1, [std::string uses copy-on-write])
  fi
else
  if test $lyx_cv_prog_clang = "yes" ; then
    AC_DEFINE(USE_LLVM_LIBCPP, 1, [use libc++ provided by llvm instead of GNU libstdc++])
  fi
fi

### We might want to get or shut warnings.
AC_ARG_ENABLE(warnings,
  AC_HELP_STRING([--enable-warnings],[tell the compiler to display more warnings]),,
  [ if test $lyx_devel_version = yes -o $lyx_prerelease = yes && test $ac_cv_prog_gxx = yes ; then
	enable_warnings=yes;
    else
	enable_warnings=no;
    fi;])
if test x$enable_warnings = xyes ; then
  lyx_flags="$lyx_flags warnings"
fi

### We might want to disable debug
AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug],[enable debug information]),,
  [AS_CASE([$build_type], [rel*], [enable_debug=no], [enable_debug=yes])]
)

AC_ARG_ENABLE(stdlib-debug,
  AC_HELP_STRING([--enable-stdlib-debug],[enable debug mode in the standard library]),,
  [AS_CASE([$build_type], [dev*], [enable_stdlib_debug=yes], 
	  [enable_stdlib_debug=no])]
)

### set up optimization
AC_ARG_ENABLE(optimization,
    AC_HELP_STRING([--enable-optimization[=value]],[enable compiler optimisation]),,
    enable_optimization=yes;)
case $enable_optimization in
    yes)
        if test $lyx_devel_version = yes ; then
            lyx_optim=-O
        else
            lyx_optim=-O2
        fi;;
    no) lyx_optim=;;
    *) lyx_optim=${enable_optimization};;
esac

AC_ARG_ENABLE(cxx11,
  AC_HELP_STRING([--disable-cxx11],[disable C++11 mode (default: enabled for known good compilers)]),,
  enable_cxx11=auto;)

AC_ARG_ENABLE(assertions,
  AC_HELP_STRING([--enable-assertions],[add runtime sanity checks in the program]),,
  [AS_CASE([$build_type], [dev*|pre*], [enable_assertions=yes],
	  [enable_assertions=no])]
)
if test "x$enable_assertions" = xyes ; then
   lyx_flags="$lyx_flags assertions"
   AC_DEFINE(ENABLE_ASSERTIONS,1,
    [Define if you want assertions to be enabled in the code])
fi

# set the compiler options correctly.
if test x$GXX = xyes; then
  dnl clang++ pretends to be g++ 4.2.1; this is not useful
  if test x$CLANG = xno; then
    dnl Useful for global version info
    gxx_version=`${CXX} -dumpversion`
    CXX_VERSION="($gxx_version)"
  else
    gxx_version=clang
  fi

  AM_CXXFLAGS="$lyx_optim"
  if test x$enable_debug = xyes ; then
      AM_CXXFLAGS="-g $AM_CXXFLAGS"
  fi
  if test $build_type = gprof ; then
    AM_CXXFLAGS="$AM_CXXFLAGS -pg"
    AM_LDFLAGS="$AM_LDFLAGS -pg"
  fi
  if test $build_type = profiling ; then
    AM_CXXFLAGS="$AM_CXXFLAGS -fno-omit-frame-pointer"
  fi

  if test x$USE_QT5 = xyes ; then
      AS_CASE([$host], [*mingw*|*cygwin*], [], [AM_CXXFLAGS="-fPIC $AM_CXXFLAGS"])
  fi
  dnl Warnings are for preprocessor too
  if test x$enable_warnings = xyes ; then
      AM_CPPFLAGS="$AM_CPPFLAGS -Wall -Wextra"
  fi
  case $gxx_version in
      2.*|3.*) AC_ERROR([gcc 4.x is required]);;
  esac
  if test x$enable_stdlib_debug = xyes ; then
    dnl FIXME: for clang/libc++, one should define _LIBCPP_DEBUG2=0
    dnl See http://clang-developers.42468.n3.nabble.com/libc-debug-mode-td3336742.html
    case $gxx_version in
      *)
        lyx_flags="$lyx_flags stdlib-debug"
	AC_DEFINE(_GLIBCXX_DEBUG, 1, [libstdc++ debug mode])
	AC_DEFINE(_GLIBCXX_DEBUG_PEDANTIC, 1, [libstdc++ pedantic debug mode])
        ;;
    esac
  fi
  dnl enable_cxx11 can be yes/no/auto.
  dnl By default, it is auto and we enable C++11 when possible
  if test x$enable_cxx11 != xno ; then
    case $gxx_version in
      4.0*|4.1*|4.2*)
         if test x$enable_cxx11 = xyes; then
            AC_ERROR([There is no C++11 support in gcc 4.2 or older])
         fi;;
      4.3*|4.4*|4.5*|4.6*)
        dnl Note that this will define __GXX_EXPERIMENTAL_CXX0X__.
        dnl The source code relies on that.
        cxx11_flags="-std=c++0x";;
      clang)
        dnl presumably all clang versions support c++11.
	dnl the deprecated-register warning is very annoying with Qt4.x right now.
        cxx11_flags="-std=c++11"
        AM_CXXFLAGS="$AM_CXXFLAGS -Wno-deprecated-register";;
      *)
        AS_CASE([$host], [*cygwin*],
                [cxx11_flags="-std=gnu++11"],
                [cxx11_flags="-std=c++11"]);;
    esac
    # cxx11_flags is useful when running preprocessor alone 
    # (see detection of regex).
    AM_CXXFLAGS="$cxx11_flags $AM_CXXFLAGS"
  fi
fi

LYX_CXX_CXX11
if test $lyx_use_cxx11 = yes; then
  if test x$GXX = xyes; then
    dnl We still use auto_ptr, which is obsoleted. Shut off the warnings.
    AM_CXXFLAGS="$AM_CXXFLAGS -Wno-deprecated-declarations"
  fi
fi
LYX_CXX_USE_REGEX([$cxx11_flags])
])

dnl Usage: LYX_USE_INCLUDED_BOOST : select if the included boost should
dnl        be used.
AC_DEFUN([LYX_USE_INCLUDED_BOOST],[
	AC_MSG_CHECKING([whether to use included boost library])
	AC_ARG_WITH(included-boost,
	    [AC_HELP_STRING([--without-included-boost], [do not use the boost lib supplied with LyX, try to find one in the system directories - compilation will abort if nothing suitable is found])],
	    [lyx_cv_with_included_boost=$withval],
	    [lyx_cv_with_included_boost=yes])
	AM_CONDITIONAL(USE_INCLUDED_BOOST, test x$lyx_cv_with_included_boost = xyes)
	AC_MSG_RESULT([$lyx_cv_with_included_boost])
	if test x$lyx_cv_with_included_boost = xyes ; then
	    BOOST_INCLUDES='-I$(top_srcdir)/3rdparty/boost'
	    BOOST_LIBS='$(top_builddir)/3rdparty/boost/liblyxboost.a'
	else
	    AC_LANG_PUSH(C++)
	    save_LIBS=$LIBS

	    AC_MSG_CHECKING([for multithreaded boost libraries])
	    LIBS="$save_LIBS -lboost_signals-mt $LIBTHREAD"
	    AC_LINK_IFELSE(
		[AC_LANG_PROGRAM([#include <boost/signal.hpp>],
			[boost::signal<void ()> s;])],
		[AC_MSG_RESULT([yes])
		 BOOST_MT="-mt"],
		[AC_MSG_RESULT([no])
		 AC_MSG_CHECKING([for plain boost libraries])
		 LIBS="$save_LIBS -lboost_signals"
		 AC_LINK_IFELSE(
		     [AC_LANG_PROGRAM([#include <boost/signal.hpp>],
			     [boost::signal<void ()> s;])],
		     [AC_MSG_RESULT([yes])
		      BOOST_MT=""],
		     [AC_MSG_RESULT([no])
		      AC_MSG_ERROR([cannot find suitable boost library (do not use --without-included-boost)])
		 ])
	    ])
	    LIBS=$save_LIBS
	    AC_LANG_POP(C++)
	    BOOST_INCLUDES=
	    if test $lyx_std_regex = yes ; then
	      BOOST_LIBS="-lboost_signals${BOOST_MT}"
	    else
	      BOOST_LIBS="-lboost_regex${BOOST_MT} -lboost_signals${BOOST_MT}"
	    fi

	    dnl In general, system boost libraries are incompatible with
	    dnl the use of stdlib-debug in libstdc++. See ticket #9736 for
	    dnl details.
	    if test $enable_stdlib_debug = "yes" ; then
		LYX_WARNING([Compiling LyX with stdlib-debug and system boost libraries may lead to
   crashes. Consider using --disable-stdlib-debug or removing
   --without-included-boost.])
	    fi
	fi
	AC_SUBST(BOOST_INCLUDES)
	AC_SUBST(BOOST_LIBS)
])


dnl Usage: LYX_USE_INCLUDED_ZLIB : select if the included zlib should
dnl        be used.
AC_DEFUN([LYX_USE_INCLUDED_ZLIB],[
  AC_MSG_CHECKING([whether to use included zlib library])
  AC_ARG_WITH(included-zlib,
    [AC_HELP_STRING([--without-included-zlib], [do not use the zlib lib supplied with LyX, try to find one in the system directories - compilation will abort if nothing suitable is found])],
    [lyx_cv_with_included_zlib=$withval],
    [lyx_cv_with_included_zlib=no])
  AM_CONDITIONAL(USE_INCLUDED_ZLIB, test x$lyx_cv_with_included_zlib = xyes)
  AC_MSG_RESULT([$lyx_cv_with_included_zlib])
  if test x$lyx_cv_with_included_zlib = xyes ; then
    ZLIB_INCLUDES='-I$(top_srcdir)/3rdparty/zlib/1.2.8 -I$(top_builddir)/3rdparty/zlib'
    ZLIB_LIBS='$(top_builddir)/3rdparty/zlib/liblyxzlib.a'
    mkdir -p 3rdparty/zlib
dnl include standard config.h for HAVE_UNISTD_H
    echo "#include <../../config.h>" > 3rdparty/zlib/zconf.h
dnl prevent clash with system zlib that might be dragged in by other libs
    echo "#define Z_PREFIX 1" >> 3rdparty/zlib/zconf.h
    cat "${srcdir}/3rdparty/zlib/1.2.8/zconf.h.in" >> 3rdparty/zlib/zconf.h
  else
    ZLIB_INCLUDES=
    AC_CHECK_HEADERS(zlib.h,
      [AC_CHECK_LIB(z, gzopen, [ZLIB_LIBS="-lz"], LYX_LIB_ERROR(libz,zlib))],
      [LYX_LIB_ERROR(zlib.h,zlib)])
  fi
  AC_SUBST(ZLIB_INCLUDES)
  AC_SUBST(ZLIB_LIBS)
])


dnl Usage: LYX_CHECK_CALLSTACK_PRINTING: define LYX_CALLSTACK_PRINTING if the
dnl        necessary APIs are available to print callstacks.
AC_DEFUN([LYX_CHECK_CALLSTACK_PRINTING],
[AC_CACHE_CHECK([whether printing callstack is possible],
               [lyx_cv_callstack_printing],
[AC_TRY_COMPILE([
#include <execinfo.h>
#include <cxxabi.h>
], [
	void* array[200];
	size_t size = backtrace(array, 200);
	backtrace_symbols(array, size);
	int status = 0;
	abi::__cxa_demangle("abcd", 0, 0, &status);
],
[lyx_cv_callstack_printing=yes], [lyx_cv_callstack_printing=no])])
if test x"$lyx_cv_callstack_printing" = xyes; then
  AC_DEFINE([LYX_CALLSTACK_PRINTING], 1,
            [Define if callstack can be printed])
fi
])


dnl Usage: LYX_USE_INCLUDED_MYTHES : select if the included MyThes should
dnl        be used.
AC_DEFUN([LYX_USE_INCLUDED_MYTHES],[
	AC_MSG_CHECKING([whether to use included MyThes library])
	AC_ARG_WITH(included-mythes,
	    [AC_HELP_STRING([--without-included-mythes], [do not use the MyThes lib supplied with LyX, try to find one in the system directories - compilation will abort if nothing suitable is found])],
	    [lyx_cv_with_included_mythes=$withval],
	    [lyx_cv_with_included_mythes=yes])
	AM_CONDITIONAL(USE_INCLUDED_MYTHES, test x$lyx_cv_with_included_mythes = xyes)
	AC_MSG_RESULT([$lyx_cv_with_included_mythes])
	if test x$lyx_cv_with_included_mythes != xyes ; then
		AC_LANG_PUSH(C++)
		AC_CHECK_HEADER(mythes.hxx,[ac_cv_header_mythes_h=yes lyx_cv_mythes_h_location="<mythes.hxx>"])
		if test x$ac_cv_header_mythes_h != xyes; then
			AC_CHECK_HEADER(mythes/mythes.hxx,[ac_cv_header_mythes_h=yes lyx_cv_mythes_h_location="<mythes/mythes.hxx>"])
		fi
		AC_CHECK_LIB(mythes, main, [MYTHES_LIBS="-lmythes" lyx_mythes=yes], [lyx_mythes=no])
		if test x$lyx_mythes != xyes; then
			AC_CHECK_LIB(mythes-1.2, main, [MYTHES_LIBS="-lmythes-1.2" lyx_mythes=yes], [lyx_mythes=no])
		fi
		AC_LANG_POP(C++)
		if test x$lyx_mythes != xyes -o x$ac_cv_header_mythes_h != xyes; then
			AC_MSG_ERROR([cannot find suitable MyThes library (do not use --without-included-mythes)])
		fi
		AC_DEFINE(USE_EXTERNAL_MYTHES, 1, [Define as 1 to use an external MyThes library])
		AC_DEFINE_UNQUOTED(MYTHES_H_LOCATION,$lyx_cv_mythes_h_location,[Location of mythes.hxx])
		AC_SUBST(MYTHES_LIBS)
	fi
])


dnl Usage: LYX_WITH_DIR(dir-name,desc,dir-var-name,default-value,
dnl                       [default-yes-value])
dnl  Adds a --with-'dir-name' option (described by 'desc') and puts the
dnl  resulting directory name in 'dir-var-name'.
AC_DEFUN([LYX_WITH_DIR],[
  AC_ARG_WITH($1,[AC_HELP_STRING([--with-$1],[specify $2])])
  AC_MSG_CHECKING([for $2])
  if test -z "$with_$3"; then
     AC_CACHE_VAL(lyx_cv_$3, lyx_cv_$3=$4)
  else
    test "x$with_$3" = xyes && with_$3=$5
    lyx_cv_$3="$with_$3"
  fi
  AC_MSG_RESULT($lyx_cv_$3)])


dnl Usage: LYX_LOOP_DIR(value,action)
dnl Executes action for values of variable `dir' in `values'. `values' can
dnl use ":" as a separator.
AC_DEFUN([LYX_LOOP_DIR],[
IFS="${IFS=	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
for dir in `eval "echo $1"`; do
  if test ! "$dir" = NONE; then
    test ! -d "$dir" && AC_MSG_ERROR([\"$dir\" is not a directory])
    $2
  fi
done
IFS=$ac_save_ifs
])


dnl Usage: LYX_ADD_LIB_DIR(var-name,dir) Adds a -L directive to variable
dnl var-name.
AC_DEFUN([LYX_ADD_LIB_DIR],[
$1="${$1} -L$2"
if test "`(uname) 2>/dev/null`" = SunOS &&
    uname -r | grep '^5' >/dev/null; then
  if test $ac_cv_prog_gxx = yes ; then
    $1="${$1} -Wl[,]-R$2"
  else
    $1="${$1} -R$2"
  fi
fi])


dnl Usage: LYX_ADD_INC_DIR(var-name,dir) Adds a -I directive to variable
dnl var-name.
AC_DEFUN([LYX_ADD_INC_DIR],[$1="${$1} -I$2 "])

### Check for a headers existence and location iff it exists
## This is supposed to be a generalised version of LYX_STL_STRING_FWD
## It almost works.  I've tried a few variations but they give errors
## of one sort or other: bad substitution or file not found etc.  The
## actual header _is_ found though and the cache variable is set however
## the reported setting (on screen) is equal to $ac_safe for some unknown
## reason.
## Additionally, autoheader can't figure out what to use as the name in
## the config.h.in file so we need to write our own entries there -- one for
## each header in the form PATH_HEADER_NAME_H
##
AC_DEFUN([LYX_PATH_HEADER],
[ AC_CHECK_HEADER($1,[
  ac_tr_safe=PATH_`echo $ac_safe | sed 'y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%'`
### the only remaining problem is getting the second parameter to this
### AC_CACHE_CACHE to print correctly. Currently it just results in value
### of $ac_safe being printed.
  AC_CACHE_CHECK([path to $1],[lyx_cv_path2_$ac_safe],
  [ cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"

#include <$1>
EOF
lyx_path_header_path=`(eval "$ac_cpp conftest.$ac_ext") 2>&5 | \
  grep $1  2>/dev/null | \
  sed -e 's/.*\(".*$1"\).*/\1/' -e "1q"`
eval "lyx_cv_path2_${ac_safe}=\$lyx_path_header_path"
rm -f conftest*])
  AC_DEFINE_UNQUOTED($ac_tr_safe, $lyx_path_header_path, [dummy])])
])
### end of LYX_PATH_HEADER

## Check what kind of packaging should be used at install time.
## The default is autodetected.
AC_DEFUN([LYX_USE_PACKAGING],
[AC_MSG_CHECKING([what packaging should be used])
AC_ARG_WITH(packaging,
  [AC_HELP_STRING([--with-packaging=THIS], [use THIS packaging for installation:
			    Possible values: posix, windows, macosx, haiku])],
  [lyx_use_packaging="$withval"], [
  case $host in
    *-apple-darwin*) lyx_use_packaging=macosx ;;
    *-pc-mingw*) lyx_use_packaging=windows ;;
    *haiku*) lyx_use_packaging=haiku ;;
    *) lyx_use_packaging=posix ;;
  esac])
AC_MSG_RESULT($lyx_use_packaging)
lyx_install_posix=false
lyx_install_macosx=false
lyx_install_cygwin=false
lyx_install_windows=false
case $lyx_use_packaging in
   macosx) AC_DEFINE(USE_MACOSX_PACKAGING, 1, [Define to 1 if LyX should use a MacOS X application bundle file layout])
	   PACKAGE=LyX${version_suffix}
	   default_prefix="/Applications/${PACKAGE}.app"
	   AC_SUBST(osx_bundle_program_name,"${program_base_name}")
	   bindir='${prefix}/Contents/MacOS'
	   libdir='${prefix}/Contents/Resources'
	   datarootdir='${prefix}/Contents/Resources'
	   pkgdatadir='${datadir}'
	   mandir='${datadir}/man'
	   lyx_install_macosx=true ;;
  windows) AC_DEFINE(USE_WINDOWS_PACKAGING, 1, [Define to 1 if LyX should use a Windows-style file layout])
	   PACKAGE=LyX${version_suffix}
	   default_prefix="C:/Program Files/${PACKAGE}"
	   bindir='${prefix}/bin'
	   libdir='${prefix}/Resources'
	   datarootdir='${prefix}/Resources'
	   pkgdatadir='${datadir}'
	   mandir='${prefix}/Resources/man'
	   lyx_install_windows=true ;;
    posix) AC_DEFINE(USE_POSIX_PACKAGING, 1, [Define to 1 if LyX should use a POSIX-style file layout])
	   PACKAGE=lyx${version_suffix}
	   program_suffix=$version_suffix
	   pkgdatadir='${datadir}/${PACKAGE}'
	   default_prefix=$ac_default_prefix
	   case ${host} in
	     *cygwin*) lyx_install_cygwin=true ;;
	     *apple-darwin*) lyx_install_macosx=true ;;
	   esac
	   lyx_install_posix=true ;;
	haiku) AC_DEFINE(USE_HAIKU_PACKAGING, 1, [Define to 1 if LyX should use a Haiku-style file layout])
	   PACKAGE=lyx${version_suffix}
	   program_suffix=$version_suffix
	   pkgdatadir='${datadir}/${PACKAGE}'
	   default_prefix=$ac_default_prefix
	   lyx_install_posix=true ;;
    *) AC_MSG_ERROR([unknown packaging type $lyx_use_packaging.]) ;;
esac
AM_CONDITIONAL(INSTALL_MACOSX, $lyx_install_macosx)
AM_CONDITIONAL(INSTALL_WINDOWS, $lyx_install_windows)
AM_CONDITIONAL(INSTALL_CYGWIN, $lyx_install_cygwin)
AM_CONDITIONAL(INSTALL_POSIX, $lyx_install_posix)
AC_SUBST(pkgdatadir)
AC_SUBST(program_suffix)
])


## ------------------------------------------------------------------------
## Check whether mkdir() is mkdir or _mkdir, and whether it takes
## one or two arguments.
##
## http://ac-archive.sourceforge.net/C_Support/ac_func_mkdir.html
## ------------------------------------------------------------------------
##
AC_DEFUN([AC_FUNC_MKDIR],
[AC_CHECK_FUNCS([mkdir _mkdir])
AC_CACHE_CHECK([whether mkdir takes one argument],
               [ac_cv_mkdir_takes_one_arg],
[AC_TRY_COMPILE([
#include <sys/stat.h>
#if HAVE_UNISTD_H
#  include <unistd.h>
#endif
], [mkdir (".");],
[ac_cv_mkdir_takes_one_arg=yes], [ac_cv_mkdir_takes_one_arg=no])])
if test x"$ac_cv_mkdir_takes_one_arg" = xyes; then
  AC_DEFINE([MKDIR_TAKES_ONE_ARG], 1,
            [Define if mkdir takes only one argument.])
fi
])


dnl Set VAR to the canonically resolved absolute equivalent of PATHNAME,
dnl (which may be a relative path, and need not refer to any existing
dnl entity).

dnl On Win32-MSYS build hosts, the returned path is resolved to its true
dnl native Win32 path name, (but with slashes, not backslashes).

dnl On any other system, it is simply the result which would be obtained
dnl if PATHNAME represented an existing directory, and the pwd command was
dnl executed in that directory.
AC_DEFUN([MSYS_AC_CANONICAL_PATH],
[ac_dir="$2"
 ( exec 2>/dev/null; cd / && pwd -W ) | grep ':' >/dev/null &&
    ac_pwd_w="pwd -W" || ac_pwd_w=pwd
 until ac_val=`exec 2>/dev/null; cd "$ac_dir" && $ac_pwd_w`
 do
   ac_dir=`AS_DIRNAME(["$ac_dir"])`
 done
 ac_dir=`echo "$ac_dir" | sed 's?^[[./]]*??'`
 ac_val=`echo "$ac_val" | sed 's?/*$[]??'`
 $1=`echo "$2" | sed "s?^[[./]]*$ac_dir/*?$ac_val/?"'
   s?/*$[]??'`
])

dnl this is used by the macro blow to general a proper config.h.in entry
m4_define([LYX_AH_CHECK_DECL],
[AH_TEMPLATE(AS_TR_CPP(HAVE_DECL_$1),
  [Define if you have the prototype for function `$1'])])

dnl Check things are declared in headers to avoid errors or warnings.
dnl Called like LYX_CHECK_DECL(function, header1 header2...)
dnl Defines HAVE_DECL_{FUNCTION}
AC_DEFUN([LYX_CHECK_DECL],
[LYX_AH_CHECK_DECL($1)
for ac_header in $2
do
  AC_MSG_CHECKING([if $1 is declared by header $ac_header])
  AC_EGREP_HEADER($1, $ac_header,
      [AC_MSG_RESULT(yes)
       AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_DECL_$1))
       break],
      [AC_MSG_RESULT(no)])
done])

dnl this is used by the macro below to generate a proper config.h.in entry
m4_define([LYX_AH_CHECK_DEF],
[AH_TEMPLATE(AS_TR_CPP(HAVE_DEF_$1),
  [Define to 1 if `$1' is defined in `$2'])])

dnl Check whether name is defined in header by using it in codesnippet.
dnl Called like LYX_CHECK_DEF(name, header, codesnippet)
dnl Defines HAVE_DEF_{NAME}
AC_DEFUN([LYX_CHECK_DEF],
[LYX_AH_CHECK_DEF($1, $2)
 AC_MSG_CHECKING([whether $1 is defined by header $2])
 AC_TRY_COMPILE([#include <$2>], [$3],
     lyx_have_def_name=yes,
     lyx_have_def_name=no)
 AC_MSG_RESULT($lyx_have_def_name)
 if test "x$lyx_have_def_name" = xyes; then
   AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_DEF_$1))
 fi
])

dnl Extract the single digits from PACKAGE_VERSION and make them available.
dnl Defines LYX_MAJOR_VERSION, LYX_MINOR_VERSION, LYX_RELEASE_LEVEL,
dnl LYX_RELEASE_PATCH (possibly equal to 0), LYX_DIR_VER, and LYX_USERDIR_VER.
AC_DEFUN([LYX_SET_VERSION_INFO],
[lyx_major=`echo $PACKAGE_VERSION | sed -e 's/[[.]].*//'`
 lyx_patch=`echo $PACKAGE_VERSION | sed -e "s/^$lyx_major//" -e 's/^.//'`
 lyx_minor=`echo $lyx_patch | sed -e 's/[[.]].*//'`
 lyx_patch=`echo $lyx_patch | sed -e "s/^$lyx_minor//" -e 's/^.//'`
 lyx_release=`echo $lyx_patch | sed -e 's/[[^0-9]].*//'`
 lyx_patch=`echo $lyx_patch | sed -e "s/^$lyx_release//" -e 's/^[[.]]//' -e 's/[[^0-9]].*//'`
 test "x$lyx_patch" = "x" && lyx_patch=0
 lyx_dir_ver=LYX_DIR_${lyx_major}${lyx_minor}x
 lyx_userdir_ver=LYX_USERDIR_${lyx_major}${lyx_minor}x
 AC_SUBST(LYX_MAJOR_VERSION,$lyx_major)
 AC_SUBST(LYX_MINOR_VERSION,$lyx_minor)
 AC_SUBST(LYX_RELEASE_LEVEL,$lyx_release)
 AC_SUBST(LYX_RELEASE_PATCH,$lyx_patch)
 AC_SUBST(LYX_DIR_VER,"$lyx_dir_ver")
 AC_SUBST(LYX_USERDIR_VER,"$lyx_userdir_ver")
])

# AM_PYTHON_CHECK_VERSION(PROG, VERSION, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ---------------------------------------------------------------------------
# Copied from autoconf 2.68, added a check that python version is < 3.0
# Run ACTION-IF-TRUE if the Python interpreter PROG has version >= VERSION.
# Run ACTION-IF-FALSE otherwise.
# This test uses sys.hexversion instead of the string equivalent (first
# word of sys.version), in order to cope with versions such as 2.2c1.
# This supports Python 2.0 or higher. (2.0 was released on October 16, 2000).
AC_DEFUN([AM_PYTHON_CHECK_VERSION],
 [prog="import sys
# split strings by '.' and convert to numeric.  Append some zeros
# because we need at least 4 digits for the hex conversion.
# map returns an iterator in Python 3.0 and a list in 2.x
minver = list(map(int, '$2'.split('.'))) + [[0, 0, 0]]
minverhex = 0
maxverhex = 50331648 # = 3.0.0.0
# xrange is not present in Python 3.0 and range returns an iterator
for i in list(range(0, 4)): minverhex = (minverhex << 8) + minver[[i]]
sys.exit(sys.hexversion < minverhex or sys.hexversion >= maxverhex)"
  AS_IF([AM_RUN_LOG([$1 -c "$prog"])], [$3], [$4])])
