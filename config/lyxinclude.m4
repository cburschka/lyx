dnl Some useful functions for LyXs configure.in                 -*- sh -*-
dnl Author: Jean-Marc Lasgouttes (Jean-Marc.Lasgouttes@inria.fr)
dnl         Lars Gullik Bjønnes (larsbj@lyx.org)
dnl         Allan Rae (rae@lyx.org)


dnl Usage LYX_CHECK_VERSION   Displays version of LyX being built and
dnl sets variables "lyx_devel_version" and "lyx_prerelease"
AC_DEFUN(LYX_CHECK_VERSION,[
changequote(, ) dnl
echo "configuring LyX version $VERSION"
if echo "$VERSION" | grep 'cvs' >/dev/null ; then
  lyx_devel_version=yes
  echo "WARNING: This is a development version. Expect bugs."
else
  lyx_devel_version=no
fi
if echo "$VERSION" | grep 'pre' > /dev/null ; then
    lyx_prerelease=yes
    echo "WARNING: This is a prerelease. Be careful and backup your documents."
else
    lyx_prerelease=no
fi
changequote([, ]) dnl
AC_SUBST(lyx_devel_version)
if test $lyx_devel_version = yes ; then
  AC_DEFINE(DEVEL_VERSION, 1, Define if you are building a development version of LyX)
fi])


dnl Define the option to set a LyX version on installed executables and directories
dnl
dnl
AC_DEFUN(LYX_VERSION_SUFFIX,[
AC_MSG_CHECKING([for install target ... ])
AC_ARG_WITH(version-suffix,
  [  --with-version-suffix[=<version>]  install lyx files as lyx<version>],
  [if test "x$withval" = "xyes";
   then
     withval="-$VERSION"
     ac_configure_args=`echo $ac_configure_args | sed "s,--with-version-suffix,--with-version-suffix=$withval,"`
   fi
   lyxname="lyx$withval"
   program_suffix=$withval],
  [lyxname=lyx])
AC_MSG_RESULT([$lyxname])
])

dnl Usage: LYX_ERROR(message)  Displays the warning "message" and sets the
dnl flag lyx_error to yes.
AC_DEFUN(LYX_ERROR,[
lyx_error_txt="$lyx_error_txt
** $1
"
lyx_error=yes])


dnl Usage: LYX_WARNING(message)  Displays the warning "message" and sets the
dnl flag lyx_warning to yes.
AC_DEFUN(LYX_WARNING,[
lyx_warning_txt="$lyx_warning_txt
== $1
"
lyx_warning=yes])


dnl Usage: LYX_LIB_ERROR(file,library)  Displays an error message indication
dnl  that 'file' cannot be found because 'lib' may be uncorrectly installed.
AC_DEFUN(LYX_LIB_ERROR,[
LYX_ERROR([Cannot find $1. Please check that the $2 library
   is correctly installed on your system.])])


dnl Usage: LYX_CHECK_ERRORS  Displays a warning message if a LYX_ERROR
dnl   has occured previously.
AC_DEFUN(LYX_CHECK_ERRORS,[
if test x$lyx_error = xyes; then
cat <<EOF
**** The following problems have been detected by configure.
**** Please check the messages below before running 'make'.
**** (see the section 'Problems' in the INSTALL file)
$lyx_error_txt
$lyx_warning_txt
deleting cache $cache_file
EOF
  rm -f $cache_file
else

if test x$lyx_warning = xyes; then
cat <<EOF
=== The following minor problems have been detected by configure.
=== Please check the messages below before running 'make'.
=== (see the section 'Problems' in the INSTALL file)
$lyx_warning_txt
EOF
fi
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


AC_DEFUN(LYX_PROG_CXX,
[AC_MSG_CHECKING([for a good enough C++ compiler])
LYX_SEARCH_PROG(CXX, $CXX $CCC g++ gcc c++ CC cxx xlC cc++, [LYX_PROG_CXX_WORKS])

if test -z "$CXX" ; then
  AC_MSG_ERROR([Unable to find a good enough C++ compiler])
fi
AC_MSG_RESULT($CXX)

AC_PROG_CXX

### We might want to get or shut warnings.
AC_ARG_ENABLE(warnings,
  [  --enable-warnings       tell the compiler to display more warnings],,
  [ if test $lyx_devel_version = yes -o $lyx_prerelease = yes && test $ac_cv_prog_gxx = yes ; then
	enable_warnings=yes;
    else
	enable_warnings=no;
    fi;])
if test x$enable_warnings = xyes ; then
  lyx_flags="$lyx_flags warnings"
  AC_DEFINE(WITH_WARNINGS, 1,
  [Define this if you want to see the warning directives put here and
   there by the developpers to get attention])
fi

### We might want to disable debug
AC_ARG_ENABLE(debug,
  [  --enable-debug          enable debug information],,
  [ if test $lyx_devel_version = yes -o $lyx_prerelease = yes && test $ac_cv_prog_gxx = yes ; then
	enable_debug=yes;
    else
	enable_debug=no;
    fi;])

### set up optimization
AC_ARG_ENABLE(optimization,
  [  --enable-optimization[=value]   enable compiler optimisation],,
	enable_optimization=yes;)
case $enable_optimization in
  yes) lyx_opt=-O;;
   no) lyx_opt=;;
    *) lyx_opt=${enable_optimization};;
esac

# set the debug flags correctly.
if test x$GXX = xyes; then
  dnl Useful for global version info
  gxx_version=`${CXX} -dumpversion`
  CXX_VERSION="($gxx_version)"

  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  else
    case $gxx_version in
      2.95.1)  CXXFLAGS="$lyx_opt -fpermissive";;
      2.95.*)  CXXFLAGS="$lyx_opt -Wno-non-template-friend";;
      2.96*)  CXXFLAGS="$lyx_opt -fno-exceptions -Wno-non-template-friend";;
      3.0*)    CXXFLAGS="$lyx_opt";;
      3.1*)    CXXFLAGS="$lyx_opt -finline-limit=500 -fno-exceptions";;
      3.2*)    CXXFLAGS="$lyx_opt -fno-exceptions";;
      *)       CXXFLAGS="$lyx_opt";;
    esac
    if test x$enable_debug = xyes ; then
	CXXFLAGS="-g $CXXFLAGS"
    fi
  fi
  if test x$enable_warnings = xyes ; then
    case $gxx_version in
	2.95.*) CXXFLAGS="$CXXFLAGS -W -Wall";;
	2.96*)  CXXFLAGS="$CXXFLAGS -W -Wall";;
	3.1*)  CXXFLAGS="$CXXFLAGS -W -Wall";;
	*)      CXXFLAGS="$CXXFLAGS -W -Wall";;
    esac
    if test $lyx_devel_version = yes ; then
      case $gxx_version in
	  2.95.*) ;;
	  2.96*) ;;
	  *) CXXFLAGS="$CXXFLAGS -Winline";;
      esac
    fi
  fi
fi])dnl


dnl NOT USED CURRENTLY*************************************
dnl Usage: LYX_CXX_RTTI : checks whether the C++ compiler
dnl   supports RTTI
AC_DEFUN(LYX_CXX_RTTI,[
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
AC_DEFUN(LYX_CXX_EXPLICIT,[
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
AC_DEFUN(LYX_CXX_STL_STACK,[
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
AC_DEFUN(LYX_STD_COUNT,[
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
AC_DEFUN(LYX_CXX_STL_MODERN_STREAMS,[
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


dnl Usage: LYX_CXX_STL_STRING : checks whether the C++ compiler
dnl   has a std::string that is usable for LyX. LyX does not require this
dnl   std::string to be standard.
AC_DEFUN(LYX_CXX_STL_STRING,[
    AC_REQUIRE([AC_PROG_CXX])
    AC_MSG_CHECKING(whether the included std::string should be used)
    AC_ARG_WITH(included-string,
       [  --with-included-string  use LyX string class instead of STL string],
       [lyx_cv_with_included_string=$withval
	AC_MSG_RESULT([$with_included_string])],
       [AC_CACHE_CHECK([],lyx_cv_with_included_string,
	[AC_TRY_COMPILE([
	    #include <string>
	    using std::string;
	],[
	    // LyX has reduced its requirements on the basic_string
	    // implementation so that the basic_string supplied
	    // with gcc is usable. In particular this means that
	    // lyx does not use std::string::clear and not the
	    // strncmp version of std::string::compare. This is mainly
	    // done so that LyX can use precompiled C++ libraries that
	    // already uses the systems basic_string, e.g. gtk--
	    string a("hello there");
	    a.erase();
	    a = "hey";
	    //char s[] = "y";
	    //int t = a.compare(a.length() - 1, 1, s);
	    a.erase();
	],[
	    lyx_cv_with_included_string=no
	],[
	    lyx_cv_with_included_string=yes
	])
	])
    ])
    if test x$lyx_cv_with_included_string = xyes ; then
	AC_DEFINE(USE_INCLUDED_STRING, 1,
	    [Define to use the lyxstring class bundled with LyX.])
	    lyx_flags="$lyx_flags included-string"
    fi
    AM_CONDITIONAL(USE_LYXSTRING, test x$lyx_cv_with_included_string = xyes)
dnl    AC_MSG_RESULT([$with_included_string])
])


dnl Usage: LYX_CXX_GOOD_STD_STRING : checks whether the C++ compiler
dnl   has a std::string that is close to the standard. So close that
dnl   methods not found in "unstandard" std::strings are present here.
AC_DEFUN(LYX_CXX_GOOD_STD_STRING,[
    AC_REQUIRE([AC_PROG_CXX])
    AC_CACHE_CHECK([whether the systems std::string is really good],
    [lyx_cv_std_string_good],
    [AC_TRY_COMPILE([
	    #include <string>
	    using std::string;
	],[
	    // From a std::string that is supposed to be close to the
	    // standard we require at least three things:
	    // - clear() and erase()
	    // - the strncmp of compare()
	    // - push_back()
	    string a("hello there");
	    a.erase();
	    a = "hey";
	    char s[] = "y";
	    int t = a.compare(a.length() - 1, 1, s);
	    a.push_back('g');
	    a.clear();
	],[
	    lyx_cv_std_string_good=yes
	],[
	    lyx_cv_std_string_good=no

	])
    ])
    if test x$lyx_cv_std_string_good = xyes ; then
	AC_DEFINE(STD_STRING_IS_GOOD, 1,
	    [Define if the systems std::string is really good.])
    fi
])


dnl NOT USED CURRENTLY*************************************
dnl LYX_CXX_PARTIAL
AC_DEFUN(LYX_CXX_PARTIAL, [
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


dnl Usage: LYX_CXX_CHEADERS : checks whether the C++ compiler
dnl   provides wrappers for C headers and use our alternate version otherwise.
AC_DEFUN(LYX_CXX_CHEADERS,[
AC_CACHE_CHECK(for C headers wrappers,lyx_cv_cxx_cheaders,
 [AC_TRY_CPP([
#include <clocale>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>],[lyx_cv_cxx_cheaders=yes],[lyx_cv_cxx_cheaders=no])])
if test $lyx_cv_cxx_cheaders = no ; then
  LYX_ADD_INC_DIR(lyx_cppflags,\$(top_srcdir)/src/cheaders)
fi])

dnl Usage: LYX_CXX_GLOBAL_CSTD: checks whether C library functions
dnl   are already in the global namespace
AC_DEFUN(LYX_CXX_GLOBAL_CSTD,[
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

dnl Usage LYX_PATH_XPM: Checks for xpm library and header
AC_DEFUN(LYX_PATH_XPM,[
### Check for Xpm library
AC_CHECK_LIB(Xpm, XpmCreateBufferFromImage, XPM_LIB="-lXpm",
	[LYX_LIB_ERROR(libXpm,Xpm)])
AC_SUBST(XPM_LIB)
### Check for Xpm headers
lyx_cv_xpm_h_location="<xpm.h>"
AC_CHECK_HEADER(X11/xpm.h,[
  ac_cv_header_xpm_h=yes
  lyx_cv_xpm_h_location="<X11/xpm.h>"],[
AC_CHECK_HEADER(xpm.h,[],[
LYX_LIB_ERROR(xpm.h,Xpm)])])
AC_DEFINE_UNQUOTED(XPM_H_LOCATION,$lyx_cv_xpm_h_location,
  [define this to the location of xpm.h to be used with #include, e.g. <xpm.h>])

### Test for the header version
if test $ac_cv_header_xpm_h = yes; then
  AC_CACHE_CHECK([xpm header version],lyx_cv_xpmversion,
  [ cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"

#include XPM_H_LOCATION
"%%%"lyx_cv_xpmv=XpmVersion;lyx_cv_xpmr=XpmRevision"%%%"
EOF
    eval `(eval "$ac_cpp conftest.$ac_ext") 2>&5 | \
      grep '^"%%%"'  2>/dev/null | \
      sed -e 's/^"%%%"\(.*\)"%%%"/\1/' -e 's/ //g'`
    case "$lyx_cv_xpmr" in
changequote(,)
     [0-9]) lyxxpmr_alpha=`echo $lyx_cv_xpmr |tr 123456789 abcdefghi`
	    lyxxpmv_alpha=" (aka 3.$lyx_cv_xpmv$lyxxpmr_alpha)";;
changequote([,])
	 *) ;;
    esac
    lyx_cv_xpmversion="$lyx_cv_xpmv.$lyx_cv_xpmr$lyxxpmv_alpha"
    rm -f conftest*])
  XPM_VERSION=${lyx_cv_xpmversion}
  case "$lyx_cv_xpmr" in
changequote(,)
	[789]|[0-9][0-9]*) ;;
changequote([,])
	*) LYX_WARNING([Version $lyx_cv_xpmversion of the Xpm library is a bit old.
   If you experience strange crashes with LyX, try to upgrade
   to at least version 4.7 (aka 3.4g).
   If you have installed a newer version of the library, check whether you
   have an old xpm.h header file in your include path.]);;
  esac
fi])


dnl Usage: LYX_FUNC_PUTENV_ARGTYPE
dnl Checks whether putenv() takes 'char const *' or 'char *' as
dnl argument. This is needed because Solaris 7 (wrongly?) uses 'char *',
dnl while everybody else uses the former...
AC_DEFUN(LYX_FUNC_PUTENV_ARGTYPE,
[AC_MSG_CHECKING([type of argument for putenv()])
 AC_CACHE_VAL(lyx_cv_func_putenv_arg,dnl
  [AC_TRY_COMPILE(dnl
[#include <cstdlib>],
[char const * foo = "bar";
 putenv(foo);],dnl
   [lyx_cv_func_putenv_arg='char const *'],[lyx_cv_func_putenv_arg='char *'])])
 AC_MSG_RESULT($lyx_cv_func_putenv_arg)
 AC_DEFINE_UNQUOTED(PUTENV_TYPE_ARG,$lyx_cv_func_putenv_arg,dnl
   [Define to the type of the argument of putenv(). Needed on Solaris 7.])])


dnl Usage: LYX_WITH_DIR(dir-name,desc,dir-var-name,default-value,
dnl                       [default-yes-value])
dnl  Adds a --with-'dir-name' option (described by 'desc') and puts the
dnl  resulting directory name in 'dir-var-name'.
AC_DEFUN(LYX_WITH_DIR,[
  AC_ARG_WITH($1,[  --with-$1        specify $2])
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
AC_DEFUN(LYX_LOOP_DIR,[
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
AC_DEFUN(LYX_ADD_LIB_DIR,[
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
AC_DEFUN(LYX_ADD_INC_DIR,[$1="${$1} -I$2 "])

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
AC_DEFUN(LYX_PATH_HEADER,
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

dnl We use this until autoconf fixes its version.
AC_DEFUN(LYX_FUNC_SELECT_ARGTYPES,
[AC_MSG_CHECKING([types of arguments for select()])
 AC_CACHE_VAL(ac_cv_func_select_arg234,dnl
 [AC_CACHE_VAL(ac_cv_func_select_arg1,dnl
  [AC_CACHE_VAL(ac_cv_func_select_arg5,dnl
   [for ac_cv_func_select_arg234 in 'fd_set *' 'int *' 'void *'; do
     for ac_cv_func_select_arg1 in 'int' 'size_t' 'unsigned long' 'unsigned'; do      for ac_cv_func_select_arg5 in 'struct timeval *' 'const struct timeval *'; do
       AC_TRY_COMPILE(dnl
[#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
extern int select ($ac_cv_func_select_arg1,$ac_cv_func_select_arg234,$ac_cv_func_select_arg234,$ac_cv_func_select_arg234,$ac_cv_func_select_arg5);],,dnl
	[ac_not_found=no ; break 3],ac_not_found=yes)
      done
     done
    done
   ])dnl AC_CACHE_VAL
  ])dnl AC_CACHE_VAL
 ])dnl AC_CACHE_VAL
 if test "$ac_not_found" = yes; then
  ac_cv_func_select_arg1=int
  ac_cv_func_select_arg234='int *'
  ac_cv_func_select_arg5='struct timeval *'
 fi
 AC_MSG_RESULT([$ac_cv_func_select_arg1,$ac_cv_func_select_arg234,$ac_cv_func_select_arg5])
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG1,$ac_cv_func_select_arg1,
		    [Define to the type of arg1 for select().])
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG234,($ac_cv_func_select_arg234),
		    [Define to the type of args 2, 3 and 4 for select().])
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG5,($ac_cv_func_select_arg5),
		    [Define to the type of arg5 for select().])
])

### Check which frontend we want to use. The default is XForms
###
AC_DEFUN(LYX_USE_FRONTEND,
[AC_MSG_CHECKING([what frontend should be used as main GUI])
AC_ARG_WITH(frontend,
  [  --with-frontend=THIS    Use THIS frontend as main GUI:
			    Possible values: xforms, qt2, gnome],
  [lyx_use_frontend="$withval"], [lyx_use_frontend="xforms"])
AC_MSG_RESULT($lyx_use_frontend)
AC_SUBST(FRONTEND)
AC_SUBST(FRONTEND_GUILIB)
AC_SUBST(FRONTEND_LDFLAGS)
AC_SUBST(FRONTEND_INCLUDES)
AC_SUBST(FRONTEND_LIBS)
])


dnl Check things are declared in headers to avoid errors or warnings.
dnl Called like LYX_CHECK_DECL(function, headerfile)
dnl Defines HAVE_DECL_{FUNCTION}
AC_DEFUN(LYX_CHECK_DECL,
[AC_MSG_CHECKING(if $1 is declared by header $2)
tr_func=`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`

tr_hdr=`echo $2 | tr . _`
AC_CACHE_VAL([lyx_cv_declare_${tr_hdr}_$1],
[AC_EGREP_HEADER($1, $2, [eval "lyx_cv_declare_${tr_hdr}_$1=yes"], [eval "lyx_cv_declare_${tr_hdr}_$1=no"])])
if eval "test \"\${lyx_cv_declare_${tr_hdr}_$1}\" = \"yes\""; then
	AC_DEFINE_UNQUOTED(HAVE_DECL_${tr_func},1,[dummy])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi])

dnl This is the multiple headers version of the LYX_CHECK_DECL macro above.
dnl Called like LYX_CHECK_DECL_HDRS(function, file1 file2 file3)
AC_DEFUN(LYX_CHECK_DECL_HDRS,
[ got="no"
for I in $2; do
tr_hdr=`echo $I | tr . _`
if test "${got}" = "no"; then
    LYX_CHECK_DECL($1, $I)
fi
if eval "test \"\${lyx_cv_declare_${tr_hdr}_$1}\" = \"yes\""; then
     got="yes"
fi
done])


## ------------------------------------------------------------------------
## Find a file (or one of more files in a list of dirs)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_FIND_FILE,
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
AC_DEFUN(LYX_PROG_LIBTOOL,
[
AC_REQUIRE([AC_ENABLE_SHARED])
AC_REQUIRE([AC_ENABLE_STATIC])
dnl libtool is only for C, so I must force him
dnl to find the correct flags for C++
ac_save_cc=$CC
ac_save_cflags="$CFLAGS"
CC=$CXX
CFLAGS="$CXXFLAGS"
AM_PROG_LIBTOOL dnl for libraries
CC=$ac_save_cc
CFLAGS="$ac_save_cflags"
])
