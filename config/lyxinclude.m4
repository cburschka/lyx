dnl Some useful functions for LyXs configure.in                 -*- sh -*-
dnl Author: Jean-Marc Lasgouttes (Jean-Marc.Lasgouttes@inria.fr)
dnl         Lars Gullik Bj�nnes (larsbj@lyx.org)
dnl         Allan Rae (rae@lyx.org)


dnl Usage LYX_GET_VERSION   Sets "lyx_version" to the version of LyX being 
dnl   built, displays it and sets variable "lyx_devel" to yes or no depending 
dnl   whether the version is a development release or not. 
AC_DEFUN(LYX_GET_VERSION,[
changequote(, ) dnl
VERSION=`grep '#define *LYX_VERSION' $1 |
              sed -e 's/^.*"\(.*\)"$/\1/' 2>/dev/null`
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
PACKAGE=lyx${program_suffix}
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
    IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
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


AC_DEFUN(LYX_PROG_CXX_WORKS,
[AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILER([class foo { int bar; }; int main(){return(0);}], ac_cv_prog_cxx_works, ac_cv_prog_cxx_cross)
AC_LANG_RESTORE
if test $ac_cv_prog_cxx_works = no; then
  CXX=
fi
cross_compiling=$ac_cv_prog_cxx_cross
])


AC_DEFUN(LYX_PROG_CXX,
[AC_BEFORE([$0], [AC_PROG_CXXCPP])dnl
AC_MSG_CHECKING([for a working C++ compiler])
LYX_SEARCH_PROG(CXX, $CCC g++ gcc c++ CC cxx xlC cc++, LYX_PROG_CXX_WORKS)

if test -z "$CXX" ; then
  AC_ERROR([Unable to find a working C++ compiler])
fi

AC_SUBST(CXX)
AC_MSG_RESULT($CXX)

AC_MSG_CHECKING([whether the C++ compiler ($CXX $CXXFLAGS $LDFLAGS) is a cross-compiler])
AC_MSG_RESULT($cross_compiling)

AC_PROG_CXX_GNU

dnl We might want to get or shut warnings.
AC_ARG_WITH(warnings,
  [  --with-warnings         tell the compiler to display more warnings],,
  [ if test $lyx_devel_version = yes -o $lyx_prerelease = yes && test $ac_cv_prog_gxx = yes ; then
	with_warnings=yes;
    else
	with_warnings=no;
    fi;])
if test x$with_warnings = xyes ; then
  lyx_flags="$lyx_flags warnings"
  AC_DEFINE(WITH_WARNINGS, 1,
  [Define this if you want to see the warning directives put here and
   there by the developpers to get attention])
fi

# optimize less for development versions
if test $lyx_devel_version = yes -o $lyx_prerelease = yes ; then
  lyx_opt="-O"
else
  lyx_opt="-O2"
fi

# set the debug flags correctly.
if test $ac_cv_prog_gxx = yes; then
  GXX=yes
dnl Check whether -g works, even if CXXFLAGS is set, in case the package
dnl plays around with CXXFLAGS (such as to build both debugging and
dnl normal versions of a library), tasteless as that idea is.
  ac_test_CXXFLAGS="${CXXFLAGS+set}"
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS=
dnl Check the version of g++
  gxx_version=`${CXX} -dumpversion || echo unknown`
  
  AC_PROG_CXX_G
  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  elif test $ac_cv_prog_cxx_g = yes; then
    case $gxx_version in
      2.95.1)  CXXFLAGS="-g $lyx_opt -fpermissive -fno-rtti -fno-exceptions";;
      2.95.2)  CXXFLAGS="-g $lyx_opt -fno-rtti -fno-exceptions";;
      2.95.*)  CXXFLAGS="-g $lyx_opt -fno-exceptions";;
      2.96*)  CXXFLAGS="-g $lyx_opt -fno-exceptions";;
      3.0*)    CXXFLAGS="-g $lyx_opt";;
      *2.91.*) CXXFLAGS="-g $lyx_opt -fno-rtti -fno-exceptions";;
      *)       CXXFLAGS="-g $lyx_opt -fno-rtti -fno-exceptions";;
    esac
  else
    CXXFLAGS="$lyx_opt"
  fi
  if test x$with_warnings = xyes ; then
    case $gxx_version in
	2.95.*) CXXFLAGS="$CXXFLAGS -W -Wall";;
	2.96*)  CXXFLAGS="$CXXFLAGS -W -Wall";;
	2.97*)  CXXFLAGS="$CXXFLAGS -W -Wall";;
	*)      CXXFLAGS="$CXXFLAGS -W -Wall";;
    esac
    if test $lyx_devel_version = yes ; then
	case $gxx_version in
	    2.95.*) ;;
	    2.96*) ;;
	    2.97*) CXXFLAGS="$CXXFLAGS -Wconversion -Winline";;
	    *2.91*) ;;
	    *) ;;
        esac
    fi
  fi
else
  GXX=
  test "${CXXFLAGS+set}" = set || CXXFLAGS="-g"
fi
])dnl


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
	explicit Expl() {};
};],,lyx_cv_explicit=yes,lyx_cv_explicit=no)
])
if test $lyx_cv_explicit = no ; then
  AC_DEFINE(explicit,[ ], 
   [Define to nothing if your compiler does not understand the
   'explicit' directive])
fi])


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
    AC_REQUIRE([LYX_PROG_CXX])
    AC_MSG_CHECKING(whether the included std::string should be used)
    AC_ARG_WITH(included-string,
       [  --with-included-string  use LyX string class instead of STL string],
       [lyx_cv_with_included_string=$withval],
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
    AC_REQUIRE([LYX_PROG_CXX])
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


dnl Usage: LYX_REGEX : checks if the header regex.h is available
dnl   if it is not available the automake variable USE_REGEX will be
dnl   defined and the regex.h and regex.c that we provide will be used.
AC_DEFUN(LYX_REGEX,[
    AC_CHECK_HEADERS(regex.h, lyx_regex=no, lyx_regex=yes)
    AM_CONDITIONAL(USE_REGEX, test x$lyx_regex = xyes)
])


dnl LYX_CXX_MUTABLE
AC_DEFUN(LYX_CXX_MUTABLE, [
    AC_REQUIRE([LYX_PROG_CXX])
    AC_CACHE_CHECK([if C++ compiler supports mutable],
    lyx_cv_cxx_mutable,[
	AC_TRY_COMPILE(
	[
	class k {       
		mutable char *c;
	public:
		void foo() const { c=0; }
	};
	],[
	],[
	lyx_cv_cxx_mutable=yes
	],[
	lyx_cv_cxx_mutable=no
	])
    ])
    if test $lyx_cv_cxx_mutable = yes ; then
	AC_DEFINE(HAVE_MUTABLE, 1,
	[Defined if your compiler suports 'mutable'.])
    fi
])


dnl LYX_CXX_PARTIAL
AC_DEFUN(LYX_CXX_PARTIAL, [
    AC_REQUIRE([LYX_PROG_CXX])
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


dnl Usage: LYX_CXX_NAMESPACES : checks whether the C++ compiler
dnl   has a correct namespace handling and define CXX_WORKING_NAMESPACES 
dnl   if true. This macro does not do a thourough test, but it should be 
dnl   good enough to suit our needs.
AC_DEFUN(LYX_CXX_NAMESPACES,[
    AC_CACHE_CHECK(for correct namespaces support,lyx_cv_cxx_namespace,
    [AC_TRY_COMPILE([
    namespace foo {
	int bar;
    }
    ],[
        foo::bar = 0;
	return 0;
    ],lyx_cv_cxx_namespace=yes,lyx_cv_cxx_namespace=no)
    ])
    if test x$lyx_cv_cxx_namespace = xyes ; then
	AC_DEFINE(CXX_WORKING_NAMESPACES, 1, 
	[Define if your C++ compiler has correct support for namespaces])
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
    dnl the test only makes sense if we support namespaces
    namespace foo {
      int bar;
    }
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
AC_CHECK_LIB(Xpm, XpmCreateBufferFromImage,LYX_LIBS="-lXpm $LYX_LIBS",
        [LYX_LIB_ERROR(libXpm,Xpm)], $LYX_LIBS)

### Check for Xpm headers
lyx_cv_xpm_h_location="<xpm.h>"
AC_CHECK_HEADER(X11/xpm.h,[
  ac_cv_header_xpm_h=yes
  lyx_cv_xpm_h_location="<X11/xpm.h>"],[
AC_CHECK_HEADER(xpm.h,[],[
LYX_LIB_ERROR(xpm.h,Xpm)])])
AC_DEFINE_UNQUOTED(XPM_H_LOCATION,$lyx_cv_xpm_h_location)

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


dnl Usage LYX_PATH_XFORMS: Checks for xforms library and flags
AC_DEFUN(LYX_PATH_XFORMS,[
### Check for xforms library
AC_CHECK_LIB(forms, fl_initialize, LYX_LIBS="-lforms $LYX_LIBS", 
  [AC_CHECK_LIB(xforms, fl_initialize, LYX_LIBS="-lxforms $LYX_LIBS", 
    [LYX_LIB_ERROR(libforms or libxforms,xforms)], $LYX_LIBS)], $LYX_LIBS) 

### Check for xforms headers
lyx_cv_forms_h_location="<forms.h>"
AC_CHECK_HEADER(X11/forms.h,[
  ac_cv_header_forms_h=yes
  lyx_cv_forms_h_location="<X11/forms.h>"],[
AC_CHECK_HEADER(forms.h,[],[
LYX_LIB_ERROR(forms.h,forms)])])
AC_DEFINE_UNQUOTED(FORMS_H_LOCATION,$lyx_cv_forms_h_location)
if test $ac_cv_header_forms_h = yes; then
  AC_CACHE_CHECK([xforms header version],lyx_cv_xfversion,
  [ cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"

#include FORMS_H_LOCATION
#if ! defined(FL_INCLUDE_VERSION)
"%%%"(unknown)"%%%"
#else
"%%%"FL_VERSION.FL_REVISION.FL_FIXLEVEL"%%%"
#endif
EOF
lyx_cv_xfversion=`(eval "$ac_cpp conftest.$ac_ext") 2>&5 | \
  grep '^"%%%"'  2>/dev/null | \
  sed -e 's/^"%%%"\(.*\)"%%%"/\1/' -e 's/ //g'`
rm -f conftest*])
case "$lyx_cv_xfversion" in 
  "(unknown)"|0.8[1-7]*) 
         LYX_ERROR(dnl
Version $lyx_cv_xfversion of xforms is not compatible with LyX. 
   This version of LyX works best with versions 0.88 (recommended) and later.) ;;
    0.88*) ;;
    0.89[01234]) LYX_WARNING(dnl
LyX should work ok with version $lyx_cv_xfversion of xforms[,] but
it is an unproven version and might still have some bugs. You should 
probably use version 0.89.6 (or 0.88) instead) ;;
    0.89*) ;;
       *) LYX_WARNING(dnl
Version $lyx_cv_xfversion of xforms might not be compatible with LyX[,] 
 since it is newer than 0.88. You might have slight problems with it.);;
esac
fi])


dnl Usage: LYX_HPUX  Checks for HP-UX and update CXXFLAGS accordingly
AC_DEFUN(LYX_HPUX,
[#It seems that HPUX requires using -fpcc-struct-return with gcc.
AC_CACHE_CHECK(for HP-UX,ac_cv_hpux,[
os=`uname -s | tr '[A-Z]' '[a-z]'`
ac_cv_hpux=no
test "$os" = hp-ux && ac_cv_hpux=yes])
if test "$ac_cv_hpux" = yes; then
 test "x$GXX" = xyes && CXXFLAGS="$CXXFLAGS -fpcc-struct-return"
fi])


dnl Usage: LYX_SUNOS4 Checks for SunOS 4.x and sets the flag lyx_broken_headers
dnl   if necessary
AC_DEFUN(LYX_SUNOS4,
[#The headers are not correct under SunOS4
AC_CACHE_CHECK(for SunOS 4.x,ac_cv_sunos4,[
changequote(, ) dnl
os=`uname -a | sed -e 's/^\([^ ]*\) [^ ]* \([0-9]\)\..*/\1\2/'`
changequote([, ]) dnl
ac_cv_sunos4=no
test "$os" = SunOS4 && ac_cv_sunos4=yes])
if test "$ac_cv_sunos4" = yes; then
 test "x$GXX" = xyes && lyx_broken_headers=yes
fi])


dnl Usage: LYX_SCO Checks for SCO and sets the flag lyx_broken_headers
dnl   if necessary
AC_DEFUN(LYX_SCO,
[AC_CACHE_CHECK(for SCO 3.2v4,ac_cv_sco,[
ac_cv_sco=no
if test `uname -s` != "SCO_SV"; then
  lyx_machine_rel=`uname -m`:`uname -r`
  if test $lyx_machine_rel = i386:3.2 || test $lyx_machine_rel = i486:3.2;
  then
    if test -f /usr/options/cb.name; then
      ac_cv_sco=no
    elif /bin/uname -X 2>/dev/null >/dev/null ; then
      ac_cv_sco=yes
    fi
  fi
fi])
if test "$ac_cv_sco" = yes; then
 test "x$GXX" = xyes && lyx_broken_headers=yes
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
IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
for dir in `eval "echo $1"`; do
  if test ! "$dir" = NONE; then
    test ! -d "$dir" && AC_ERROR([\"$dir\" is not a directory])
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

### Check which libsigc++ we're using and make sure any external one works
### Check for libsigc++ library
AC_DEFUN(LYX_WITH_SIGC,
[AC_MSG_CHECKING(whether the included libsigc++ should be used)
AC_ARG_WITH([included-libsigc],
  [  --without-included-libsigc
                             Use the libsigc++ installed on the system],
  [lyx_use_included_libsigc=$withval],
  [lyx_use_included_libsigc=yes])
AC_MSG_RESULT([$lyx_use_included_libsigc])
if test x$lyx_use_included_libsigc = xno; then
  ### Check for libsigc++
  ## can't use AC_SUBST right here!
  AM_PATH_SIGC(0.8.7,
    [ INCLUDED_SIGC=
    ],
    [LYX_ERROR(dnl
    [Cannot find libsigc++ library or headers at least as recent as 0.8.7.
     Check your installation.  Have you installed the development package?])
  ])
else
  ### Use the included libsigc++
  ### sigc-config hasn't been created yet so we can't just do the same as above
  ### unless of course someone gets keen and merges the sigc++ configure.in 
  ### with this one.  We don't really gain much by doing that though except
  ### a considerably smaller dist and more difficult maintenance.
  ### It'd also mean we'd have the equivalent of config/gettext.m4
  lyx_flags="$lyx_flags included-libsigc"
  SIGC_LIBS="\`\${top_builddir}/sigc++/sigc-config --libs-names | sed -e 's/-lsigc//'\`"
  # Libsigc++ always installs into a subdirectory called sigc++.  Therefore we
  # have to always use #include <sigc++/signal_system.h> in our code.
  # Remember if you decide to do anything to the sigc++ code to do your mods on
  # the makeLyXsigc.sh script in development/tools using a current cvs checkout
  # of libsigc++.  A tarball distribution doesn't have everything in it that we
  # need.
  # We need both these -I entries to build when builddir != srcdir
  if test "x$src_dir" = "x." ; then
    SIGC_CFLAGS="-I\${top_srcdir}"
  else
    SIGC_CFLAGS="-I\${top_builddir} -I\${top_srcdir}"
  fi
  INCLUDED_SIGC="\${top_builddir}/sigc++/libsigc.la"
  ## can't substitute these here like this otherwise all remaining tests fail
  ## instead we SUBST directly into the Makefiles
  ##LIBS="$LIBS \$SIGC_LIBS"
  ##CPPFLAGS="$CPPFLAGS \$SIGC_CFLAGS"
  AC_SUBST(SIGC_LIBS)
  AC_SUBST(SIGC_CFLAGS)
fi
AC_SUBST(INCLUDED_SIGC)
])

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
  AC_DEFINE_UNQUOTED($ac_tr_safe, $lyx_path_header_path)])
])
### end of LYX_PATH_HEADER

### Check for stl_string_fwd.h existence and location if it exists
AC_DEFUN(LYX_STL_STRING_FWD,
[ AC_CHECK_HEADER(stl_string_fwd.h,[
  AC_CACHE_CHECK([path to stl_string_fwd.h],lyx_cv_path_stl_string_fwd_h,
  [ cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"

#include <stl_string_fwd.h>
EOF
lyx_cv_path_stl_string_fwd_h=`(eval "$ac_cpp conftest.$ac_ext") 2>&5 | \
  grep 'stl_string_fwd.h'  2>/dev/null | \
  sed -e 's/.*\(".*stl_string_fwd.h"\).*/\1/' -e "1q"`
rm -f conftest*])
  AC_DEFINE_UNQUOTED(STL_STRING_FWD_H_LOCATION,$lyx_cv_path_stl_string_fwd_h,
[define this to the location of stl_string_fwd.h to be used with #include,
  NOTE: Do not set it to <stl_string_fwd.h> as that will find the LyX
  	supplied version of the header.
  e.g. <../include/stl_string_fwd.h> or better yet use an absolute path])])
])


dnl AC_VALIDATE_CACHE_SYSTEM_TYPE[(cmd)]
dnl if the cache file is inconsistent with the current host,
dnl target and build system types, execute CMD or print a default
dnl error message.
AC_DEFUN(AC_VALIDATE_CACHE_SYSTEM_TYPE, [
    AC_REQUIRE([AC_CANONICAL_SYSTEM])
    AC_MSG_CHECKING([config.cache system type])
    if { test x"${ac_cv_host_system_type+set}" = x"set" &&
         test x"$ac_cv_host_system_type" != x"$host"; } ||
       { test x"${ac_cv_build_system_type+set}" = x"set" &&
         test x"$ac_cv_build_system_type" != x"$build"; } ||
       { test x"${ac_cv_target_system_type+set}" = x"set" &&
         test x"$ac_cv_target_system_type" != x"$target"; }; then
        AC_MSG_RESULT([different])
        ifelse($#, 1, [$1],
                [AC_MSG_ERROR(["you must remove config.cache and restart configure"])])
    else
        AC_MSG_RESULT([same])
    fi
    ac_cv_host_system_type="$host"
    ac_cv_build_system_type="$build"
    ac_cv_target_system_type="$target"
])

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
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG1,$ac_cv_func_select_arg1)
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG234,($ac_cv_func_select_arg234))
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG5,($ac_cv_func_select_arg5))
])

### Check which frontend we want to use. The default is XForms
###
AC_DEFUN(LYX_USE_FRONTEND,
[AC_MSG_CHECKING(what frontend should be used as main GUI)
AC_ARG_WITH(frontend,
  [  --with-frontend[=value] EXPERIMENTAL (only xforms works) Use THIS frontend as main GUI:
                          Possible values: xforms,kde,gnome],
  [lyx_use_frontend="$withval"], [lyx_use_frontend="xforms"])
AC_MSG_RESULT($lyx_use_frontend)
lyx_flags="$lyx_flags frontend-$lyx_use_frontend"
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
        AC_DEFINE_UNQUOTED(HAVE_DECL_${tr_func})
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
