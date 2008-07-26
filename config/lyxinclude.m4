dnl Some useful functions for LyXs configure.in                 -*- sh -*-
dnl Author: Jean-Marc Lasgouttes (Jean-Marc.Lasgouttes@inria.fr)
dnl         Lars Gullik Bjønnes (larsbj@lyx.org)
dnl         Allan Rae (rae@lyx.org)


dnl Usage LYX_CHECK_VERSION   Displays version of LyX being built and
dnl sets variables "lyx_devel_version" and "lyx_prerelease"
AC_DEFUN([LYX_CHECK_VERSION],[
echo "configuring LyX version" AC_PACKAGE_VERSION
lyx_devel_version=no
lyx_prerelease=no
build_type=release
AC_MSG_CHECKING([for build type])
AC_ARG_ENABLE(build-type,
  AC_HELP_STRING([--enable-build-type=TYPE],[set build setting according to TYPE=dev(elopment), rel(ease) or pre(release)]),
  [case $enableval in
    dev*) lyx_devel_version=yes
          build_type=development;;
    pre*) lyx_prerelease=yes
          build_type=prerelease;;
    rel*) ;;
    *) AC_ERROR([Bad build type specification \"$enableval\". Please use one of dev(elopment), rel(ease) or pre(release)]);;
   esac],
  [case AC_PACKAGE_VERSION in
    *svn*) lyx_devel_version=yes
          build_type=development;;
    *pre*|*alpha*|*beta*|*rc*) lyx_prerelease=yes
          build_type=prerelease;;
   esac])
AC_MSG_RESULT([$build_type])
if test $lyx_devel_version == yes ; then
           AC_DEFINE(DEVEL_VERSION, 1, [Define if you are building a development version of LyX])
           LYX_DATE="not released yet"
fi
AC_SUBST(lyx_devel_version)])


dnl Define the option to set a LyX version on installed executables and directories
dnl
dnl
AC_DEFUN([LYX_VERSION_SUFFIX],[
AC_MSG_CHECKING([for version suffix])
dnl We need the literal double quotes in the rpm spec file
RPM_VERSION_SUFFIX='""'
AC_ARG_WITH(version-suffix,
  [  --with-version-suffix[=<version>]  install lyx files as lyx<version>],
  [if test "x$withval" = "xyes";
   then
     withval="-"AC_PACKAGE_VERSION
     ac_configure_args=`echo $ac_configure_args | sed "s,--with-version-suffix,--with-version-suffix=$withval,"`
   fi
   AC_SUBST(version_suffix,$withval)
   RPM_VERSION_SUFFIX="--with-version-suffix=$withval"])
AC_SUBST(RPM_VERSION_SUFFIX)
AC_MSG_RESULT([$withval])
])


dnl Usage: LYX_ERROR(message)  Displays the warning "message" and sets the
dnl flag lyx_error to yes.
AC_DEFUN([LYX_ERROR],[
lyx_error_txt="$lyx_error_txt
** $1
"
lyx_error=yes])


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
LYX_ERROR([Cannot find $1. Please check that the $2 library
   is correctly installed on your system.])])


dnl Usage: LYX_CHECK_ERRORS  Displays a warning message if a LYX_ERROR
dnl   has occured previously.
AC_DEFUN([LYX_CHECK_ERRORS],[
if test x$lyx_warning = xyes; then
cat <<EOF
=== The following minor problems have been detected by configure.
=== Please check the messages below before running 'make'.
=== (see the section 'Problems' in the INSTALL file)
$lyx_warning_txt
EOF
fi
if test x$lyx_error = xyes; then
cat <<EOF
**** The following problems have been detected by configure.
**** Please check the messages below before running 'make'.
**** (see the section 'Problems' in the INSTALL file)
$lyx_error_txt
$lyx_warning_txt
EOF
exit 1
else

cat <<EOF
Configuration of LyX was successful.
Type 'make' to compile the program,
and then 'make install' to install it.
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


AC_DEFUN([LYX_PROG_CXX_WORKS],
[rm -f conftest.C
cat >conftest.C <<EOF
class foo {
   // we require the mutable keyword
   mutable int bar;
 };
 // we require namespace support
 namespace baz {
   int bar;
 }
 int main() {
   return(0);
 }
EOF
$CXX -c $CXXFLAGS $CPPFLAGS conftest.C >&5 || CXX=
rm -f conftest.C conftest.o conftest.obj || true
])


AC_DEFUN([LYX_PROG_CXX],
[AC_MSG_CHECKING([for a good enough C++ compiler])
LYX_SEARCH_PROG(CXX, $CXX $CCC g++ gcc c++ CC cxx xlC cc++, [LYX_PROG_CXX_WORKS])

if test -z "$CXX" ; then
  AC_MSG_ERROR([Unable to find a good enough C++ compiler])
fi
AC_MSG_RESULT($CXX)

AC_PROG_CXX
AC_PROG_CXXCPP

### We might want to get or shut warnings.
AC_ARG_ENABLE(warnings,
  AC_HELP_STRING([--enable-warnings],[tell the compiler to display more warnings]),,
  [ if test $lyx_devel_version = yes -o $lyx_prerelease = yes && test $ac_cv_prog_gxx = yes ; then
	enable_warnings=yes;
    else
	enable_warnings=no;
    fi;])
if test x$enable_warnings = xyes ; then
  lyx_flags="warnings $lyx_flags"
fi

### We might want to disable debug
AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug],[enable debug information]),,
  [ if test $lyx_devel_version = yes -o $lyx_prerelease = yes ; then
	enable_debug=yes;
    else
	enable_debug=no;
    fi;])

AC_ARG_ENABLE(stdlib-debug,
  AC_HELP_STRING([--enable-stdlib-debug],[enable debug mode in the standard library]),,
  [ if test $lyx_devel_version = yes ; then
      enable_stdlib_debug=yes;
    else
      enable_stdlib_debug=no;
    fi;])

AC_ARG_ENABLE(concept-checks,
  AC_HELP_STRING([--enable-concept-checks],[enable concept checks]),,
  [ if test $lyx_devel_version = yes -o $lyx_prerelease = yes ; then
	enable_concept_checks=yes;
    else
        enable_concept_checks=no;
    fi;])

AC_ARG_ENABLE(profiling,
  AC_HELP_STRING([--enable-profiling],[enable profiling]),,
  enable_profiling=no;)

### set up optimization
AC_ARG_ENABLE(optimization,
    AC_HELP_STRING([--enable-optimization[=value]],[enable compiler optimisation]),,
    enable_optimization=yes;)
case $enable_optimization in
    yes)
        if test $lyx_devel_version = yes ; then
            lyx_opt=-O
        else
            lyx_opt=-O2
        fi;;
    no) lyx_opt=;;
    *) lyx_opt=${enable_optimization};;
esac

AC_ARG_ENABLE(pch,
  AC_HELP_STRING([--enable-pch],[enable precompiled headers]),,
	enable_pch=no;)
lyx_pch_comp=no

# set the compiler options correctly.
if test x$GXX = xyes; then
  dnl Useful for global version info
  gxx_version=`${CXX} -dumpversion`
  CXX_VERSION="($gxx_version)"

  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  else
      CFLAGS="$lyx_opt"
      CXXFLAGS="$lyx_opt"
    if test x$enable_debug = xyes ; then
        CFLAGS="-g $CFLAGS"
	CXXFLAGS="-g $CXXFLAGS"
    fi
    if test x$enable_profiling = xyes ; then
        CFLAGS="-pg $CFLAGS"
        CXXFLAGS="-pg $CXXFLAGS"
        LDFLAGS="-pg $LDFLAGS"
    fi
  fi
  if test "$ac_env_CPPFLAGS_set" != set; then
    if test x$enable_warnings = xyes ; then
        case $gxx_version in
            3.1*|3.2*|3.3*)
                CPPFLAGS="-W -Wall $CPPFLAGS"
                ;;
            *)
                CPPFLAGS="-Wextra -Wall $CPPFLAGS "
                ;;
        esac
    fi
  fi
  case $gxx_version in
      3.1*)    AM_CXXFLAGS="-finline-limit=500 ";;
      3.2*|3.3*)    AM_CXXFLAGS="";;
      3.4*|4.*)
          AM_CXXFLAGS=""
          test $enable_pch = yes && lyx_pch_comp=yes
          ;;
      *)       AM_CXXFLAGS="";;
  esac
  if test x$enable_stdlib_debug = xyes ; then
    case $gxx_version in
      3.4*|4.*)
        lyx_flags="stdlib-debug $lyx_flags"
	AC_DEFINE(_GLIBCXX_DEBUG, 1, [libstdc++ debug mode])
	AC_DEFINE(_GLIBCXX_DEBUG_PEDANTIC, 1, [libstdc++ pedantic debug mode])
        ;;
    esac
  fi
  if test x$enable_concept_checks = xyes ; then
    case $gxx_version in
      3.3*)
        lyx_flags="concept-checks $lyx_flags"
        AC_DEFINE(_GLIBCPP_CONCEPT_CHECKS, 1, [libstdc++ concept checking])
	;;
      3.4*|4.*)
        lyx_flags="concept-checks $lyx_flags"
	AC_DEFINE(_GLIBCXX_CONCEPT_CHECKS, 1, [libstdc++ concept checking])
	;;
    esac
  fi
fi
test "$lyx_pch_comp" = yes && lyx_flags="pch $lyx_flags"
AM_CONDITIONAL(LYX_BUILD_PCH, test "$lyx_pch_comp" = yes)
])dnl


dnl NOT USED CURRENTLY*************************************
dnl Usage: LYX_CXX_RTTI : checks whether the C++ compiler
dnl   supports RTTI
AC_DEFUN([LYX_CXX_RTTI],[
### Check whether the compiler supports runtime type information
AC_CACHE_CHECK(whether the C++ compiler supports RTTI,lyx_cv_rtti,
 [AC_TRY_RUN([
class rtti {
public:
   virtual int tag() { return 0; }
};
class derived1 : public rtti {
public:
    int tag() { return 1; }
};
class derived2 : public rtti {
public:
    int tag() { return 2; }
};
int main() {
    derived1 * foo1 = new derived1();
    derived2 * foo2 = new derived2();
    rtti * bar = foo1;
    derived1 * bar1 = dynamic_cast<derived1 *>(bar);
    if (bar1 == 0)
	exit(1);
    bar = foo2;
    bar1 = dynamic_cast<derived1 *>(bar);
    if (bar1 != 0)
	exit(1);
    return 0;
}
],lyx_cv_rtti=yes,lyx_cv_rtti=no,lyx_cv_rtti=no)
])
if test x$lyx_cv_rtti = xyes ; then
  AC_DEFINE(HAVE_RTTI, 1,
   [Define to 1 if your compiler supports runtime type information])
fi])


dnl Usage: LYX_CXX_EXPLICIT : checks whether the C++ compiler
dnl   understands the "explicit" directive.
AC_DEFUN([LYX_CXX_EXPLICIT],[
### Check whether the compiler understands the keyword `explicit'
AC_CACHE_CHECK(whether the C++ compiler understands explicit,lyx_cv_explicit,
 [AC_TRY_COMPILE([
class Expl {
public:
	explicit Expl() {}
};],,lyx_cv_explicit=yes,lyx_cv_explicit=no)
])
if test $lyx_cv_explicit = no ; then
  AC_DEFINE(explicit,[ ],
   [Define to nothing if your compiler does not understand the
   'explicit' directive])
fi])


dnl NOT USED CURRENTLY*************************************
dnl Usage: LYX_CXX_STL_STACK : checks whether the C++ compiler
dnl   has a working stl stack template
AC_DEFUN([LYX_CXX_STL_STACK],[
AC_CACHE_CHECK(for broken STL stack template,lyx_cv_broken_stack,
 [AC_TRY_COMPILE([
#include <stack>
using std::stack;
],[
    stack<int> stakk;
    stakk.push(0);
],lyx_cv_broken_stack=no,lyx_cv_broken_stack=yes)
])
if test $lyx_cv_broken_stack = yes ; then
  AC_DEFINE(BROKEN_STL_STACK, 1,
   [Define if you have the STL from libg++ 2.7.x, where stack<> is not defined
   correctly])
fi])


dnl Usage: LYX_STD_COUNT : checks wherer the C++ library have a conforming
dnl    count template, if not the old HP version is assumed.
AC_DEFUN([LYX_STD_COUNT],[
AC_CACHE_CHECK(for conforming std::count,lyx_cv_std_count,
 [AC_TRY_COMPILE([
#include <algorithm>
using std::count;
int countChar(char * b, char * e, char const c)
{
	return count(b, e, c);
}
],[
    char a[] = "hello";
    int i = countChar(a, a + 5, 'l');
],lyx_cv_std_count=yes,lyx_cv_std_count=no)
])
if test $lyx_cv_std_count = yes ; then
    AC_DEFINE(HAVE_STD_COUNT, 1,
    [Define if you have a conforming std::count template, otherwise HP version of count template is assumed.])
fi])


dnl Usage: LYX_CXX_STL_MODERN_STREAMS : checks whether the C++ compiler
dnl   supports modern STL streams
AC_DEFUN([LYX_CXX_STL_MODERN_STREAMS],[
AC_CACHE_CHECK(for modern STL streams,lyx_cv_modern_streams,
 [AC_TRY_COMPILE([
#include <iostream>
],[
 std::streambuf * test = std::cerr.rdbuf();
 test->pubsync();
],lyx_cv_modern_streams=yes,lyx_cv_modern_streams=no)
])
if test $lyx_cv_modern_streams = yes ; then
  AC_DEFINE(MODERN_STL_STREAMS, 1,
   [Define if you have modern standard-compliant STL streams])
fi])


dnl Usage: LYX_USE_INCLUDED_BOOST : select if the included boost should
dnl        be used.
AC_DEFUN([LYX_USE_INCLUDED_BOOST],[
	AC_MSG_CHECKING([whether to use boost included library])
	AC_ARG_WITH(included-boost,
	    [  --without-included-boost  do not use the boost lib supplied with LyX, try to find one in the system directories - compilation will abort if nothing suitable is found],
	    [lyx_cv_with_included_boost=$withval],
	    [lyx_cv_with_included_boost=yes])
	AM_CONDITIONAL(USE_INCLUDED_BOOST, test x$lyx_cv_with_included_boost = xyes)
	AC_MSG_RESULT([$lyx_cv_with_included_boost])
])


dnl NOT USED CURRENTLY*************************************
dnl LYX_CXX_PARTIAL
AC_DEFUN([LYX_CXX_PARTIAL], [
    AC_REQUIRE([AC_PROG_CXX])
    AC_CACHE_CHECK([if C++ compiler supports partial specialization],
	[lyx_cv_cxx_partial_specialization],
	[AC_TRY_COMPILE(
	    [
	    template<class T, class K>
	    class k {
	    public:
	    };
	    template<class T> class k<void,T> { };
	    ],[
	    k<float, float> b;
	    k<void,void> a;
	    ],[
	    lyx_cv_cxx_partial_specialization=yes
	    ],[
	    lyx_cv_cxx_partial_specialization=no
	    ])
	])
    if test x$lyx_cv_cxx_partial_specialization = xyes ; then
	AC_DEFINE(HAVE_PARTIAL_SPECIALIZATION, 1,
	[Defined if your compiler supports partial specialization.])
    fi
])


dnl Usage: LYX_CXX_GLOBAL_CSTD: checks whether C library functions
dnl   are already in the global namespace
AC_DEFUN([LYX_CXX_GLOBAL_CSTD],[
    AC_CACHE_CHECK(whether C library functions are already in the global namespace,
    lyx_cv_cxx_global_cstd,
    [AC_TRY_COMPILE([
    #include <cctype>
    using std::tolower;
    ],[
    return 0;
    ],[lyx_cv_cxx_global_cstd=no],[lyx_cv_cxx_global_cstd=yes])])
    if test x$lyx_cv_cxx_global_cstd = xyes; then
	AC_DEFINE(CXX_GLOBAL_CSTD,1,
	[Define if your C++ compiler puts C library functions in the global namespace])
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

### Check which frontends we want to use.
###
AC_DEFUN([LYX_USE_FRONTENDS],
[AC_MSG_CHECKING([what frontend should be used for the GUI])
AC_ARG_WITH(frontend,
  [  --with-frontend=THIS    Use THIS frontend as main GUI:
			    Possible values: qt4],
  [FRONTENDS="$withval"],[FRONTENDS="qt4"])
if test "x$FRONTENDS" = x ; then
  AC_MSG_RESULT(none)
  AC_ERROR("Please select a frontend using --with-frontend")
fi
AC_MSG_RESULT($FRONTENDS)
AC_SUBST(FRONTENDS)
AC_SUBST(FRONTENDS_SUBDIRS)
AC_SUBST(FRONTENDS_PROGS)
])


## Check what kind of packaging should be used at install time.
## The default is autodetected.
AC_DEFUN([LYX_USE_PACKAGING],
[AC_MSG_CHECKING([what packaging should be used])
AC_ARG_WITH(packaging,
  [  --with-packaging=THIS   Use THIS packaging for installation:
			    Possible values: posix, windows, macosx],
  [lyx_use_packaging="$withval"], [
  case $host in
    *-apple-darwin*) lyx_use_packaging=macosx ;;
     *-pc-mingw32*) lyx_use_packaging=windows;;
                  *) lyx_use_packaging=posix;;
  esac])
AC_MSG_RESULT($lyx_use_packaging)
lyx_install_macosx=false
case $lyx_use_packaging in
   macosx) AC_DEFINE(USE_MACOSX_PACKAGING, 1, [Define to 1 if LyX should use a MacOS X application bundle file layout])
	   PACKAGE=LyX${version_suffix}
	   default_prefix="/Applications/${PACKAGE}.app"
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
	   mandir='${prefix}/Resources/man' ;;
    posix) AC_DEFINE(USE_POSIX_PACKAGING, 1, [Define to 1 if LyX should use a POSIX-style file layout])
	   PACKAGE=lyx${version_suffix}
	   program_suffix=$version_suffix
	   pkgdatadir='${datadir}/${PACKAGE}'
	   default_prefix=$ac_default_prefix ;;
    *) LYX_ERROR([Unknown packaging type $lyx_use_packaging]) ;;
esac
AM_CONDITIONAL(INSTALL_MACOSX, $lyx_install_macosx)
dnl Next two lines are only for autoconf <= 2.59
datadir='${datarootdir}'
AC_SUBST(datarootdir)
AC_SUBST(pkgdatadir)
AC_SUBST(program_suffix)
])


## ------------------------------------------------------------------------
## Find a file (or one of more files in a list of dirs)
## ------------------------------------------------------------------------
##
AC_DEFUN([AC_FIND_FILE],
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    if test -r "$i/$j"; then
      $3=$i
      break 2
    fi
  done
done
])

dnl just a wrapper to clean up configure.in
AC_DEFUN([LYX_PROG_LIBTOOL],
[
AC_REQUIRE([AC_ENABLE_SHARED])
AC_REQUIRE([AC_ENABLE_STATIC])
dnl libtool is only for C, so I must force him
dnl to find the correct flags for C++
ac_save_cc=$CC
ac_save_cflags="$CFLAGS"
CC=$CXX
CFLAGS="$CXXFLAGS"
AC_PROG_LIBTOOL dnl for libraries
CC=$ac_save_cc
CFLAGS="$ac_save_cflags"
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
 lyx_dir_ver=`echo LYX_DIR_${lyx_major}${lyx_minor}x`
 lyx_userdir_ver=`echo LYX_USERDIR_${lyx_major}${lyx_minor}x`
 AC_SUBST(LYX_MAJOR_VERSION,$lyx_major)
 AC_SUBST(LYX_MINOR_VERSION,$lyx_minor)
 AC_SUBST(LYX_RELEASE_LEVEL,$lyx_release)
 AC_SUBST(LYX_RELEASE_PATCH,$lyx_patch)
 AC_SUBST(LYX_DIR_VER,"$lyx_dir_ver")
 AC_SUBST(LYX_USERDIR_VER,"$lyx_userdir_ver")
])
