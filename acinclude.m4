dnl Some useful functions for LyXs configure.in                 -*- sh -*-
dnl Author: Jean-Marc Lasgouttes (Jean-Marc.Lasgouttes@inria.fr)
dnl         Lars Gullik Bjønnes (larsbj@lyx.org)
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
  gxx_version=`${CXX} --version`
  
  AC_PROG_CXX_G
  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  elif test $ac_cv_prog_cxx_g = yes; then
    case $gxx_version in
      2.95.1)  CXXFLAGS="-g $lyx_opt -fpermissive -fno-rtti -fno-exceptions";;
      2.95.*)  CXXFLAGS="-g $lyx_opt -fno-rtti -fno-exceptions";;
      2.96*)   CXXFLAGS="-g $lyx_opt -fhonor-std";;
      *2.91.*) CXXFLAGS="-g $lyx_opt -fno-rtti -fno-exceptions";;
      *)       CXXFLAGS="-g $lyx_opt -fno-rtti -fno-exceptions";;
    esac
  else
    CXXFLAGS="$lyx_opt"
  fi
  if test x$with_warnings = xyes ; then
    case $gxx_version in
	2.95.*) CXXFLAGS="$CXXFLAGS -W -Wall -Wconversion -Winline";;
	2.96*)  CXXFLAGS="$CXXFLAGS -W -Wall -Wconversion -Winline";;
	*)      CXXFLAGS="$CXXFLAGS -ansi -W -Wall -Wno-return-type";;
    esac
    if test $lyx_devel_version = yes ; then
	case $gxx_version in
	    2.95.*) ;;
	    2.96*) ;;
	    *2.91*) ;;
	    *) CXXFLAGS="$CXXFLAGS -pedantic";;
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
#include <string>
#include <algorithm>
using std::string;
using std::count;
int countChar(string const & a, char const c)
{
        return count(a.begin(), a.end(), c);
}
],[
    string a("hello");
    int i = countChar(a, 'l');
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
	    [Define is the systems std::string is really good.])
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
"%%%"FL_VERSION.FL_REVISION"%%%"
#endif
EOF
lyx_cv_xfversion=`(eval "$ac_cpp conftest.$ac_ext") 2>&5 | \
  grep '^"%%%"'  2>/dev/null | \
  sed -e 's/^"%%%"\(.*\)"%%%"/\1/' -e 's/ //g'`
rm -f conftest*])
case "$lyx_cv_xfversion" in 
  "(unknown)"|0.8[1-7]) 
         LYX_ERROR(dnl
Version $lyx_cv_xfversion of xforms is not compatible with LyX. 
   This version of LyX works best with versions 0.88 (recommended) and later.) ;;
    0.88) ;;
    0.89) LYX_WARNING(dnl
LyX should work ok with version $lyx_cv_xfversion of xforms[,] but
it is an unproven version and might still have some bugs. If you
have problems[,] please use version 0.88 instead.) ;;
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
  [  --with-frontend[=value] Use THIS frontend as main GUI:
                          Possible values: xforms,kde,gtk],
  [lyx_use_frontend="$withval"], [lyx_use_frontend="xforms"])
AC_MSG_RESULT($lyx_use_frontend)
lyx_flags="$lyx_flags frontend-$lyx_use_frontend"
AC_SUBST(FRONTEND)
AC_SUBST(FRONTEND_GUILIB)
AC_SUBST(FRONTEND_LDFLAGS)
AC_SUBST(FRONTEND_INCLUDES)
AC_SUBST(FRONTEND_LIBS)
])
## libtool.m4 - Configure libtool for the target system. -*-Shell-script-*-
## Copyright (C) 1996-1999 Free Software Foundation, Inc.
## Originally by Gordon Matzigkeit <gord@gnu.ai.mit.edu>, 1996
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##
## As a special exception to the GNU General Public License, if you
## distribute this file as part of a program that contains a
## configuration script generated by Autoconf, you may include it under
## the same distribution terms that you use for the rest of that program.

# serial 40 AC_PROG_LIBTOOL
AC_DEFUN(AC_PROG_LIBTOOL,
[AC_REQUIRE([AC_LIBTOOL_SETUP])dnl

# Save cache, so that ltconfig can load it
AC_CACHE_SAVE

# Actually configure libtool.  ac_aux_dir is where install-sh is found.
CC="$CC" CFLAGS="$CFLAGS" CPPFLAGS="$CPPFLAGS" \
LD="$LD" LDFLAGS="$LDFLAGS" LIBS="$LIBS" \
LN_S="$LN_S" NM="$NM" RANLIB="$RANLIB" \
DLLTOOL="$DLLTOOL" AS="$AS" OBJDUMP="$OBJDUMP" \
${CONFIG_SHELL-/bin/sh} $ac_aux_dir/ltconfig --no-reexec \
$libtool_flags --no-verify $ac_aux_dir/ltmain.sh $lt_target \
|| AC_MSG_ERROR([libtool configure failed])

# Reload cache, that may have been modified by ltconfig
AC_CACHE_LOAD

# This can be used to rebuild libtool when needed
LIBTOOL_DEPS="$ac_aux_dir/ltconfig $ac_aux_dir/ltmain.sh"

# Always use our own libtool.
LIBTOOL='$(SHELL) $(top_builddir)/libtool'
AC_SUBST(LIBTOOL)dnl

# Redirect the config.log output again, so that the ltconfig log is not
# clobbered by the next message.
exec 5>>./config.log
])

AC_DEFUN(AC_LIBTOOL_SETUP,
[AC_PREREQ(2.13)dnl
AC_REQUIRE([AC_ENABLE_SHARED])dnl
AC_REQUIRE([AC_ENABLE_STATIC])dnl
AC_REQUIRE([AC_ENABLE_FAST_INSTALL])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
AC_REQUIRE([AC_PROG_RANLIB])dnl
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_LD])dnl
AC_REQUIRE([AC_PROG_NM])dnl
AC_REQUIRE([AC_PROG_LN_S])dnl
dnl

case "$target" in
NONE) lt_target="$host" ;;
*) lt_target="$target" ;;
esac

# Check for any special flags to pass to ltconfig.
libtool_flags="--cache-file=$cache_file"
test "$enable_shared" = no && libtool_flags="$libtool_flags --disable-shared"
test "$enable_static" = no && libtool_flags="$libtool_flags --disable-static"
test "$enable_fast_install" = no && libtool_flags="$libtool_flags --disable-fast-install"
test "$ac_cv_prog_gcc" = yes && libtool_flags="$libtool_flags --with-gcc"
test "$ac_cv_prog_gnu_ld" = yes && libtool_flags="$libtool_flags --with-gnu-ld"
ifdef([AC_PROVIDE_AC_LIBTOOL_DLOPEN],
[libtool_flags="$libtool_flags --enable-dlopen"])
ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[libtool_flags="$libtool_flags --enable-win32-dll"])
AC_ARG_ENABLE(libtool-lock,
  [  --disable-libtool-lock  avoid locking (might break parallel builds)])
test "x$enable_libtool_lock" = xno && libtool_flags="$libtool_flags --disable-lock"
test x"$silent" = xyes && libtool_flags="$libtool_flags --silent"

# Some flags need to be propagated to the compiler or linker for good
# libtool support.
case "$lt_target" in
*-*-irix6*)
  # Find out which ABI we are using.
  echo '[#]line __oline__ "configure"' > conftest.$ac_ext
  if AC_TRY_EVAL(ac_compile); then
    case "`/usr/bin/file conftest.o`" in
    *32-bit*)
      LD="${LD-ld} -32"
      ;;
    *N32*)
      LD="${LD-ld} -n32"
      ;;
    *64-bit*)
      LD="${LD-ld} -64"
      ;;
    esac
  fi
  rm -rf conftest*
  ;;

*-*-sco3.2v5*)
  # On SCO OpenServer 5, we need -belf to get full-featured binaries.
  SAVE_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -belf"
  AC_CACHE_CHECK([whether the C compiler needs -belf], lt_cv_cc_needs_belf,
    [AC_TRY_LINK([],[],[lt_cv_cc_needs_belf=yes],[lt_cv_cc_needs_belf=no])])
  if test x"$lt_cv_cc_needs_belf" != x"yes"; then
    # this is probably gcc 2.8.0, egcs 1.0 or newer; no need for -belf
    CFLAGS="$SAVE_CFLAGS"
  fi
  ;;

ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[*-*-cygwin* | *-*-mingw*)
  AC_CHECK_TOOL(DLLTOOL, dlltool, false)
  AC_CHECK_TOOL(AS, as, false)
  AC_CHECK_TOOL(OBJDUMP, objdump, false)
  ;;
])
esac
])

# AC_LIBTOOL_DLOPEN - enable checks for dlopen support
AC_DEFUN(AC_LIBTOOL_DLOPEN, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])])

# AC_LIBTOOL_WIN32_DLL - declare package support for building win32 dll's
AC_DEFUN(AC_LIBTOOL_WIN32_DLL, [AC_BEFORE([$0], [AC_LIBTOOL_SETUP])])

# AC_ENABLE_SHARED - implement the --enable-shared flag
# Usage: AC_ENABLE_SHARED[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_SHARED, [dnl
define([AC_ENABLE_SHARED_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(shared,
changequote(<<, >>)dnl
<<  --enable-shared[=PKGS]  build shared libraries [default=>>AC_ENABLE_SHARED_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_shared=yes ;;
no) enable_shared=no ;;
*)
  enable_shared=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_shared=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_shared=AC_ENABLE_SHARED_DEFAULT)dnl
])

# AC_DISABLE_SHARED - set the default shared flag to --disable-shared
AC_DEFUN(AC_DISABLE_SHARED, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_SHARED(no)])

# AC_ENABLE_STATIC - implement the --enable-static flag
# Usage: AC_ENABLE_STATIC[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_STATIC, [dnl
define([AC_ENABLE_STATIC_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(static,
changequote(<<, >>)dnl
<<  --enable-static[=PKGS]  build static libraries [default=>>AC_ENABLE_STATIC_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_static=yes ;;
no) enable_static=no ;;
*)
  enable_static=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_static=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_static=AC_ENABLE_STATIC_DEFAULT)dnl
])

# AC_DISABLE_STATIC - set the default static flag to --disable-static
AC_DEFUN(AC_DISABLE_STATIC, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_STATIC(no)])


# AC_ENABLE_FAST_INSTALL - implement the --enable-fast-install flag
# Usage: AC_ENABLE_FAST_INSTALL[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_FAST_INSTALL, [dnl
define([AC_ENABLE_FAST_INSTALL_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(fast-install,
changequote(<<, >>)dnl
<<  --enable-fast-install[=PKGS]  optimize for fast installation [default=>>AC_ENABLE_FAST_INSTALL_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_fast_install=yes ;;
no) enable_fast_install=no ;;
*)
  enable_fast_install=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_fast_install=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_fast_install=AC_ENABLE_FAST_INSTALL_DEFAULT)dnl
])

# AC_ENABLE_FAST_INSTALL - set the default to --disable-fast-install
AC_DEFUN(AC_DISABLE_FAST_INSTALL, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_FAST_INSTALL(no)])

# AC_PROG_LD - find the path to the GNU or non-GNU linker
AC_DEFUN(AC_PROG_LD,
[AC_ARG_WITH(gnu-ld,
[  --with-gnu-ld           assume the C compiler uses GNU ld [default=no]],
test "$withval" = no || with_gnu_ld=yes, with_gnu_ld=no)
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
ac_prog=ld
if test "$ac_cv_prog_gcc" = yes; then
  # Check if gcc -print-prog-name=ld gives a path.
  AC_MSG_CHECKING([for ld used by GCC])
  ac_prog=`($CC -print-prog-name=ld) 2>&5`
  case "$ac_prog" in
    # Accept absolute paths.
changequote(,)dnl
    [\\/]* | [A-Za-z]:[\\/]*)
      re_direlt='/[^/][^/]*/\.\./'
changequote([,])dnl
      # Canonicalize the path of ld
      ac_prog=`echo $ac_prog| sed 's%\\\\%/%g'`
      while echo $ac_prog | grep "$re_direlt" > /dev/null 2>&1; do
	ac_prog=`echo $ac_prog| sed "s%$re_direlt%/%"`
      done
      test -z "$LD" && LD="$ac_prog"
      ;;
  "")
    # If it fails, then pretend we aren't using GCC.
    ac_prog=ld
    ;;
  *)
    # If it is relative, then search for the first ld in PATH.
    with_gnu_ld=unknown
    ;;
  esac
elif test "$with_gnu_ld" = yes; then
  AC_MSG_CHECKING([for GNU ld])
else
  AC_MSG_CHECKING([for non-GNU ld])
fi
AC_CACHE_VAL(ac_cv_path_LD,
[if test -z "$LD"; then
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_prog" || test -f "$ac_dir/$ac_prog$ac_exeext"; then
      ac_cv_path_LD="$ac_dir/$ac_prog"
      # Check to see if the program is GNU ld.  I'd rather use --version,
      # but apparently some GNU ld's only accept -v.
      # Break only if it was the GNU/non-GNU ld that we prefer.
      if "$ac_cv_path_LD" -v 2>&1 < /dev/null | egrep '(GNU|with BFD)' > /dev/null; then
	test "$with_gnu_ld" != no && break
      else
	test "$with_gnu_ld" != yes && break
      fi
    fi
  done
  IFS="$ac_save_ifs"
else
  ac_cv_path_LD="$LD" # Let the user override the test with a path.
fi])
LD="$ac_cv_path_LD"
if test -n "$LD"; then
  AC_MSG_RESULT($LD)
else
  AC_MSG_RESULT(no)
fi
test -z "$LD" && AC_MSG_ERROR([no acceptable ld found in \$PATH])
AC_PROG_LD_GNU
])

AC_DEFUN(AC_PROG_LD_GNU,
[AC_CACHE_CHECK([if the linker ($LD) is GNU ld], ac_cv_prog_gnu_ld,
[# I'd rather use --version here, but apparently some GNU ld's only accept -v.
if $LD -v 2>&1 </dev/null | egrep '(GNU|with BFD)' 1>&5; then
  ac_cv_prog_gnu_ld=yes
else
  ac_cv_prog_gnu_ld=no
fi])
])

# AC_PROG_NM - find the path to a BSD-compatible name lister
AC_DEFUN(AC_PROG_NM,
[AC_MSG_CHECKING([for BSD-compatible nm])
AC_CACHE_VAL(ac_cv_path_NM,
[if test -n "$NM"; then
  # Let the user override the test.
  ac_cv_path_NM="$NM"
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH /usr/ccs/bin /usr/ucb /bin; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/nm || test -f $ac_dir/nm$ac_exeext ; then
      # Check to see if the nm accepts a BSD-compat flag.
      # Adding the `sed 1q' prevents false positives on HP-UX, which says:
      #   nm: unknown option "B" ignored
      if ($ac_dir/nm -B /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -B"
	break
      elif ($ac_dir/nm -p /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -p"
	break
      else
	ac_cv_path_NM=${ac_cv_path_NM="$ac_dir/nm"} # keep the first match, but
	continue # so that we can try to find one that supports BSD flags
      fi
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_path_NM" && ac_cv_path_NM=nm
fi])
NM="$ac_cv_path_NM"
AC_MSG_RESULT([$NM])
])

# AC_CHECK_LIBM - check for math library
AC_DEFUN(AC_CHECK_LIBM,
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
LIBM=
case "$lt_target" in
*-*-beos* | *-*-cygwin*)
  # These system don't have libm
  ;;
*-ncr-sysv4.3*)
  AC_CHECK_LIB(mw, _mwvalidcheckl, LIBM="-lmw")
  AC_CHECK_LIB(m, main, LIBM="$LIBM -lm")
  ;;
*)
  AC_CHECK_LIB(m, main, LIBM="-lm")
  ;;
esac
])

# AC_LIBLTDL_CONVENIENCE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl convenience library, adds --enable-ltdl-convenience to
# the configure arguments.  Note that LIBLTDL is not AC_SUBSTed, nor
# is AC_CONFIG_SUBDIRS called.  If DIR is not provided, it is assumed
# to be `${top_builddir}/libltdl'.  Make sure you start DIR with
# '${top_builddir}/' (note the single quotes!) if your package is not
# flat, and, if you're not using automake, define top_builddir as
# appropriate in the Makefiles.
AC_DEFUN(AC_LIBLTDL_CONVENIENCE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  case "$enable_ltdl_convenience" in
  no) AC_MSG_ERROR([this package needs a convenience libltdl]) ;;
  "") enable_ltdl_convenience=yes
      ac_configure_args="$ac_configure_args --enable-ltdl-convenience" ;;
  esac
  LIBLTDL=ifelse($#,1,$1,['${top_builddir}/libltdl'])/libltdlc.la
  INCLTDL=ifelse($#,1,-I$1,['-I${top_builddir}/libltdl'])
])

# AC_LIBLTDL_INSTALLABLE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl installable library, and adds --enable-ltdl-install to
# the configure arguments.  Note that LIBLTDL is not AC_SUBSTed, nor
# is AC_CONFIG_SUBDIRS called.  If DIR is not provided, it is assumed
# to be `${top_builddir}/libltdl'.  Make sure you start DIR with
# '${top_builddir}/' (note the single quotes!) if your package is not
# flat, and, if you're not using automake, define top_builddir as
# appropriate in the Makefiles.
# In the future, this macro may have to be called after AC_PROG_LIBTOOL.
AC_DEFUN(AC_LIBLTDL_INSTALLABLE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  AC_CHECK_LIB(ltdl, main,
  [test x"$enable_ltdl_install" != xyes && enable_ltdl_install=no],
  [if test x"$enable_ltdl_install" = xno; then
     AC_MSG_WARN([libltdl not installed, but installation disabled])
   else
     enable_ltdl_install=yes
   fi
  ])
  if test x"$enable_ltdl_install" = x"yes"; then
    ac_configure_args="$ac_configure_args --enable-ltdl-install"
    LIBLTDL=ifelse($#,1,$1,['${top_builddir}/libltdl'])/libltdl.la
    INCLTDL=ifelse($#,1,-I$1,['-I${top_builddir}/libltdl'])
  else
    ac_configure_args="$ac_configure_args --enable-ltdl-install=no"
    LIBLTDL="-lltdl"
    INCLTDL=
  fi
])

dnl old names
AC_DEFUN(AM_PROG_LIBTOOL, [indir([AC_PROG_LIBTOOL])])dnl
AC_DEFUN(AM_ENABLE_SHARED, [indir([AC_ENABLE_SHARED], $@)])dnl
AC_DEFUN(AM_ENABLE_STATIC, [indir([AC_ENABLE_STATIC], $@)])dnl
AC_DEFUN(AM_DISABLE_SHARED, [indir([AC_DISABLE_SHARED], $@)])dnl
AC_DEFUN(AM_DISABLE_STATIC, [indir([AC_DISABLE_STATIC], $@)])dnl
AC_DEFUN(AM_PROG_LD, [indir([AC_PROG_LD])])dnl
AC_DEFUN(AM_PROG_NM, [indir([AC_PROG_NM])])dnl

dnl This is just to silence aclocal about the macro not being used
ifelse([AC_DISABLE_FAST_INSTALL])dnl
# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 5

AC_DEFUN(AM_WITH_NLS,
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    USE_INCLUDED_LIBINTL=no

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS)
      AC_MSG_CHECKING([whether included gettext is requested])
      AC_ARG_WITH(included-gettext,
        [  --with-included-gettext use the GNU gettext library included here],
        nls_cv_force_use_gnu_gettext=$withval,
        nls_cv_force_use_gnu_gettext=no)
      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)

      nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
      if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
        dnl User does not insist on using GNU NLS library.  Figure out what
        dnl to use.  If gettext or catgets are available (in this order) we
        dnl use this.  Else we have to fall back to GNU NLS library.
	dnl catgets is only used if permitted by option --with-catgets.
	nls_cv_header_intl=
	nls_cv_header_libgt=
	CATOBJEXT=NONE

	AC_CHECK_HEADER(libintl.h,
	  [AC_CACHE_CHECK([for gettext in libc], gt_cv_func_gettext_libc,
	    [AC_TRY_LINK([#include <libintl.h>], [return (int) gettext ("")],
	       gt_cv_func_gettext_libc=yes, gt_cv_func_gettext_libc=no)])

	   if test "$gt_cv_func_gettext_libc" != "yes"; then
	     AC_CHECK_LIB(intl, bindtextdomain,
	       [AC_CACHE_CHECK([for gettext in libintl],
		 gt_cv_func_gettext_libintl,
		 [AC_CHECK_LIB(intl, gettext,
dnl ============== A fix is here! ======================
dnl -lintl was not added to the LIBS variable in this case
		  [gt_cv_func_gettext_libintl=yes
		   INTLLIBS="-lintl"],
dnl ==== end of fix
		  gt_cv_func_gettext_libintl=no)],
		 gt_cv_func_gettext_libintl=no)])
	   fi

	   if test "$gt_cv_func_gettext_libc" = "yes" \
	      || test "$gt_cv_func_gettext_libintl" = "yes"; then
	      AC_DEFINE(HAVE_GETTEXT)
	      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
		[test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)dnl
	      if test "$MSGFMT" != "no"; then
		AC_CHECK_FUNCS(dcgettext)
		AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
		AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		AC_TRY_LINK(, [extern int _nl_msg_cat_cntr;
			       return _nl_msg_cat_cntr],
		  [CATOBJEXT=.gmo
		   DATADIRNAME=share],
		  [CATOBJEXT=.mo
		   DATADIRNAME=lib])
		INSTOBJEXT=.mo
	      fi
	    fi
	])

        if test "$CATOBJEXT" = "NONE"; then
	  AC_MSG_CHECKING([whether catgets can be used])
	  AC_ARG_WITH(catgets,
	    [  --with-catgets          use catgets functions if available],
	    nls_cv_use_catgets=$withval, nls_cv_use_catgets=no)
	  AC_MSG_RESULT($nls_cv_use_catgets)

	  if test "$nls_cv_use_catgets" = "yes"; then
	    dnl No gettext in C library.  Try catgets next.
	    AC_CHECK_LIB(i, main)
	    AC_CHECK_FUNC(catgets,
	      [AC_DEFINE(HAVE_CATGETS)
	       INTLOBJS="\$(CATOBJS)"
	       AC_PATH_PROG(GENCAT, gencat, no)dnl
	       if test "$GENCAT" != "no"; then
		 AC_PATH_PROG(GMSGFMT, gmsgfmt, no)
		 if test "$GMSGFMT" = "no"; then
		   AM_PATH_PROG_WITH_TEST(GMSGFMT, msgfmt,
		    [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)
		 fi
		 AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		   [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		 USE_INCLUDED_LIBINTL=yes
		 CATOBJEXT=.cat
		 INSTOBJEXT=.cat
		 DATADIRNAME=lib
		 INTLDEPS='$(top_builddir)/intl/libintl.a'
		 INTLLIBS=$INTLDEPS
		 LIBS=`echo $LIBS | sed -e 's/-lintl//'`
		 nls_cv_header_intl=intl/libintl.h
		 nls_cv_header_libgt=intl/libgettext.h
	       fi])
	  fi
        fi

        if test "$CATOBJEXT" = "NONE"; then
	  dnl Neither gettext nor catgets in included in the C library.
	  dnl Fall back on GNU gettext library.
	  nls_cv_use_gnu_gettext=yes
        fi
      fi

      if test "$nls_cv_use_gnu_gettext" = "yes"; then
        dnl Mark actions used to generate GNU NLS library.
        INTLOBJS="\$(GETTOBJS)"
        AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
        AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
        AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
        AC_SUBST(MSGFMT)
	USE_INCLUDED_LIBINTL=yes
        CATOBJEXT=.gmo
        INSTOBJEXT=.mo
        DATADIRNAME=share
	INTLDEPS='$(top_builddir)/intl/libintl.a'
	INTLLIBS=$INTLDEPS
	LIBS=`echo $LIBS | sed -e 's/-lintl//'`
        nls_cv_header_intl=intl/libintl.h
        nls_cv_header_libgt=intl/libgettext.h
      fi

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi

      # We need to process the po/ directory.
      POSUB=po
    else
      DATADIRNAME=share
      nls_cv_header_intl=intl/libintl.h
      nls_cv_header_libgt=intl/libgettext.h
    fi
    AC_LINK_FILES($nls_cv_header_libgt, $nls_cv_header_intl)
    AC_OUTPUT_COMMANDS(
     [case "$CONFIG_FILES" in *po/Makefile.in*)
        sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile
      esac])


    # If this is used in GNU gettext we have to set USE_NLS to `yes'
    # because some of the sources are only built for this goal.
    if test "$PACKAGE" = gettext; then
      USE_NLS=yes
      USE_INCLUDED_LIBINTL=yes
    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done

    dnl Make all variables we use known to autoconf.
    AC_SUBST(USE_INCLUDED_LIBINTL)
    AC_SUBST(CATALOGS)
    AC_SUBST(CATOBJEXT)
    AC_SUBST(DATADIRNAME)
    AC_SUBST(GMOFILES)
    AC_SUBST(INSTOBJEXT)
    AC_SUBST(INTLDEPS)
    AC_SUBST(INTLLIBS)
    AC_SUBST(INTLOBJS)
    AC_SUBST(POFILES)
    AC_SUBST(POSUB)
  ])

AC_DEFUN(AM_GNU_GETTEXT,
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h sys/param.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
strdup __argz_count __argz_stringify __argz_next])

   if test "${ac_cv_func_stpcpy+set}" != "set"; then
     AC_CHECK_FUNCS(stpcpy)
   fi
   if test "${ac_cv_func_stpcpy}" = "yes"; then
     AC_DEFINE(HAVE_STPCPY)
   fi

   AM_LC_MESSAGES
   AM_WITH_NLS

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
dnl ============== Fix is here! ======================
dnl Some sh do not like substitution in bounds of for loops
dnl    for lang in ${LINGUAS=$ALL_LINGUAS}; do
    tmplinguas=${LINGUAS=$ALL_LINGUAS}
    for lang in ${tmplinguas}; do
dnl ==== end of fix
         case "$ALL_LINGUAS" in
          *$lang*) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl The reference to <locale.h> in the installed <libintl.h> file
   dnl must be resolved because we cannot expect the users of this
   dnl to define HAVE_LOCALE_H.
   if test $ac_cv_header_locale_h = yes; then
     INCLUDE_LOCALE_H="#include <locale.h>"
   else
     INCLUDE_LOCALE_H="\
/* The system does not provide the header <locale.h>.  Take care yourself.  */"
   fi
   AC_SUBST(INCLUDE_LOCALE_H)

   dnl Determine which catalog format we have (if any is needed)
   dnl For now we know about two different formats:
   dnl   Linux libc-5 and the normal X/Open format
   test -d intl || mkdir intl
   if test "$CATOBJEXT" = ".cat"; then
     AC_CHECK_HEADER(linux/version.h, msgformat=linux, msgformat=xopen)

     dnl Transform the SED scripts while copying because some dumb SEDs
     dnl cannot handle comments.
     sed -e '/^#/d' $srcdir/intl/$msgformat-msg.sed > intl/po2msg.sed
   fi
   dnl po2tbl.sed is always needed.
   sed -e '/^#.*[^\\]$/d' -e '/^#$/d' \
     $srcdir/intl/po2tbl.sed.in > intl/po2tbl.sed

   dnl In the intl/Makefile.in we have a special dependency which makes
   dnl only sense for gettext.  We comment this out for non-gettext
   dnl packages.
   if test "$PACKAGE" = "gettext"; then
     GT_NO="#NO#"
     GT_YES=
   else
     GT_NO=
     GT_YES="#YES#"
   fi
   AC_SUBST(GT_NO)
   AC_SUBST(GT_YES)

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but ($top_srcdir).
   dnl Try to locate is.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl *** For now the libtool support in intl/Makefile is not for real.
   l=
   AC_SUBST(l)

   dnl Generate list of files to be processed by xgettext which will
   dnl be included in po/Makefile.
   test -d po || mkdir po
   if test "x$srcdir" != "x."; then
     if test "x`echo $srcdir | sed 's@/.*@@'`" = "x"; then
       posrcprefix="$srcdir/"
     else
       posrcprefix="../$srcdir/"
     fi
   else
     posrcprefix="../"
   fi
   rm -f po/POTFILES
   sed -e "/^#/d" -e "/^\$/d" -e "s,.*,	$posrcprefix& \\\\," -e "\$s/\(.*\) \\\\/\1/" \
	< $srcdir/po/POTFILES.in > po/POTFILES
  ])
# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

AC_DEFUN(AM_LC_MESSAGES,
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES)
    fi
  fi])
# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN(AM_PATH_PROG_WITH_TEST,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])
# Configure paths for Libsigc++
# Based on Gtk-- script by Erik Andersen and Tero Pulkkinen 

dnl Test for Libsigc++, and define SIGC_CFLAGS and SIGC_LIBS
dnl   to be used as follows:
dnl AM_PATH_SIGC(MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN(AM_PATH_SIGC,
[dnl 
dnl Get the cflags and libraries from the sigc-config script
dnl

dnl
dnl Prefix options
dnl
AC_ARG_WITH(sigc-prefix,
[  --with-sigc-prefix=PREFIX
                          Prefix where libsigc++ is installed (optional)]
, sigc_config_prefix="$withval", sigc_config_prefix="")

AC_ARG_WITH(sigc-exec-prefix,
[  --with-sigc-exec-prefix=PREFIX 
                          Exec prefix where  libsigc++ is installed (optional)]
, sigc_config_exec_prefix="$withval", sigc_config_exec_prefix="")

AC_ARG_ENABLE(sigctest, 
[  --disable-sigctest     Do not try to compile and run a test libsigc++ 
                          program],
, enable_sigctest=yes)

dnl
dnl Prefix handling
dnl
  if test x$sigc_config_exec_prefix != x ; then
     sigc_config_args="$sigc_config_args --exec-prefix=$sigc_config_exec_prefix"
     if test x${SIGC_CONFIG+set} != xset ; then
        SIGC_CONFIG=$sigc_config_exec_prefix/bin/sigc-config
     fi
  fi
  if test x$sigc_config_prefix != x ; then
     sigc_config_args="$sigc_config_args --prefix=$sigc_config_prefix"
     if test x${SIGC_CONFIG+set} != xset ; then
        SIGC_CONFIG=$sigc_config_prefix/bin/sigc-config
     fi
  fi

dnl
dnl See if sigc-config is alive
dnl
  AC_PATH_PROG(SIGC_CONFIG, sigc-config, no)
  sigc_version_min=$1

dnl
dnl  Version check
dnl
  AC_MSG_CHECKING(for libsigc++ - version >= $sigc_version_min)
  no_sigc=""
  if test "$SIGC_CONFIG" = "no" ; then
    no_sigc=yes
  else
    sigc_version=`$SIGC_CONFIG --version`

    SIGC_CFLAGS=`$SIGC_CONFIG $sigc_config_args --cflags`
    SIGC_LIBS=`$SIGC_CONFIG $sigc_config_args --libs`
    SIGC_MACROS=`$SIGC_CONFIG $sigc_config_args --libs`

    sigc_major_version=`echo $sigc_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sigc_minor_version=`echo $sigc_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sigc_micro_version=`echo $sigc_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    sigc_major_min=`echo $sigc_version_min | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sigc_minor_min=`echo $sigc_version_min | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sigc_micro_min=`echo $sigc_version_min | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    sigc_version_proper=`expr \
        $sigc_major_version \> $sigc_major_min \| \
        $sigc_major_version \= $sigc_major_min \& \
        $sigc_minor_version \> $sigc_minor_min \| \
        $sigc_major_version \= $sigc_major_min \& \
        $sigc_minor_version \= $sigc_minor_min \& \
        $sigc_micro_version \>= $sigc_micro_min `

    if test "$sigc_version_proper" = "1" ; then
      AC_MSG_RESULT([$sigc_major_version.$sigc_minor_version.$sigc_micro_version])
    else
      AC_MSG_RESULT(no)
      no_sigc=yes
    fi

    if test "X$no_sigc" = "Xyes" ; then
      enable_sigctest=no
    fi

    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS

dnl
dnl
dnl
    if test "x$enable_sigctest" = "xyes" ; then
      AC_MSG_CHECKING(if libsigc++ sane)
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $SIGC_CFLAGS"
      LIBS="$LIBS $SIGC_LIBS"

      rm -f conf.sigctest
      AC_TRY_RUN([
#include <stdio.h>
#include <sigc++/signal_system.h>

#ifdef SIGC_CXX_NAMESPACES
using namespace SigC;
#endif

int foo1(int i) 
  {
   return 1;
  }

int main(int argc,char **argv)
  {
   if (sigc_major_version!=$sigc_major_version ||
       sigc_minor_version!=$sigc_minor_version ||
       sigc_micro_version!=$sigc_micro_version)
     { printf("(%d.%d.%d) ",
         sigc_major_version,sigc_minor_version,sigc_micro_version);
       return 1;
     }
   Signal1<int,int> sig1;
   sig1.connect(slot(foo1));
   sig1(1);
   return 0;
  }

],[
  AC_MSG_RESULT(yes)
],[
  AC_MSG_RESULT(no)
  no_sigc=yes
]
,[echo $ac_n "cross compiling; assumed OK... $ac_c"])

       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi

  dnl
  dnl
  if test "x$no_sigc" = x ; then
     ifelse([$2], , :, [$2])     
  else
     SIGC_CFLAGS=""
     SIGC_LIBS=""
     SIGC_MACROS=""
     ifelse([$3], , :, [$3])
  fi

  AC_LANG_RESTORE

  AC_SUBST(SIGC_CFLAGS)
  AC_SUBST(SIGC_LIBS)
  AC_SUBST(SIGC_MACROS)
])

# Configure paths and libs when using KDE GUI
##    -*- shell-script -*-

##    This file is part of the KDE libraries/packages
##    Copyright (C) 1997 Janos Farkas (chexum@shadow.banki.hu)
##              (C) 1997 Stephan Kulow (coolo@kde.org)

##    This file is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.

##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.

##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
##    Boston, MA 02111-1307, USA.        

## IMPORTANT NOTE:
## Please do not modify this file unless you expect your modifications to be
## carried into every other module in the repository. If you decide that you
## really want to modify it, contact coolo@kde.org mentioning that you have
## and that the modified file should be committed to every module.
##
## Single-module modifications are best placed in configure.in for kdelibs
## and kdebase or configure.in.1 if present.


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

AC_DEFUN(KDE_FIND_PATH,
[
   AC_MSG_CHECKING([for $1])
   AC_CACHE_VAL(kde_cv_path_$1,
   [
     kde_cv_path_$1="NONE"
     if test -n "$$2"; then
        kde_cv_path_$1="$$2";
     else
        dirs="$3"
        kde_save_IFS=$IFS
        IFS=':'
        for dir in $PATH; do
          dirs="$dirs $dir"
        done
        IFS=$kde_save_IFS
 
        for dir in $dirs; do
          if test -x "$dir/$1"; then
            if test -n "$5"
            then
              evalstr="$dir/$1 $5 2>&1 "
              if eval $evalstr; then
                kde_cv_path_$1="$dir/$1"
                break
              fi
            else
                kde_cv_path_$1="$dir/$1"
                break
            fi
          fi
        done
 
     fi
 
   ])
 
   if test -z "$kde_cv_path_$1" || test "$kde_cv_path_$1" = "NONE"; then
      AC_MSG_RESULT(not found)
      $4
   else
     AC_MSG_RESULT($kde_cv_path_$1)
     $2=$kde_cv_path_$1
   fi
])
AC_DEFUN(KDE_MOC_ERROR_MESSAGE,
[
    AC_MSG_ERROR([No Qt meta object compiler (moc) found!
Please check whether you installed Qt correctly.
You need to have a running moc binary.
configure tried to run $ac_cv_path_moc and the test didn't
succeed. If configure shouldn't have tried this one, set
the environment variable MOC to the right one before running
configure.
])
])

dnl ------------------------------------------------------------------------
dnl Find the meta object compiler in the PATH, in $QTDIR/bin, and some
dnl more usual places
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN(AC_PATH_QT_MOC,
[
   KDE_FIND_PATH(moc, MOC, [$ac_qt_bindir $QTDIR/bin \
            /usr/bin /usr/X11R6/bin /usr/lib/qt/bin \
            /usr/local/qt/bin], [KDE_MOC_ERROR_MESSAGE])
 
   if test -z "$MOC"; then
     if test -n "$ac_cv_path_moc"; then
       output=`eval "$ac_cv_path_moc --help 2>&1 | sed -e '1q' | grep Qt"`
     fi
     echo "configure:__oline__: tried to call $ac_cv_path_moc --help 2>&1 | sed -e '1q' | grep Qt" >&AC_FD_CC
     echo "configure:__oline__: moc output: $output" >&AC_FD_CC
 
     if test -z "$output"; then
       KDE_MOC_ERROR_MESSAGE
    fi
   fi
 
   AC_SUBST(MOC)
])

AC_DEFUN(KDE_REPLACE_ROOT,
  $1=`echo "$$1" | sed -e "s#^/#\$\{install_root\}/#"`
)

AC_DEFUN(AC_CREATE_KFSSTND,
[
AC_REQUIRE([AC_CHECK_RPATH])

if test "$1" = "default"; then

  AC_MSG_CHECKING(for KDE paths)

  if test -z "$kde_htmldir"; then
    kde_htmldir="\$(prefix)/share/doc/HTML"
  fi
  if test -z "$kde_appsdir"; then
    kde_appsdir="\$(prefix)/share/applnk"
  fi
  if test -z "$kde_icondir"; then
    kde_icondir="\$(prefix)/share/icons"
  fi
  if test -z "$kde_sounddir"; then
    kde_sounddir="\$(prefix)/share/sounds"
  fi
  if test -z "$kde_datadir"; then
    kde_datadir="\$(prefix)/share/apps"
  fi
  if test -z "$kde_locale"; then
    kde_locale="\$(prefix)/share/locale"
  fi
  if test -z "$kde_cgidir"; then
    kde_cgidir="\$(exec_prefix)/cgi-bin"
  fi
  if test -z "$kde_confdir"; then
    kde_confdir="\$(prefix)/share/config"
  fi
  if test -z "$kde_mimedir"; then
    kde_mimedir="\$(prefix)/share/mimelnk"
  fi
  if test -z "$kde_toolbardir"; then
    kde_toolbardir="\$(prefix)/share/toolbar"
  fi
  if test -z "$kde_wallpaperdir"; then
    kde_wallpaperdir="\$(prefix)/share/wallpapers"
  fi
  if test -z "$kde_bindir"; then
    kde_bindir="\$(exec_prefix)/bin"
  fi
  if test -z "$kde_partsdir"; then
    kde_partsdir="\$(exec_prefix)/parts"
  fi
  AC_MSG_RESULT(defaults)	

else 

AC_CACHE_VAL(kde_cv_all_paths,
[
AC_MSG_CHECKING([for kde headers installed])
AC_LANG_CPLUSPLUS
cat > conftest.$ac_ext <<EOF
#include <stdio.h>
#include "confdefs.h"
#include <kapp.h>

int main() {
printf("kde_htmldir=\\"%s\\"\n", KApplication::kde_htmldir().data());
printf("kde_appsdir=\\"%s\\"\n", KApplication::kde_appsdir().data());
printf("kde_icondir=\\"%s\\"\n", KApplication::kde_icondir().data());
printf("kde_sounddir=\\"%s\\"\n", KApplication::kde_sounddir().data());
printf("kde_datadir=\\"%s\\"\n", KApplication::kde_datadir().data());
printf("kde_locale=\\"%s\\"\n", KApplication::kde_localedir().data());
printf("kde_cgidir=\\"%s\\"\n", KApplication::kde_cgidir().data());
printf("kde_confdir=\\"%s\\"\n", KApplication::kde_configdir().data());
printf("kde_mimedir=\\"%s\\"\n", KApplication::kde_mimedir().data());
printf("kde_toolbardir=\\"%s\\"\n", KApplication::kde_toolbardir().data());
printf("kde_wallpaperdir=\\"%s\\"\n", KApplication::kde_wallpaperdir().data());
printf("kde_bindir=\\"%s\\"\n", KApplication::kde_bindir().data());
printf("kde_partsdir=\\"%s\\"\n", KApplication::kde_partsdir().data());
return 0;
}
EOF

if test -n "$kde_libraries"; then
  KDE_TEST_RPATH="-rpath $kde_libraries"
fi

if test -n "$qt_libraries"; then
  KDE_TEST_RPATH="$KDE_TEST_RPATH -rpath $qt_libraries"
fi

if test -n "$x_libraries"; then
  KDE_TEST_RPATH="$KDE_TEST_RPATH -rpath $x_libraries"
fi

KDE_TEST_RPATH="$KDE_TEST_RPATH $KDE_EXTRA_RPATH"

ac_compile='${CXX-g++} -c $CXXFLAGS $all_includes $CPPFLAGS conftest.$ac_ext'
if AC_TRY_EVAL(ac_compile); then
  AC_MSG_RESULT(yes)
else
  AC_MSG_ERROR([your system is not able to compile a small KDE application!
Check, if you installed the KDE header files correctly.])
fi

AC_MSG_CHECKING([for kde libraries installed])
ac_link='/bin/sh ./libtool --mode=link ${CXX-g++} -o conftest $CXXFLAGS $all_includes $CPPFLAGS $LDFLAGS $all_libraries conftest.$ac_ext $LIBS -lkdecore -lqt -lXext -lX11 $LIBSOCKET $KDE_TEST_RPATH 1>&5'

if AC_TRY_EVAL(ac_link) && test -s conftest; then 
  AC_MSG_RESULT(yes)
else
  AC_MSG_ERROR([your system fails at linking a small KDE application!
Check, if your compiler is installed correctly and if you have used the
same compiler to compile Qt and kdelibs as you did use now])
fi

AC_MSG_CHECKING([for KDE paths])
if eval `./conftest 2>&5`; then
  AC_MSG_RESULT(done)
else
  AC_MSG_RESULT(problems)
fi

./conftest 2> /dev/null >&5 # make an echo for config.log
kde_have_all_paths=yes
AC_LANG_CPLUSPLUS

if test "$kde_have_all_paths" = "yes"; then
kde_cv_all_paths="kde_have_all_paths=\"yes\" \
	kde_htmldir=\"$kde_htmldir\" \
	kde_appsdir=\"$kde_appsdir\" \
	kde_icondir=\"$kde_icondir\" \
	kde_sounddir=\"$kde_sounddir\" \
	kde_datadir=\"$kde_datadir\" \
	kde_locale=\"$kde_locale\" \
	kde_cgidir=\"$kde_cgidir\" \
	kde_confdir=\"$kde_confdir\" \
	kde_mimedir=\"$kde_mimedir\" \
	kde_toolbardir=\"$kde_toolbardir\" \
	kde_wallpaperdir=\"$kde_wallpaperdir\" \
	kde_bindir=\"$kde_bindir\" \
	kde_partsdir=\"$kde_partsdir\""
fi
rm -fr conftest*

])

eval "$kde_cv_all_paths"

if test -z "$kde_htmldir" || test -z "$kde_appsdir" ||
   test -z "$kde_icondir" || test -z "$kde_sounddir" ||
   test -z "$kde_datadir" || test -z "$kde_locale"  ||
   test -z "$kde_cgidir"  || test -z "$kde_confdir" ||
   test -z "$kde_mimedir" || test -z "$kde_toolbardir" ||
   test -z "$kde_wallpaperdir" || test -z "$kde_bindir" ||
   test -z "$kde_partsdir" || test "$kde_have_all_paths" != "yes"; then
  kde_have_all_paths=no
  AC_MSG_ERROR([configure could not run a little KDE program to test the environment. 
Since it had compiled and linked before, it must be a strange problem on your system.
Look at config.log for details. If you are not able to fix this, please contact Stephan Kulow <coolo@kde.org>.])
fi

if test -n "$install_root"; then
  KDE_REPLACE_ROOT(kde_htmldir)
  KDE_REPLACE_ROOT(kde_appsdir)
  KDE_REPLACE_ROOT(kde_icondir)
  KDE_REPLACE_ROOT(kde_sounddir)
  KDE_REPLACE_ROOT(kde_datadir)
  KDE_REPLACE_ROOT(kde_locale)
  KDE_REPLACE_ROOT(kde_cgidir)
  KDE_REPLACE_ROOT(kde_confdir)
  KDE_REPLACE_ROOT(kde_mimedir)
  KDE_REPLACE_ROOT(kde_toolbardir)
  KDE_REPLACE_ROOT(kde_wallpaperdir)
  KDE_REPLACE_ROOT(kde_bindir)
  KDE_REPLACE_ROOT(kde_partsdir)
  AC_SUBST(install_root)
fi

fi

bindir=$kde_bindir

])

AC_DEFUN(AC_SUBST_KFSSTND,
[
AC_SUBST(kde_htmldir)
AC_SUBST(kde_appsdir)
AC_SUBST(kde_icondir)
AC_SUBST(kde_sounddir)
kde_minidir="$kde_icondir/mini"
AC_SUBST(kde_minidir)
AC_SUBST(kde_datadir)
AC_SUBST(kde_locale)
AC_SUBST(kde_cgidir)
AC_SUBST(kde_confdir)
AC_SUBST(kde_mimedir)
AC_SUBST(kde_toolbardir)
AC_SUBST(kde_wallpaperdir)
AC_SUBST(kde_bindir)
AC_SUBST(kde_partsdir)
])

AC_DEFUN(KDE_MISC_TESTS,
[
   AC_LANG_C
   dnl Checks for libraries. 
   AC_CHECK_LIB(compat, main, [LIBCOMPAT="-lcompat"]) dnl for FreeBSD
   AC_SUBST(LIBCOMPAT)
   AC_CHECK_LIB(crypt, main, [LIBCRYPT="-lcrypt"]) dnl for BSD
   AC_SUBST(LIBCRYPT)
   AC_CHECK_LIB(dnet, dnet_ntoa, [X_EXTRA_LIBS="$X_EXTRA_LIBS -ldnet"])
   if test $ac_cv_lib_dnet_dnet_ntoa = no; then
      AC_CHECK_LIB(dnet_stub, dnet_ntoa,
        [X_EXTRA_LIBS="$X_EXTRA_LIBS -ldnet_stub"])
   fi
   AC_CHECK_FUNC(gethostbyname)
   if test $ac_cv_func_gethostbyname = no; then
     AC_CHECK_LIB(nsl, gethostbyname, X_EXTRA_LIBS="$X_EXTRA_LIBS -lnsl")
   fi
   AC_CHECK_FUNC(connect)
   if test $ac_cv_func_connect = no; then
      AC_CHECK_LIB(socket, connect, X_EXTRA_LIBS="-lsocket $X_EXTRA_LIBS", ,
        $X_EXTRA_LIBS)
   fi

   AC_CHECK_FUNC(remove)
   if test $ac_cv_func_remove = no; then
      AC_CHECK_LIB(posix, remove, X_EXTRA_LIBS="$X_EXTRA_LIBS -lposix")
   fi
 
   # BSDI BSD/OS 2.1 needs -lipc for XOpenDisplay.
   AC_CHECK_FUNC(shmat)
   if test $ac_cv_func_shmat = no; then
     AC_CHECK_LIB(ipc, shmat, X_EXTRA_LIBS="$X_EXTRA_LIBS -lipc")
   fi

   LIBSOCKET=$X_EXTRA_LIBS
   AC_SUBST(LIBSOCKET)
   AC_SUBST(X_EXTRA_LIBS)
   AC_CHECK_LIB(ucb, killpg, [LIBUCB="-lucb"]) dnl for Solaris2.4
   AC_SUBST(LIBUCB)

   case $host in  dnl this *is* LynxOS specific
   *-*-lynxos* )
        AC_MSG_CHECKING([LynxOS header file wrappers])
        [CFLAGS="$CFLAGS -D__NO_INCLUDE_WARN__"]
        AC_MSG_RESULT(disabled)
        AC_CHECK_LIB(bsd, gethostbyname, [LIBSOCKET="-lbsd"]) dnl for LynxOS
         ;;
    esac

])

## ------------------------------------------------------------------------
## Find the header files and libraries for X-Windows. Extended the 
## macro AC_PATH_X
## ------------------------------------------------------------------------
##
AC_DEFUN(K_PATH_X,
[
AC_MSG_CHECKING(for X)
AC_CACHE_VAL(ac_cv_have_x,
[# One or both of the vars are not set, and there is no cached value.
ac_x_includes=NO ac_x_libraries=NO
AC_PATH_X_DIRECT
AC_PATH_X_XMKMF
if test "$ac_x_includes" = NO || test "$ac_x_libraries" = NO; then
  AC_MSG_ERROR([Can't find X includes. Please check your installation and add the correct paths!])
else
  # Record where we found X for the cache.
  ac_cv_have_x="have_x=yes \
                ac_x_includes=$ac_x_includes ac_x_libraries=$ac_x_libraries"
fi])dnl
eval "$ac_cv_have_x"
 
if test "$have_x" != yes; then
  AC_MSG_RESULT($have_x)
  no_x=yes
else
  # If each of the values was on the command line, it overrides each guess.
  test "x$x_includes" = xNONE && x_includes=$ac_x_includes
  test "x$x_libraries" = xNONE && x_libraries=$ac_x_libraries
  # Update the cache value to reflect the command line values.
  ac_cv_have_x="have_x=yes \
                ac_x_includes=$x_includes ac_x_libraries=$x_libraries"
  AC_MSG_RESULT([libraries $x_libraries, headers $x_includes])
fi

if test -z "$x_includes" || test "x$x_includes" = xNONE; then
  X_INCLUDES=""
  x_includes="."; dnl better than nothing :-
 else
  X_INCLUDES="-I$x_includes"
fi

if test -z "$x_libraries" || test "x$x_libraries" = xNONE; then
  X_LDFLAGS=""
  x_libraries="/usr/lib"; dnl better than nothing :-
 else
  X_LDFLAGS="-L$x_libraries"
fi
all_includes="$all_includes $X_INCLUDES"  
all_libraries="$all_libraries $X_LDFLAGS"  

AC_SUBST(X_INCLUDES)
AC_SUBST(X_LDFLAGS)
AC_SUBST(x_libraries)
AC_SUBST(x_includes)
])

AC_DEFUN(KDE_PRINT_QT_PROGRAM,
[
AC_LANG_CPLUSPLUS
cat > conftest.$ac_ext <<EOF
#include "confdefs.h"
#include <qmovie.h>
#include <qapplication.h>
int main() {
  QMovie m;
  m.setSpeed(20);
  return 0;
}
EOF
])

AC_DEFUN(KDE_CHECK_QT_DIRECT,
[
AC_MSG_CHECKING([if Qt compiles without flags])
AC_CACHE_VAL(kde_cv_qt_direct,
[
ac_LD_LIBRARY_PATH_safe=$LD_LIBRARY_PATH
ac_LIBRARY_PATH="$LIBRARY_PATH"
ac_cxxflags_safe="$CXXFLAGS"
ac_ldflags_safe="$LDFLAGS"
ac_libs_safe="$LIBS"

CXXFLAGS="$CXXFLAGS -I$qt_includes"
LDFLAGS="$X_LDFLAGS"
LIBS="-lqt -lXext -lX11 $LIBSOCKET"
LD_LIBRARY_PATH=
export LD_LIBRARY_PATH
LIBRARY_PATH=
export LIBRARY_PATH

KDE_PRINT_QT_PROGRAM

if AC_TRY_EVAL(ac_link) && test -s conftest; then
  kde_cv_qt_direct="yes"
else
  kde_cv_qt_direct="no"
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
fi
rm -f conftest*
CXXFLAGS="$ac_cxxflags_safe"
LDFLAGS="$ac_ldflags_safe"
LIBS="$ac_libs_safe"

LD_LIBRARY_PATH="$ac_LD_LIBRARY_PATH_safe"
export LD_LIBRARY_PATH
LIBRARY_PATH="$ac_LIBRARY_PATH"
export LIBRARY_PATH
])

if test "$kde_cv_qt_direct" = "yes"; then
  AC_MSG_RESULT(yes)
  $1
else
  AC_MSG_RESULT(no)
  $2
fi
])

## ------------------------------------------------------------------------
## Try to find the Qt headers and libraries.
## $(QT_LDFLAGS) will be -Lqtliblocation (if needed)
## and $(QT_INCLUDES) will be -Iqthdrlocation (if needed)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_PATH_QT_1_3,
[
AC_REQUIRE([K_PATH_X])

AC_MSG_CHECKING([for Qt])
ac_qt_includes=NO ac_qt_libraries=NO ac_qt_bindir=NO
qt_libraries=""
qt_includes=""
AC_ARG_WITH(qt-dir,
    [  --with-qt-dir           where the root of qt is installed ],
    [  ac_qt_includes="$withval"/include
       ac_qt_libraries="$withval"/lib
       ac_qt_bindir="$withval"/bin
    ])

AC_ARG_WITH(qt-includes,
    [  --with-qt-includes      where the Qt includes are. ],
    [  
       ac_qt_includes="$withval"
    ])
    
kde_qt_libs_given=no

AC_ARG_WITH(qt-libraries,
    [  --with-qt-libraries     where the Qt library is installed.],
    [  ac_qt_libraries="$withval"
       kde_qt_libs_given=yes
    ])

if test "$ac_qt_includes" = NO || test "$ac_qt_libraries" = NO; then

AC_CACHE_VAL(ac_cv_have_qt,
[#try to guess Qt locations

qt_incdirs="$ac_qt_includes /usr/lib/qt/include /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt $x_includes $QTINC"
test -n "$QTDIR" && qt_incdirs="$QTDIR/include $QTDIR $qt_incdirs"
AC_FIND_FILE(qmovie.h, $qt_incdirs, qt_incdir)
ac_qt_includes="$qt_incdir"

if test ! "$ac_qt_libraries" = "NO"; then
  qt_libdirs="$ac_qt_libraries"
fi

qt_libdirs="$qt_libdirs /usr/lib/qt/lib /usr/X11R6/lib /usr/lib /usr/local/qt/lib /usr/lib/qt $x_libraries $QTLIB"
test -n "$QTDIR" && qt_libdirs="$QTDIR/lib $QTDIR $qt_libdirs"

test=NONE
qt_libdir=NONE
for dir in $qt_libdirs; do
  try="ls -1 $dir/libqt*"
  if test=`eval $try 2> /dev/null`; then qt_libdir=$dir; break; else echo "tried $dir" >&AC_FD_CC ; fi
done

dnl AC_FIND_FILE(libqt.so libqt.so.1.40 libqt.so.1.41 libqt.so.1 libqt.a libqt.sl, $qt_libdirs, qt_libdir)
ac_qt_libraries="$qt_libdir"

ac_cxxflags_safe="$CXXFLAGS"
ac_ldflags_safe="$LDFLAGS"
ac_libs_safe="$LIBS"

CXXFLAGS="$CXXFLAGS -I$qt_incdir"
LDFLAGS="-L$qt_libdir $X_LDFLAGS"
LIBS="$LIBS -lqt -lXext -lX11 $LIBSOCKET"

KDE_PRINT_QT_PROGRAM

if AC_TRY_EVAL(ac_link) && test -s conftest; then
  rm -f conftest*
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
  ac_qt_libraries="NO"
fi
rm -f conftest*
CXXFLAGS="$ac_cxxflags_safe"
LDFLAGS="$ac_ldflags_safe"
LIBS="$ac_libs_safe"

if test "$ac_qt_includes" = NO || test "$ac_qt_libraries" = NO; then
  ac_cv_have_qt="have_qt=no"
  ac_qt_notfound=""
  if test "$ac_qt_includes" = NO; then
    if test "$ac_qt_libraries" = NO; then
      ac_qt_notfound="(headers and libraries)";
    else
      ac_qt_notfound="(headers)";
    fi
  else
    ac_qt_notfound="(libraries)";
  fi

  AC_MSG_ERROR([Qt-1.4 $ac_qt_notfound not found. Please check your installation! ]);
else
  have_qt="yes"
fi
])
else
  have_qt="yes"
fi

eval "$ac_cv_have_qt"

if test "$have_qt" != yes; then
  AC_MSG_RESULT([$have_qt]);
else
  ac_cv_have_qt="have_qt=yes \
    ac_qt_includes=$ac_qt_includes ac_qt_libraries=$ac_qt_libraries"
  AC_MSG_RESULT([libraries $ac_qt_libraries, headers $ac_qt_includes])
  
  qt_libraries="$ac_qt_libraries"
  qt_includes="$ac_qt_includes"
fi

if test ! "$kde_qt_libs_given" = "yes"; then
KDE_CHECK_QT_DIRECT(qt_libraries= ,[])
fi

AC_SUBST(qt_libraries)
AC_SUBST(qt_includes)

if test "$qt_includes" = "$x_includes" || test -z "$qt_includes"; then
 QT_INCLUDES="";
else
 QT_INCLUDES="-I$qt_includes"
 all_includes="$QT_INCLUDES $all_includes"
fi

if test "$qt_libraries" = "$x_libraries" || test -z "$qt_libraries"; then
 QT_LDFLAGS=""
else
 QT_LDFLAGS="-L$qt_libraries"
 all_libraries="$QT_LDFLAGS $all_libraries"
fi

AC_SUBST(QT_INCLUDES)
AC_SUBST(QT_LDFLAGS)
AC_PATH_QT_MOC
])

AC_DEFUN(AC_PATH_QT,
[
AC_PATH_QT_1_3
])

## ------------------------------------------------------------------------
## Now, the same with KDE
## $(KDE_LDFLAGS) will be the kdeliblocation (if needed)
## and $(kde_includes) will be the kdehdrlocation (if needed)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_BASE_PATH_KDE,
[
AC_REQUIRE([KDE_MISC_TESTS])
AC_REQUIRE([AC_PATH_QT])dnl
AC_MSG_CHECKING([for KDE])

if test "${prefix}" != NONE; then
  kde_includes=${prefix}/include
  ac_kde_includes=$prefix/include

  if test "${exec_prefix}" != NONE; then
    kde_libraries=${exec_prefix}/lib
    ac_kde_libraries=$exec_prefix/lib
  else
    kde_libraries=${prefix}/lib
    ac_kde_libraries=$prefix/lib
  fi
else
  ac_kde_includes=
  ac_kde_libraries=
  kde_libraries=""
  kde_includes=""
fi

AC_CACHE_VAL(ac_cv_have_kde,
[#try to guess kde locations

if test -z "$1"; then

kde_incdirs="$ac_kde_includes /usr/lib/kde/include /usr/local/kde/include /usr/kde/include /usr/include/kde /usr/include /opt/kde/include $x_includes $qt_includes"
test -n "$KDEDIR" && kde_incdirs="$KDEDIR/include $KDEDIR $kde_incdirs"
AC_FIND_FILE(ksock.h, $kde_incdirs, kde_incdir)
ac_kde_includes="$kde_incdir"

if test -n "$ac_kde_includes" && test ! -r "$ac_kde_includes/ksock.h"; then
  AC_MSG_ERROR([
in the prefix, you've chosen, are no kde headers installed. This will fail.
So, check this please and use another prefix!])
fi

kde_libdirs="$ac_kde_libraries /usr/lib/kde/lib /usr/local/kde/lib /usr/kde/lib /usr/lib/kde /usr/lib /usr/X11R6/lib /opt/kde/lib /usr/X11R6/kde/lib"
test -n "$KDEDIR" && kde_libdirs="$KDEDIR/lib $KDEDIR $kde_libdirs"
AC_FIND_FILE(libkdecore.la, $kde_libdirs, kde_libdir)
ac_kde_libraries="$kde_libdir"

if test -n "$ac_kde_libraries" && test ! -r "$ac_kde_libraries/libkdecore.la"; then
AC_MSG_ERROR([
in the prefix, you've chosen $ac_kde_libraries, are no kde libraries installed. This will fail.
So, check this please and use another prefix!])
fi
ac_kde_libraries="$kde_libdir"

if test "$ac_kde_includes" = NO || test "$ac_kde_libraries" = NO; then
  ac_cv_have_kde="have_kde=no"
else
  ac_cv_have_kde="have_kde=yes \
    ac_kde_includes=$ac_kde_includes ac_kde_libraries=$ac_kde_libraries"
fi

else dnl test -z $1 
  
  ac_cv_have_kde="have_kde=no"

fi
])dnl

eval "$ac_cv_have_kde"

if test "$have_kde" != "yes"; then
 if test "${prefix}" = NONE; then
  ac_kde_prefix="$ac_default_prefix"
 else
  ac_kde_prefix="$prefix"
 fi
 if test "$exec_prefix" = NONE; then
  ac_kde_exec_prefix="$ac_kde_prefix"
  AC_MSG_RESULT([will be installed in $ac_kde_prefix])
 else
  ac_kde_exec_prefix="$exec_prefix"
  AC_MSG_RESULT([will be installed in $ac_kde_prefix and $ac_kde_exec_prefix])
 fi

 kde_libraries="${ac_kde_exec_prefix}/lib"
 kde_includes=${ac_kde_prefix}/include

else
  ac_cv_have_kde="have_kde=yes \
    ac_kde_includes=$ac_kde_includes ac_kde_libraries=$ac_kde_libraries"
  AC_MSG_RESULT([libraries $ac_kde_libraries, headers $ac_kde_includes])
  
  kde_libraries="$ac_kde_libraries"
  kde_includes="$ac_kde_includes"
fi
AC_SUBST(kde_libraries)
AC_SUBST(kde_includes)

if test "$kde_includes" = "$x_includes" || test "$kde_includes" = "$qt_includes" ; then
 KDE_INCLUDES=""
else
 KDE_INCLUDES="-I$kde_includes"
 all_includes="$KDE_INCLUDES $all_includes"
fi

if test "$kde_libraries" = "$x_libraries" || test "$kde_libraries" = "$qt_libraries" ; then
 KDE_LDFLAGS=""
else
 KDE_LDFLAGS="-L$kde_libraries"
 all_libraries="$KDE_LDFLAGS $all_libraries"
fi

AC_SUBST(KDE_LDFLAGS)
AC_SUBST(KDE_INCLUDES)

KDE_CHECK_EXTRA_LIBS

AC_SUBST(all_includes)
AC_SUBST(all_libraries)

])

AC_DEFUN(KDE_CHECK_EXTRA_LIBS,
[
AC_MSG_CHECKING(for extra includes)
AC_ARG_WITH(extra-includes, [  --with-extra-includes   adds non standard include paths], 
  kde_use_extra_includes="$withval",
  kde_use_extra_includes=NONE
)
if test -n "$kde_use_extra_includes" && \
   test "$kde_use_extra_includes" != "NONE"; then

   ac_save_ifs=$IFS
   IFS=':'
   for dir in $kde_use_extra_includes; do
     all_includes="$all_includes -I$dir"
     USER_INCLUDES="$USER_INCLUDES -I$dir"
   done
   IFS=$ac_save_ifs
   kde_use_extra_includes="added"
else
   kde_use_extra_includes="no"
fi

AC_MSG_RESULT($kde_use_extra_includes)

AC_MSG_CHECKING(for extra libs)
AC_ARG_WITH(extra-libs, [  --with-extra-libs       adds non standard library paths], 
  kde_use_extra_libs=$withval,
  kde_use_extra_libs=NONE
)
if test -n "$kde_use_extra_libs" && \
   test "$kde_use_extra_libs" != "NONE"; then

   ac_save_ifs=$IFS
   IFS=':'
   for dir in $kde_use_extra_libs; do
     all_libraries="$all_libraries -L$dir"
     KDE_EXTRA_RPATH="$KDE_EXTRA_RPATH -rpath $dir"
     USER_LDFLAGS="$USER_LDFLAGS -L$dir"
   done
   IFS=$ac_save_ifs
   kde_use_extra_libs="added"
else
   kde_use_extra_libs="no"
fi

AC_MSG_RESULT($kde_use_extra_libs)

])

AC_DEFUN(KDE_CHECK_KIMGIO,
[
   AC_REQUIRE([AC_FIND_TIFF])
   AC_REQUIRE([AC_FIND_JPEG]) 
   AC_REQUIRE([AC_FIND_PNG]) 

   LIB_KIMGIO='-lkimgio $(LIBJPEG) $(LIBTIFF) $(LIBPNG) -lm'
   AC_SUBST(LIB_KIMGIO)
   LIB_KHTMLW='-lkhtmlw $(LIB_KIMGIO) -ljscript'
   AC_SUBST(LIB_KHTMLW)
   LIB_KHTML='-lkhtml $(LIB_KIMGIO) -ljscript'
   AC_SUBST(LIB_KHTML)
])

AC_DEFUN(KDE_CREATE_LIBS_ALIASES,
[
   AC_REQUIRE([KDE_MISC_TESTS])

   KDE_LIBS='-lqt -lkdecore -lkdeui -lkfm -lkfile'
   AC_SUBST(KDE_LIBS)
#   LIB_X11='-lX11 $(LIBSOCKET)'
#   AC_SUBST(LIB_X11)
#   LIB_QT='-lqt $(LIB_X11)'
#   AC_SUBST(LIB_QT)
#   LIB_KDECORE='-lkdecore -lXext $(LIB_QT)'
#   AC_SUBST(LIB_KDECORE)
#   LIB_KDEUI='-lkdeui $(LIB_KDECORE)'
#   AC_SUBST(LIB_KDEUI)
#   LIB_KFM='-lkfm $(LIB_KDECORE)'
#   AC_SUBST(LIB_KFM)
#   LIB_KFILE='-lkfile $(LIB_KFM) $(LIB_KDEUI)'
#   AC_SUBST(LIB_KFILE)
])

AC_DEFUN(AC_PATH_KDE,
[
  AC_BASE_PATH_KDE
  AC_ARG_ENABLE(path-check, [  --disable-path-check    don't try to find out, where to install],
  [
  if test "$enableval" = "no"; 
    then ac_use_path_checking="default"
    else ac_use_path_checking=""
  fi
  ], [ac_use_path_checking=""]
  )

  AC_ARG_WITH(install-root, [  --with-install-root     the root, where to install to [default=/]],
  [
  if test "$withval" = "no";
    then kde_install_root="";
    else kde_install_root=$withval;
  fi
  ], [kde_install_root=""]
  )
  
  if test -n "$kde_install_root"; then
     install_root="$kde_install_root"
  else
     install_root=
  fi

  AC_CREATE_KFSSTND($ac_use_path_checking)

  AC_SUBST_KFSSTND
  KDE_CREATE_LIBS_ALIASES
])

dnl slightly changed version of AC_CHECK_FUNC(setenv)
AC_DEFUN(AC_CHECK_SETENV,
[AC_MSG_CHECKING([for setenv])
AC_CACHE_VAL(ac_cv_func_setenv,
[AC_LANG_C
AC_TRY_LINK(
dnl Don't include <ctype.h> because on OSF/1 3.0 it includes <sys/types.h>
dnl which includes <sys/select.h> which contains a prototype for
dnl select.  Similarly for bzero.
[#include <assert.h>
]ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
#include <stdlib.h>
], [
/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_$1) || defined (__stub___$1)
choke me
#else
setenv("TEST", "alle", 1);
#endif
], eval "ac_cv_func_setenv=yes", eval "ac_cv_func_setenv=no")])

if test "$ac_cv_func_setenv" = "yes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_FUNC_SETENV)
else
  AC_MSG_RESULT(no)
fi
])

AC_DEFUN(AC_CHECK_GETDOMAINNAME,
[
AC_LANG_CPLUSPLUS
save_CXXFLAGS="$CXXFLAGS"
if test "$GCC" = "yes"; then
CXXFLAGS="$CXXFLAGS -pedantic-errors"
fi
AC_MSG_CHECKING(for getdomainname)
AC_CACHE_VAL(ac_cv_func_getdomainname,
[
AC_TRY_COMPILE([
#include <stdlib.h>
#include <unistd.h>
],
[
char buffer[200];
getdomainname(buffer, 200);
],
ac_cv_func_getdomainname=yes,
ac_cv_func_getdomainname=no)
])
AC_MSG_RESULT($ac_cv_func_getdomainname)
if eval "test \"`echo `$ac_cv_func_getdomainname\" = yes"; then
  AC_DEFINE(HAVE_GETDOMAINNAME)
fi
CXXFLAGS="$save_CXXFLAGS"
])

AC_DEFUN(AC_CHECK_GETHOSTNAME,
[
AC_LANG_CPLUSPLUS
save_CXXFLAGS="$CXXFLAGS"
if test "$GCC" = "yes"; then
CXXFLAGS="$CXXFLAGS -pedantic-errors"
fi

AC_MSG_CHECKING([for gethostname])
AC_CACHE_VAL(ac_cv_func_gethostname,
[
AC_TRY_COMPILE([
#include <stdlib.h>
#include <unistd.h>
],
[
char buffer[200];
gethostname(buffer, 200);
],
ac_cv_func_gethostname=yes,
ac_cv_func_gethostname=no)
])
AC_MSG_RESULT($ac_cv_func_gethostname)
if eval "test \"`echo `$ac_cv_func_gethostname\" = yes"; then
  AC_DEFINE(HAVE_GETHOSTNAME)
fi
CXXFLAGS="$save_CXXFLAGS"
])

AC_DEFUN(AC_CHECK_USLEEP,
[
AC_LANG_CPLUSPLUS

AC_MSG_CHECKING([for usleep])
AC_CACHE_VAL(ac_cv_func_usleep,
[
ac_libs_safe="$LIBS"
LIBS="$LIBS $LIBUCB"
AC_TRY_LINK([
#include <stdlib.h>
#include <unistd.h>
],
[
usleep(200);
],
ac_cv_func_usleep=yes,
ac_cv_func_usleep=no)
])
AC_MSG_RESULT($ac_cv_func_usleep)
if eval "test \"`echo `$ac_cv_func_usleep\" = yes"; then
  AC_DEFINE(HAVE_USLEEP)
fi
LIBS="$ac_libs_safe"
])

AC_DEFUN(AC_FIND_GIF,
   [AC_MSG_CHECKING([for giflib])
AC_CACHE_VAL(ac_cv_lib_gif,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -lgif -lX11 $LIBSOCKET"
AC_TRY_LINK(dnl
[
#ifdef __cplusplus
extern "C" {
#endif
int GifLastError(void);
#ifdef __cplusplus
}
#endif
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
],
            [return GifLastError();],
            eval "ac_cv_lib_gif=yes",
            eval "ac_cv_lib_gif=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo $ac_cv_lib_gif`\" = yes"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_LIBGIF)
else
  AC_MSG_ERROR(You need giflib23. Please install the kdesupport package)
fi
])

AC_DEFUN(AC_FIND_JPEG,
   [AC_MSG_CHECKING([for jpeglib])
AC_CACHE_VAL(ac_cv_lib_jpeg,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -ljpeg -lm"
AC_TRY_LINK(
[/* Override any gcc2 internal prototype to avoid an error.  */
struct jpeg_decompress_struct;
typedef struct jpeg_decompress_struct * j_decompress_ptr;
typedef int size_t;
#ifdef __cplusplus
extern "C" {
#endif
    void jpeg_CreateDecompress(j_decompress_ptr cinfo,
                                    int version, size_t structsize);
#ifdef __cplusplus
}
#endif
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
],
            [jpeg_CreateDecompress(0L, 0, 0);],
            eval "ac_cv_lib_jpeg=-ljpeg",
            eval "ac_cv_lib_jpeg=no")
LIBS="$ac_save_LIBS"

dnl what to do, if the normal way fails:
if eval "test \"`echo $ac_cv_lib_jpeg`\" = no"; then
	if test -f "$kde_libraries/libjpeg.so"; then
	   test -f ./libjpegkde.so || $LN_S $kde_libraries/libjpeg.so ./libjpegkde.so
	   ac_cv_lib_jpeg="-L\${topdir} -ljpegkde"
	else if test -f "$kde_libraries/libjpeg.sl"; then
	   test -f ./libjpegkde.sl ||$LN_S $kde_libraries/libjpeg.sl ./libjpegkde.sl
	   ac_cv_lib_jpeg="-L\${topdir} -ljpegkde"	
	else if test -f "$kde_libraries/libjpeg.a"; then
	   test -f ./libjpegkde.a || $LN_S $kde_libraries/libjpeg.a ./libjpegkde.a
	   ac_cv_lib_jpeg="-L\${topdir} -ljpegkde"
        else
	  AC_MSG_ERROR([
You need jpeglib6a. Please install the kdesupport package.
If you have already installed kdesupport, you may have an
old libjpeg somewhere. 
In this case copy $KDEDIR/lib/libjpeg* to /usr/lib.
])
	fi
      fi
   fi
fi
])dnl
if eval "test ! \"`echo $ac_cv_lib_jpeg`\" = no"; then
  LIBJPEG="$ac_cv_lib_jpeg"
  AC_SUBST(LIBJPEG)
  AC_MSG_RESULT($ac_cv_lib_jpeg)
  AC_DEFINE_UNQUOTED(HAVE_LIBJPEG)
fi
])

AC_DEFUN(AC_FIND_ZLIB,
[
AC_MSG_CHECKING([for libz])
AC_CACHE_VAL(ac_cv_lib_z,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -lz $LIBSOCKET"
AC_TRY_LINK(dnl
[
#include<zlib.h>
],
            [return (zlibVersion() == ZLIB_VERSION); ],
            eval "ac_cv_lib_z='-lz'",
            eval "ac_cv_lib_z=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test ! \"`echo $ac_cv_lib_z`\" = no"; then
dnl  AC_DEFINE_UNQUOTED(HAVE_LIBZ)
  LIBZ="$ac_cv_lib_z"
  AC_SUBST(LIBZ)
  AC_MSG_RESULT($ac_cv_lib_z)
else
  AC_MSG_RESULT(no)
  LIBZ=""
  AC_SUBST(LIBZ)
fi
])

AC_DEFUN(AC_FIND_TIFF,
[
AC_REQUIRE([AC_FIND_ZLIB])
AC_REQUIRE([AC_FIND_JPEG])
AC_MSG_CHECKING([for libtiff])
AC_CACHE_VAL(ac_cv_lib_tiff,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -ltiff $LIBJPEG $LIBZ -lX11 $LIBSOCKET"
AC_TRY_LINK(dnl
[
#include<tiffio.h>
],
            [return (TIFFOpen( "", "r") == 0); ],
            eval "ac_cv_lib_tiff='-ltiff $LIBJPEG $LIBZ'",
            eval "ac_cv_lib_tiff=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test ! \"`echo $ac_cv_lib_tiff`\" = no"; then
  AC_DEFINE_UNQUOTED(HAVE_LIBTIFF)
  LIBTIFF="$ac_cv_lib_tiff"
  AC_SUBST(LIBTIFF)
  AC_MSG_RESULT($ac_cv_lib_tiff)
else
  AC_MSG_RESULT(no)
  LIBTIFF=""
  AC_SUBST(LIBTIFF)
fi
])


AC_DEFUN(AC_FIND_PNG,
[
AC_REQUIRE([AC_FIND_ZLIB])
AC_MSG_CHECKING([for libpng])
AC_CACHE_VAL(ac_cv_lib_png,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -lpng $LIBZ -lm -lX11 $LIBSOCKET"
AC_LANG_C
AC_TRY_LINK(dnl
    [
    #include<png.h>
    ],
    [
    png_structp png_ptr = png_create_read_struct(  // image ptr
		PNG_LIBPNG_VER_STRING, 0, 0, 0 );
    return( png_ptr != 0 ); 
    ],
    eval "ac_cv_lib_png='-lpng $LIBZ -lm'",
    eval "ac_cv_lib_png=no")
    LIBS="$ac_save_LIBS"
])dnl
if eval "test ! \"`echo $ac_cv_lib_png`\" = no"; then
  AC_DEFINE_UNQUOTED(HAVE_LIBPNG)
  LIBPNG="$ac_cv_lib_png"
  AC_SUBST(LIBPNG)
  AC_MSG_RESULT($ac_cv_lib_png)
else
  AC_MSG_RESULT(no)
  LIBPNG=""
  AC_SUBST(LIBPNG)
fi
])

AC_DEFUN(AC_CHECK_GNU_EXTENSIONS,
[
AC_MSG_CHECKING(if you need GNU extensions)
AC_CACHE_VAL(ac_cv_gnu_extensions,
[
cat > conftest.c << EOF
#include <features.h>

#ifdef __GNU_LIBRARY__
yes
#endif
EOF

if (eval "$ac_cpp conftest.c") 2>&5 |
  egrep "yes" >/dev/null 2>&1; then
  rm -rf conftest*
  ac_cv_gnu_extensions=yes
else
  ac_cv_gnu_extensions=no
fi
])

AC_MSG_RESULT($ac_cv_gnu_extensions)
if test "$ac_cv_gnu_extensions" = "yes"; then
  AC_DEFINE_UNQUOTED(_GNU_SOURCE)
fi
])

AC_DEFUN(AC_CHECK_COMPILERS,
[
  dnl this is somehow a fat lie, but prevents other macros from double checking
  AC_PROVIDE([AC_PROG_CC])
  AC_PROVIDE([AC_PROG_CPP])
  AC_ARG_ENABLE(debug,[  --enable-debug 	  creates debugging code [default=no]],
  [ 
   if test $enableval = "no"; dnl 
     then ac_use_debug_code="no"
     else ac_use_debug_code="yes"
   fi
  ], [ac_use_debug_code="no"])

  AC_ARG_ENABLE(strict,[  --enable-strict         compiles with strict compiler options (may not work!)],
   [ 
    if test $enableval = "no"; then 
         ac_use_strict_options="no"
       else 
         ac_use_strict_options="yes"
    fi
   ], [ac_use_strict_options="no"])

dnl this was AC_PROG_CC. I had to include it manualy, since I had to patch it
  AC_MSG_CHECKING(for a C-Compiler)
  dnl if there is one, print out. if not, don't matter
  AC_MSG_RESULT($CC) 
 
  if test -z "$CC"; then AC_CHECK_PROG(CC, gcc, gcc) fi
  if test -z "$CC"; then AC_CHECK_PROG(CC, cc, cc, , , /usr/ucb/cc) fi
  if test -z "$CC"; then AC_CHECK_PROG(CC, xlc, xlc) fi
  test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])

  AC_PROG_CC_WORKS
  AC_PROG_CC_GNU

  if test $ac_cv_prog_gcc = yes; then
    GCC=yes
  else
    GCC=
  fi

  if test -z "$CFLAGS"; then
    if test "$ac_use_debug_code" = "yes"; then
      AC_PROG_CC_G
      if test $ac_cv_prog_cc_g = yes; then
        CFLAGS="-g"
      fi
    else
      if test "$GCC" = "yes"; then
        CFLAGS="-O2"
      else
        CFLAGS=""
      fi
    fi

    if test "$GCC" = "yes"; then
     CFLAGS="$CFLAGS -Wall"

     if test "$ac_use_strict_options" = "yes"; then
	CFLAGS="$CFLAGS -W -ansi -pedantic"     
     fi
    fi

  fi

  case "$host" in 
  *-*-sysv4.2uw*) CFLAGS="$CFLAGS -D_UNIXWARE";;
  esac

  if test -z "$LDFLAGS" && test "$ac_use_debug_code" = "no" && test "$GCC" = "yes"; then
     LDFLAGS="-s"
  fi


dnl this is AC_PROG_CPP. I had to include it here, since autoconf checks
dnl dependecies between AC_PROG_CPP and AC_PROG_CC (or is it automake?)

  AC_MSG_CHECKING(how to run the C preprocessor)
  # On Suns, sometimes $CPP names a directory.
  if test -n "$CPP" && test -d "$CPP"; then
    CPP=
  fi
  if test -z "$CPP"; then
  AC_CACHE_VAL(ac_cv_prog_CPP,
  [  # This must be in double quotes, not single quotes, because CPP may get
    # substituted into the Makefile and "${CC-cc}" will confuse make.
    CPP="${CC-cc} -E"
    # On the NeXT, cc -E runs the code through the compiler's parser,
    # not just through cpp.
    dnl Use a header file that comes with gcc, so configuring glibc    
    dnl with a fresh cross-compiler works.
    AC_TRY_CPP([#include <assert.h>
    Syntax Error], ,
    CPP="${CC-cc} -E -traditional-cpp"
    AC_TRY_CPP([#include <assert.h>
    Syntax Error], , CPP=/lib/cpp))
    ac_cv_prog_CPP="$CPP"])dnl
    CPP="$ac_cv_prog_CPP"
  else
    ac_cv_prog_CPP="$CPP"
  fi
  AC_MSG_RESULT($CPP)
  AC_SUBST(CPP)dnl


  AC_MSG_CHECKING(for a C++-Compiler)
  dnl if there is one, print out. if not, don't matter
  AC_MSG_RESULT($CXX) 
 
  if test -z "$CXX"; then AC_CHECK_PROG(CXX, g++, g++) fi
  if test -z "$CXX"; then AC_CHECK_PROG(CXX, CC, CC) fi
  if test -z "$CXX"; then AC_CHECK_PROG(CXX, xlC, xlC) fi
  if test -z "$CXX"; then AC_CHECK_PROG(CXX, DCC, DCC) fi
  test -z "$CXX" && AC_MSG_ERROR([no acceptable C++-compiler found in \$PATH])

  AC_PROG_CXX_WORKS
  AC_PROG_CXX_GNU

  if test $ac_cv_prog_gxx = yes; then
    GXX=yes
  else
    AC_MSG_CHECKING(whether we are using SPARC CC)
    GXX=
    cat > conftest.C << EOF
#ifdef __SUNPRO_CC
   yes;
#endif
EOF

    ac_try="$CXX -E conftest.C"
    if { (eval echo configure:__online__: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; } | egrep yes >/dev/null 2>&1; then
      ac_cv_prog_CC=yes
    else
      ac_cv_prog_CC=no
    fi
    AC_MSG_RESULT($ac_cv_prog_CC)
  fi

  if test -z "$CXXFLAGS"; then 
    if test "$ac_use_debug_code" = "yes"; then
      AC_PROG_CXX_G
      if test $ac_cv_prog_cxx_g = yes; then
        CXXFLAGS="-g"
      fi
      if test "$ac_cv_prog_CC" = "yes"; then
        CXXFLAGS="$CXXFLAGS -pto"
      fi
    else
      if test "$GXX" = "yes"; then
         CXXFLAGS="-O2"
      else
         if test "$ac_cv_prog_CC" = "yes"; then
            CXXFLAGS="-pto -O2"
         else
            CXXFLAGS=""
         fi
      fi
    fi

    if test "$GXX" = "yes"; then
       CXXFLAGS="$CXXFLAGS -Wall"
 
       if test "$ac_use_strict_options" = "yes"; then
	CXXFLAGS="$CXXFLAGS -W -ansi -Wtraditional  -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Woverloaded-virtual -Wbad-function-cast  -Wsynth"
       fi

       if test "$kde_very_strict" = "yes"; then
         CXXFLAGS="$CXXFLAGS -Wold-style-cast -Wshadow -Wredundant-decls -Wconversion"
       fi
    fi
  fi  

    case "$host" in
      *-*-sysv4.2uw*) CXXFLAGS="$CXXFLAGS -D_UNIXWARE";;
    esac    

])

dnl just a wrapper to clean up configure.in
AC_DEFUN(KDE_PROG_LIBTOOL,
[
AC_REQUIRE([AM_ENABLE_SHARED])
AC_REQUIRE([AM_ENABLE_STATIC])
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

AC_DEFUN(KDE_DO_IT_ALL,
[
AC_PREFIX_DEFAULT(${KDEDIR:-/usr/local/kde})
KDE_PROG_LIBTOOL
AM_KDE_WITH_NLS
AC_PATH_KDE
])

AC_DEFUN(AC_CHECK_RPATH,
[
AC_MSG_CHECKING(for rpath)
AC_ARG_ENABLE(rpath,
      [  --disable-rpath         do not use the rpath feature of ld],
      USE_RPATH=$enableval, USE_RPATH=yes)
if test -z "$KDE_RPATH" && test "$USE_RPATH" = "yes"; then

  KDE_RPATH="-rpath \$(kde_libraries)"

  if test -n "$qt_libraries"; then
    KDE_RPATH="$KDE_RPATH -rpath \$(qt_libraries)"
  fi
  dnl $x_libraries is set to /usr/lib in case
  if test -n "$X_LDFLAGS"; then 
    KDE_RPATH="$KDE_RPATH -rpath \$(x_libraries)"
  fi
  if test -n "$KDE_EXTRA_RPATH"; then
    KDE_RPATH="$KDE_RPATH \$(KDE_EXTRA_RPATH)"
  fi
fi 
AC_SUBST(KDE_EXTRA_RPATH)
AC_SUBST(KDE_RPATH)
AC_MSG_RESULT($USE_RPATH)
])

dnl This is a merge of some macros out of the gettext aclocal.m4
dnl since we don't need anything, I took the things we need
AC_DEFUN(AM_KDE_WITH_NLS,
  [AC_MSG_CHECKING([whether NLS is requested])
    AC_LANG_CPLUSPLUS
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS)

      AM_PATH_PROG_WITH_TEST_KDE(MSGFMT, msgfmt, 
         [test -n "`$ac_dir/$ac_word --version 2>&1 | grep 'GNU gettext'`"], msgfmt)
      AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)

      if test -z "`$MSGFMT --version 2>&1 | grep 'GNU gettext'`"; then
        AC_MSG_RESULT([found msgfmt program is not GNU msgfmt; ignore it])
        msgfmt=":"
      fi
      AC_SUBST(MSGFMT)

      AM_PATH_PROG_WITH_TEST_KDE(XGETTEXT, xgettext,
	[test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext programs is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi
     AC_SUBST(XGETTEXT)
    fi

  ])

# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.

# serial 1
# Stephan Kulow: I appended a _KDE against name conflicts

dnl AM_PATH_PROG_WITH_TEST_KDE(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN(AM_PATH_PROG_WITH_TEST_KDE,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])


dnl From Jim Meyering.
dnl FIXME: migrate into libit.

AC_DEFUN(AM_FUNC_OBSTACK,
[AC_CACHE_CHECK([for obstacks], am_cv_func_obstack,
 [AC_TRY_LINK([#include "obstack.h"],
	      [struct obstack *mem;obstack_free(mem,(char *) 0)],
	      am_cv_func_obstack=yes,
	      am_cv_func_obstack=no)])
 if test $am_cv_func_obstack = yes; then
   AC_DEFINE(HAVE_OBSTACK)
 else
   LIBOBJS="$LIBOBJS obstack.o"
 fi
])

dnl From Jim Meyering.  Use this if you use the GNU error.[ch].
dnl FIXME: Migrate into libit

AC_DEFUN(AM_FUNC_ERROR_AT_LINE,
[AC_CACHE_CHECK([for error_at_line], am_cv_lib_error_at_line,
 [AC_TRY_LINK([],[error_at_line(0, 0, "", 0, "");],
              am_cv_lib_error_at_line=yes,
	      am_cv_lib_error_at_line=no)])
 if test $am_cv_lib_error_at_line = no; then
   LIBOBJS="$LIBOBJS error.o"
 fi
 AC_SUBST(LIBOBJS)dnl
])

# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.

# serial 1
# Stephan Kulow: I put a KDE in it to avoid name conflicts

AC_DEFUN(AM_KDE_GNU_GETTEXT,
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl
   AC_REQUIRE([AM_KDE_WITH_NLS])dnl
   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h values.h alloca.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
__argz_count __argz_stringify __argz_next stpcpy])

   AM_LC_MESSAGES

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       for lang in ${LINGUAS=$ALL_LINGUAS}; do
         case "$ALL_LINGUAS" in
          *$lang*) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

  ])

AC_DEFUN(AC_HAVE_XPM,
 [AC_REQUIRE_CPP()dnl

 test -z "$XPM_LDFLAGS" && XPM_LDFLAGS=
 test -z "$XPM_INCLUDE" && XPM_INCLUDE=

 AC_ARG_WITH(xpm, [  --without-xpm           disable color pixmap XPM tests],
	xpm_test=$withval, xpm_test="yes")
 if test "x$xpm_test" = xno; then
   ac_cv_have_xpm=no
 else
   AC_MSG_CHECKING(for XPM)
   AC_CACHE_VAL(ac_cv_have_xpm,
   [
    AC_LANG_C
    ac_save_ldflags="$LDFLAGS"
    ac_save_cflags="$CFLAGS"
    LDFLAGS="$LDFLAGS $XPM_LDFLAGS $all_libraries -lXpm -lX11 -lXext $LIBSOCKET"
    CFLAGS="$CFLAGS $X_INCLUDES"
    test ! -z "$XPM_INCLUDE" && CFLAGS="-I$XPM_INCLUDE $CFLAGS"
    AC_TRY_LINK([#include <X11/xpm.h>],[],
	ac_cv_have_xpm="yes",ac_cv_have_xpm="no")
    LDFLAGS="$ac_save_ldflags"
    CFLAGS="$ac_save_cflags"
   ])dnl
 
  if test "$ac_cv_have_xpm" = no; then
    AC_MSG_RESULT(no)
    XPM_LDFLAGS=""
    XPMINC=""
    $2
  else
    AC_DEFINE(HAVE_XPM)
    if test "$XPM_LDFLAGS" = ""; then
       XPMLIB="-lXpm"
    else
       XPMLIB="-L$XPM_LDFLAGS -lXpm"
    fi
    if test "$XPM_INCLUDE" = ""; then
       XPMINC=""
    else
       XPMINC="-I$XPM_INCLUDE"
    fi
    AC_MSG_RESULT(yes)
    $1
  fi
 fi
 AC_SUBST(XPMINC)
 AC_SUBST(XPMLIB)
]) 

AC_DEFUN(AC_HAVE_GL,
 [AC_REQUIRE_CPP()dnl

 test -z "$GL_LDFLAGS" && GL_LDFLAGS=
 test -z "$GL_INCLUDE" && GL_INCLUDE=

 AC_ARG_WITH(gl, [  --without-gl            disable 3D GL modes],
	gl_test=$withval, gl_test="yes")
 if test "x$gl_test" = xno; then
   ac_cv_have_gl=no
 else
   AC_MSG_CHECKING(for GL)
   AC_CACHE_VAL(ac_cv_have_gl,
   [
    AC_LANG_C
    ac_save_ldflags="$LDFLAGS"
    ac_save_cflags="$CFLAGS"
    LDFLAGS="$LDFLAGS $GL_LDFLAGS $all_libraries -lMesaGL -lMesaGLU -lX11 -lXext -lm $LIBSOCKET"
    CFLAGS="$CFLAGS $X_INCLUDES"
    test ! -z "$GL_INCLUDE" && CFLAGS="-I$GL_INCLUDE $CFLAGS"
    AC_TRY_LINK([],[],
	ac_cv_have_gl="yes",ac_cv_have_gl="no")
    LDFLAGS="$ac_save_ldflags"
    CFLAGS="$ac_save_cflags"
   ])dnl
 
  if test "$ac_cv_have_gl" = no; then
    AC_MSG_RESULT(no)
    GL_LDFLAGS=""
    GLINC=""
    $2
  else
    AC_DEFINE(HAVE_GL)
    if test "$GL_LDFLAGS" = ""; then
       GLLIB="-lMesaGL -lMesaGLU"
    else
       GLLIB="-L$GL_LDFLAGS -lMesaGL -lMesaGLU"
    fi
    if test "$GL_INCLUDE" = ""; then
       GLINC=""
    else
       GLINC="-I$GL_INCLUDE"
    fi
    AC_MSG_RESULT(yes)
    $1
  fi
 fi
 AC_SUBST(GLINC)
 AC_SUBST(GLLIB)
]) 

 dnl PAM pam
 
 dnl Should test for PAM (Pluggable Authentication Modules)
 AC_DEFUN(AC_PATH_PAM_DIRECT,
 [
 test -z "$pam_direct_test_library" && pam_direct_test_library=pam
 test -z "$pam_direct_test_include" && pam_direct_test_include=security/pam_appl.h
 
   for ac_dir in               \
                               \
     /usr/local/include        \
     /usr/include              \
     /usr/unsupported/include  \
     /opt/include              \
     /usr/pam/include          \
     /usr/local/pam/include    \
     /usr/lib/pam/include      \
 			      \
     $extra_include            \
     ; \
   do
     if test -r "$ac_dir/$pam_direct_test_include"; then
       no_pam= ac_pam_includes=$ac_dir
       break
     fi
   done
 
 # First see if replacing the include by lib works.
 for ac_dir in `echo "$ac_pam_includes" | sed s/include/lib/` \
                           \
     /lib                  \
     /usr/lib              \
     /usr/local/lib        \
     /usr/unsupported/lib  \
     /lib/security         \
     /usr/security/lib     \
     $extra_lib            \
     ; \
 do
   for ac_extension in a so sl; do
     if test -r $ac_dir/lib${pam_direct_test_library}.$ac_extension; then
       no_pam= ac_pam_libraries=$ac_dir
       break 2
     fi
   done
 done
])

AC_DEFUN(AC_PATH_PAM,
 [AC_REQUIRE_CPP()dnl

  AC_CHECK_LIB(pam_misc, main, [PAM_MISC_LIB="-lpam_misc"], [], [-lpam -ldl])

 AC_MSG_CHECKING(for PAM)
 AC_ARG_WITH(pam, 
[  --with-pam[=ARG]        enable support for PAM: ARG=[yes|no|service name]],
  [
    if test "x$withval" = "xyes"; then
      no_pam=
      default_pam=yes
    elif test "x$withval" = "xno"; then
      no_pam=yes
    else
      no_pam=
      pam_service="$withval"
        if test -z "$pam_service"; then
        default_pam=yes
        else
        default_pam=
        fi 
      fi
  ], no_pam=yes
 )

 if test ! "$no_pam" = yes; then

 AC_CACHE_VAL(ac_cv_path_pam,
 [
 ac_pam_includes=NONE
 ac_pam_libraries=NONE
 if test -z "$pam_libraries"; then
   pam_libraries=NONE
 fi
 if test -z "$pam_includes"; then
   pam_includes=NONE
 fi

 AC_PATH_PAM_DIRECT
 
 test "x$pam_includes" = xNONE && pam_includes=$ac_pam_includes
 test "x$pam_libraries" = xNONE && pam_libraries=$ac_pam_libraries
 
 if test ! "x$pam_includes" = xNONE && test ! "x$pam_libraries" = xNONE; then
   ac_pam_libs="-lpam $PAM_MISC_LIB -ldl"
   ac_cv_path_pam="no_pam= ac_pam_includes=$ac_pam_includes ac_pam_libraries=$ac_pam_libraries ac_pam_libs=\"$ac_pam_libs\""
 else
   ac_cv_path_pam="no_pam=yes"
 fi
 ])

 eval "$ac_cv_path_pam"
 
 fi

 if test "$no_pam" = yes; then
   AC_MSG_RESULT(no)
 else
   AC_DEFINE(HAVE_PAM)
   PAMLIBS="$ac_pam_libs"
   test "x$pam_includes" = xNONE && pam_includes=$ac_pam_includes
   test "x$pam_libraries" = xNONE && pam_libraries=$ac_pam_libraries
   AC_MSG_RESULT([libraries $pam_libraries, headers $pam_includes])
 if test "$default_pam" = yes; then
   AC_MSG_RESULT(["default pam service name will be used"])
 else
   AC_DEFINE_UNQUOTED(KDE_PAM_SERVICE,"$pam_service")
   AC_MSG_RESULT(["pam service name will be: " $pam_service])
 fi
dnl test whether struct pam_message is const (Linux) or not (Sun)
   pam_appl_h="$ac_pam_includes/security/pam_appl.h"
   AC_MSG_CHECKING(for const pam_message)
   AC_EGREP_HEADER([struct pam_message],
      $pam_appl_h,
      [ AC_EGREP_HEADER([const struct pam_message],
                        $pam_appl_h,
                        [AC_MSG_RESULT(["const: Linux-type PAM"]) ],
                        [AC_MSG_RESULT(["nonconst: Sun-type PAM"])
                        AC_DEFINE(PAM_MESSAGE_NONCONST)] 
                        )],
       [AC_MSG_RESULT(["not found - assume const, Linux-type PAM"])]
       )
 fi
 
 if test "x$pam_libraries" != x && test "x$pam_libraries" != xNONE ; then
     PAMLIBPATHS="-L$pam_libraries"
 fi
 if test "x$pam_includes" != x && test "x$pam_includes" != xNONE ; then
     PAMINC="-I$pam_includes"
 fi
 
 AC_SUBST(PAMINC)
 AC_SUBST(PAMLIBS)
 AC_SUBST(PAMLIBPATHS)

]) 

AC_DEFUN(KDE_CHECK_LIBDL,
[
AC_CHECK_LIB(dl, dlopen, [
LIBDL="-ldl"
ac_cv_have_dlfcn=yes
])

AC_CHECK_LIB(dld, shl_unload, [
LIBDL="-ldld"
ac_cv_have_shload=yes
])

AC_SUBST(LIBDL)
])

AC_DEFUN(KDE_CHECK_DLOPEN,
[
KDE_CHECK_LIBDL
AC_CHECK_HEADERS(dlfcn.h dl.h)
if test "$ac_cv_header_dlfcn_h" = "no"; then
  ac_cv_have_dlfcn=no
fi

if test "$ac_cv_header_dl_h" = "no"; then
  ac_cv_have_shload=no
fi

enable_dlopen=no
AC_ARG_ENABLE(dlopen,
[  --disable-dlopen        link staticly [default=no]] ,
[if test "$enableval" = yes; then
  enable_dlopen=yes
fi],
enable_dlopen=yes)

# override the user's opinion, if we know it better ;)
if test "$ac_cv_have_dlfcn" = "no" && test "$ac_cv_have_shload" = "no"; then
  enable_dlopen=no
fi

if test "$ac_cv_have_dlfcn" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_DLFCN)
fi

if test "$ac_cv_have_shload" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_SHLOAD)
fi

if test "$enable_dlopen" = no ; then
  test -n "$1" && eval $1
else
  test -n "$2" && eval $2
fi

])

AC_DEFUN(KDE_CHECK_DYNAMIC_LOADING,
[
KDE_CHECK_DLOPEN(libtool_enable_shared=no, libtool_enable_static=no)
KDE_PROG_LIBTOOL
AC_MSG_CHECKING([dynamic loading])
eval "`egrep '^build_libtool_libs=' libtool`"
if test "$build_libtool_libs" = "yes" && test "$enable_dlopen" = "yes"; then
  dynamic_loading=yes
  AC_DEFINE_UNQUOTED(HAVE_DYNAMIC_LOADING)
else
  dynamic_loading=no
fi
AC_MSG_RESULT($dynamic_loading)
if test "$dynamic_loading" = "yes"; then
  $1
else
  $2
fi
])

AC_DEFUN(KDE_ADD_INCLUDES,
[
if test -z "$1"; then 
  test_include="Pix.h"
else
  test_include="$1"
fi

AC_MSG_CHECKING([for libg++ ($test_include)])

AC_CACHE_VAL(kde_cv_libgpp_includes,
[
kde_cv_libgpp_includes=no

   for ac_dir in               \
                               \
     /usr/include/g++          \
     /usr/include              \
     /usr/unsupported/include  \
     /opt/include              \
     $extra_include            \
     ; \
   do
     if test -r "$ac_dir/$test_include"; then
       kde_cv_libgpp_includes=$ac_dir
       break
     fi
   done
])

AC_MSG_RESULT($kde_cv_libgpp_includes)
if test "$kde_cv_libgpp_includes" != "no"; then
  all_includes="-I$kde_cv_libgpp_includes $all_includes"
fi
])
])


AC_DEFUN(KDE_CHECK_MICO,
[
AC_REQUIRE([KDE_CHECK_LIBDL])
AC_MSG_CHECKING(for MICO)
AC_ARG_WITH(micodir,
  [  --with-micodir=micodir  where mico is installed ],
  kde_micodir=$withval,
  kde_micodir=/usr/local
)
AC_MSG_RESULT($kde_micodir)
if test ! -r  $kde_micodir/include/CORBA.h; then
  AC_MSG_ERROR([No CORBA.h found, specify another micodir])
fi

MICO_INCLUDES=-I$kde_micodir/include
AC_SUBST(MICO_INCLUDES)
MICO_LDFLAGS=-L$kde_micodir/lib
AC_SUBST(MICO_LDFLAGS)

AC_MSG_CHECKING([for MICO version])
AC_CACHE_VAL(kde_cv_mico_version,
[
AC_LANG_C
cat >conftest.$ac_ext <<EOF
#include <stdio.h>
#include <mico/version.h>
int main() { 
    
   printf("MICO_VERSION=%s\n",MICO_VERSION); 
   return (0); 
}
EOF
ac_compile='${CC-gcc} $CFLAGS $MICO_INCLUDES conftest.$ac_ext -o conftest'
if AC_TRY_EVAL(ac_compile); then
  if eval `./conftest 2>&5`; then
    kde_cv_mico_version=$MICO_VERSION
  else
    AC_MSG_ERROR([your system is not able to execute a small application to
    find MICO version! Check $kde_micodir/include/mico/version.h])
  fi 
else
  AC_MSG_ERROR([your system is not able to compile a small application to
  find MICO version! Check $kde_micodir/include/mico/version.h])
fi
])

dnl installed MICO version
mico_v_maj=`echo $kde_cv_mico_version | sed -e 's/^\(.*\)\..*\..*$/\1/'`
mico_v_mid=`echo $kde_cv_mico_version | sed -e 's/^.*\.\(.*\)\..*$/\1/'`
mico_v_min=`echo $kde_cv_mico_version | sed -e 's/^.*\..*\.\(.*\)$/\1/'`

dnl required MICO version
req_v_maj=`echo $1 | sed -e 's/^\(.*\)\..*\..*$/\1/'`
req_v_mid=`echo $1 | sed -e 's/^.*\.\(.*\)\..*$/\1/'`
req_v_min=`echo $1 | sed -e 's/^.*\..*\.\(.*\)$/\1/'` 

if test "$mico_v_maj" -lt "$req_v_maj" || \
   ( test "$mico_v_maj" -eq "$req_v_maj" && \
        test "$mico_v_mid" -lt "$req_v_mid" ) || \
   ( test "$mico_v_mid" -eq "$req_v_mid" && \
        test "$mico_v_min" -lt "$req_v_min" )

then
  AC_MSG_ERROR([found MICO version $kde_cv_mico_version but version $1 \
at least is required. You should upgrade MICO.])
else
  AC_MSG_RESULT([$kde_cv_mico_version (minimum version $1, ok)])
fi

LIBMICO="-lmico$kde_cv_mico_version $LIBDL"
AC_SUBST(LIBMICO)
IDL=$kde_micodir/bin/idl
AC_SUBST(IDL)
])


AC_DEFUN(KDE_CHECK_MINI_STL,
[
AC_REQUIRE([KDE_CHECK_MICO])

AC_MSG_CHECKING(if we use mico's mini-STL)
AC_CACHE_VAL(kde_cv_have_mini_stl,
[
AC_LANG_CPLUSPLUS
kde_save_cxxflags="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS $MICO_INCLUDES"
AC_TRY_COMPILE(
[
#include <mico/config.h>
],
[
#ifdef HAVE_MINI_STL
#error "nothing"
#endif
],
kde_cv_have_mini_stl=no,
kde_cv_have_mini_stl=yes)
CXXFLAGS="$kde_save_cxxflags"
])


AC_MSG_RESULT($kde_cv_have_mini_stl)
if test "$kde_cv_have_mini_stl" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_MINI_STL)
fi
])

])


AC_DEFUN(KDE_CHECK_LIBPTHREAD,
[
AC_CHECK_LIB(pthread, pthread_create, [LIBPTHREAD="-lpthread"], LIBPTHREAD= )
AC_SUBST(LIBPTHREAD)
])

AC_DEFUN(KDE_TRY_LINK_PYTHON,
[
AC_CACHE_VAL(kde_cv_try_link_python_$1,
[
kde_save_cxxflags="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS $PYTHONINC"
kde_save_libs="$LIBS"
LIBS="$LIBS $LIBPYTHON $2 $LIBDL $LIBSOCKET"
kde_save_ldflags="$LDFLAGS"
LDFLAGS="$LDFLAGS $PYTHONLIB"

AC_TRY_LINK(
[
#include <Python.h>
],[
	PySys_SetArgv(1, 0);
],
	[kde_cv_try_link_python_$1=yes],
	[kde_cv_try_link_python_$1=no]
)
CXXFLAGS="$kde_save_cxxflags"
LIBS="$kde_save_libs"
LDFLAGS="$kde_save_ldflags"
])

if test "$kde_cv_try_link_python_$1" = "yes"; then
  $3
else
  $4
fi

])

AC_DEFUN(KDE_CHECK_PYTHON,
[
AC_REQUIRE([KDE_CHECK_LIBDL])
AC_REQUIRE([KDE_CHECK_LIBPTHREAD])
if test -z "$1"; then 
  version="1.5"
else
  version="$1"
fi

AC_MSG_CHECKING([for Python$version])

AC_ARG_WITH(pythondir, 
[  --with-pythondir=pythondir   use python installed in pythondir ],
[
  ac_python_dir=$withval
], ac_python_dir=/usr/local
)

python_incdirs="$ac_python_dir/include/python$version /usr/include/python$version /usr/local/include/python$version /usr/local/include"
AC_FIND_FILE(Python.h, $python_incdirs, python_incdir)
if test ! -r $python_incdir/Python.h; then
  AC_MSG_ERROR(Python.h not found.)
fi

PYTHONINC=-I$python_incdir

python_libdirs="$ac_python_dir/lib/python$version/config /usr/lib/python$version/config /usr/local/python$version/config"
AC_FIND_FILE(libpython$version.a, $python_libdirs, python_libdir)
if test ! -r $python_libdir/libpython$version.a; then
  AC_MSG_ERROR(libpython$version.a not found.)
fi

PYTHONLIB=-L$python_libdir
LIBPYTHON=-lpython$version

AC_MSG_RESULT(header $python_incdir library $python_libdir)

dnl Note: this test is very weak
AC_MSG_CHECKING(if an Python application links)
KDE_TRY_LINK_PYTHON(normal, "", AC_MSG_RESULT(yes),
 [
    AC_MSG_RESULT(no)
    AC_MSG_CHECKING(if Python depends on -lpthread)
    KDE_TRY_LINK_PYTHON(pthread, "$LIBPTHREAD",
    [  
       AC_MSG_RESULT(yes)
       LIBPYTHON="$LIBPYTHON $LIBPTHREAD $LIBDL"
    ],
    [
       AC_MSG_RESULT(no)
       AC_MSG_CHECKING(if Python depeds on -ltcl)
       KDE_TRY_LINK_PYTHON(tcl, "-ltcl",
       [
	  AC_MSG_RESULT(yes)
	  LIBPYTHON="$LIBPYTHON -ltcl"
       ],
       [
	  AC_MSG_RESULT(no)
	AC_MSG_WARN([it seems, Python depends on another library. 
    Pleae use \"make LIBPTYHON='-lpython$version -lotherlib'\" to fix this
    and contact the authors to let them know about this problem])
	])
    ])
 ]) 

AC_SUBST(PYTHONINC)
AC_SUBST(PYTHONLIB)
AC_SUBST(LIBPYTHON)

])


AC_DEFUN(KDE_CHECK_STL_SGI,
[
    AC_MSG_CHECKING([if STL implementation is SGI like])
    AC_CACHE_VAL(kde_cv_stl_type_sgi,
    [
      AC_TRY_COMPILE([
#include <string>
],[
  string astring="Hallo Welt.";
  astring.erase(0, 6); // now astring is "Welt"
  return 0;
], kde_cv_stl_type_sgi=yes,
   kde_cv_stl_type_sgi=no)
])

   AC_MSG_RESULT($kde_cv_stl_type_sgi)

   if test "$kde_cv_stl_type_sgi" = "yes"; then
	AC_DEFINE_UNQUOTED(HAVE_SGI_STL) 
   fi
])

AC_DEFUN(KDE_CHECK_STL_HP,
[
    AC_MSG_CHECKING([if STL implementation is HP like])
    AC_CACHE_VAL(kde_cv_stl_type_hp,
    [
      AC_TRY_COMPILE([
#include <string>
],[
  string astring="Hello World";
  astring.remove(0, 6); // now astring is "World"
  return 0;
], kde_cv_stl_type_hp=yes,
   kde_cv_stl_type_hp=no)
])
   AC_MSG_RESULT($kde_cv_stl_type_hp)

   if test "$kde_cv_stl_type_hp" = "yes"; then
	AC_DEFINE_UNQUOTED(HAVE_HP_STL) 
   fi
])

AC_DEFUN(KDE_CHECK_STL,
[
    KDE_CHECK_STL_SGI
    
    if test "$kde_cv_stl_type_sgi" = "no"; then
       KDE_CHECK_STL_HP

       if test "$kde_cv_stl_type_hp" = "no"; then
         AC_MSG_ERROR("no known STL type found")
       fi
    fi

])

AC_DEFUN(AC_FIND_QIMGIO,
   [AC_REQUIRE([AC_FIND_JPEG])
AC_MSG_CHECKING([for qimgio])
AC_CACHE_VAL(ac_cv_lib_qimgio,
[ac_save_LIBS="$LIBS"
LIBS="$all_libraries -lqimgio -lpng -lz -lqt $LIBJPEG -lX11 $LIBSOCKET"
AC_TRY_LINK(dnl
[
void qInitImageIO ();
],
            [qInitImageIO();],
            eval "ac_cv_lib_qimgio=yes",
            eval "ac_cv_lib_qimgio=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo $ac_cv_lib_qimgio`\" = yes"; then
  LIBQIMGIO="-lqimgio -lpng -lz $LIBJPEG"
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_QIMGIO)
  AC_SUBST(LIBQIMGIO)
else
  AC_MSG_RESULT(not found)
fi
])

AC_DEFUN(KDE_CHECK_ANSI,
[
AC_MSG_CHECKING([for strdup])

    AC_CACHE_VAL(kde_cv_stl_type_sgi,
    [
AC_LANG_CPLUSPLUS
save_CXXFLAGS="$CXXFLAGS"
if test "$GCC" = "yes"; then
  CXXFLAGS="$CXXFLAGS -pedantic-errors"
fi

AC_TRY_COMPILE([
#include <string.h>
],[
  char buffer[] = "Hallo";
  strdup(buffer)
], kde_cv_has_strdup=yes,
   kde_cv_has_strdup=no)
CXXFLAGS="$save_CXXFLAGS"
])
AC_MSG_RESULT($kde_cv_has_strdup)

if test "$kde_cv_has_strdup" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_STRDUP)
fi

])

AC_DEFUN(KDE_CHECK_INSURE,
[
  AC_ARG_ENABLE(insure, [  --enable-insure             use insure++ for debugging [default=no]],
  [
  if test $enableval = "no"; dnl
	then ac_use_insure="no"
	else ac_use_insure="yes"
   fi
  ], [ac_use_insure="no"])

  AC_MSG_CHECKING(if we will use Insure++ to debug)
  AC_MSG_RESULT($ac_use_insure)
  if test "$ac_use_insure" = "yes"; dnl
       then CC="insure"; CXX="insure"; dnl CFLAGS="$CLAGS -fno-rtti -fno-exceptions "????
   fi
])          

dnl this is for kdm:

AC_DEFUN(AC_CHECK_KDM,
[
AC_CHECK_FUNCS(getsecretkey)
dnl checks for X server

AC_PATH_PROG(X_SERVER, X)
if test ! -z "$X_SERVER"; then
X_SERVER=`echo $X_SERVER | sed -e 's+/X$++'`
AC_DEFINE_UNQUOTED(XBINDIR,$X_SERVER)
XBINDIR=$X_SERVER
AC_SUBST(XBINDIR)
fi

dnl This one tries to find XDMDIR for config files
AC_ARG_WITH(xdmdir,
	[  --with-xdmdir	          If the xdm config dir can't be found automaticly],
	[ ac_xdmdir=$withval],
	[ ac_xdmdir="no"])

AC_MSG_CHECKING([for xdm configuration dir])
if test "$ac_xdmdir" = "no"; then
    rm -fr conftestdir
    if mkdir conftestdir; then
	cd conftestdir
    cat > Imakefile <<'EOF'
acfindxdm:
	@echo 'ac_xdmdir="$(XDMDIR)";'
EOF
	if (xmkmf) > /dev/null 2> /dev/null && test -f Makefile; then
	    eval `${MAKE-make} acfindxdm 2>/dev/null 2>/dev/null | grep -v make`
	fi
	cd ..
	rm -fr conftestdir
	dnl Check if Imake was right
	if test -f $ac_xdmdir/xdm-config; then
	    AC_MSG_RESULT($ac_xdmdir)
	else
	    dnl Here we must do something else
	    dnl Maybe look for xdm-config in standard places, and
	    dnl if that fails use a fresh copy in $KDEDIR/config/kdm/
	    AC_FIND_FILE(xdm-config,/etc/X11/xdm /var/X11/xdm /usr/openwin/xdm /usr/X11R6/lib/X11/xdm,ac_xdmdir)
	    if test -f $ac_xdmdir/xdm-config; then
                AC_MSG_RESULT($ac_xdmdir)
            else                                 
		if test "${prefix}" = NONE; then
			ac_xdmdir=$ac_default_prefix/config/kdm
		else
			ac_xdmdir=$prefix/config/kdm
		fi
		AC_MSG_RESULT([xdm config dir not found, installing defaults in $ac_xdmdir])
		xdmconfigsubdir=xdmconfig
		AC_SUBST(xdmconfigsubdir)
	    fi
	fi
    fi
else
    if test -f $ac_xdmdir/xdm-config; then
	AC_MSG_RESULT($ac_xdmdir)
    else

	AC_MSG_RESULT([xdm config dir not found, installing defaults in $ac_xdmdir])
	xdmconfigsubdir=xdmconfig
	AC_SUBST(xdmconfigsubdir)
    fi
fi
AC_DEFINE_UNQUOTED(XDMDIR,"$ac_xdmdir")
AC_SUBST(ac_xdmdir)

AC_PATH_PAM
if test "x$no_pam" = "xyes"; then 
	pam_support="no"
else
	pam_support="yes"
        shadow_support="no" # if pam is installed, use it. We can't savely 
	                    # test, if it works *sigh*
fi

AC_ARG_WITH(shadow,
	[  --with-shadow		  If you want shadow password support ],
	[ if test "$withval" = "yes"; then
             shadow_support="yes"
          else
             shadow_support="no"
          fi
	  if test "$pam_support" = "yes" && test "$shadow_support=yes"; then
		AC_MSG_WARN("You can not define both pam AND shadow")
	  fi
	],
	[ if test -z "$shadow_support"; then shadow_support="no"; fi ] )

if test "$pam_support" = "yes"; then
  AC_CHECK_LIB(pam, main, [PASSWDLIB="-lpam -ldl"
  AC_DEFINE_UNQUOTED(HAVE_PAM_LIB)],
  [],-ldl)
fi

if test -z "$PASSWDLIB" && test "$shadow_support" = "yes"; then
  AC_CHECK_LIB(shadow, main,
    [ PASSWDLIB="-lshadow"
      AC_DEFINE_UNQUOTED(HAVE_SHADOW_LIB)
    ])
fi
AC_SUBST(PASSWDLIB)
AC_CHECK_LIB(util, main, [LIBUTIL="-lutil"]) dnl for FreeBSD
AC_SUBST(LIBUTIL)
AC_CHECK_LIB(s, main, [LIB_LIBS="-ls"]) dnl for AIX
AC_SUBST(LIB_LIBS)
AC_CHECK_LIB(Xdmcp, main, [LIBXDMCP="-lXdmcp"], , $X_LDFLAGS -lX11) dnl for Unixware
AC_SUBST(LIBXDMCP)

if test -n "$LIBXDMCP"; then
  ac_cpp_safe=$ac_cpp
  ac_cpp='$CXXCPP $CPPFLAGS $X_INCLUDES'
  AC_CHECK_HEADERS(X11/Xdmcp.h)
  ac_cpp=$ac_cpp_safe
fi

])
# Configure paths for GTK+
# Owen Taylor     97-11-3

dnl AM_PATH_GTK([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GTK, and define GTK_CFLAGS and GTK_LIBS
dnl
AC_DEFUN(AM_PATH_GTK,
[dnl 
dnl Get the cflags and libraries from the gtk-config script
dnl
AC_ARG_WITH(gtk-prefix,[  --with-gtk-prefix=PFX   Prefix where GTK is installed (optional)],
            gtk_config_prefix="$withval", gtk_config_prefix="")
AC_ARG_WITH(gtk-exec-prefix,[  --with-gtk-exec-prefix=PFX Exec prefix where GTK is installed (optional)],
            gtk_config_exec_prefix="$withval", gtk_config_exec_prefix="")
AC_ARG_ENABLE(gtktest, [  --disable-gtktest       Do not try to compile and run a test GTK program],
		    , enable_gtktest=yes)

  for module in . $4
  do
      case "$module" in
         gthread) 
             gtk_config_args="$gtk_config_args gthread"
         ;;
      esac
  done

  if test x$gtk_config_exec_prefix != x ; then
     gtk_config_args="$gtk_config_args --exec-prefix=$gtk_config_exec_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_exec_prefix/bin/gtk-config
     fi
  fi
  if test x$gtk_config_prefix != x ; then
     gtk_config_args="$gtk_config_args --prefix=$gtk_config_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_prefix/bin/gtk-config
     fi
  fi

  AC_PATH_PROG(GTK_CONFIG, gtk-config, no)
  min_gtk_version=ifelse([$1], ,0.99.7,$1)
  AC_MSG_CHECKING(for GTK - version >= $min_gtk_version)
  no_gtk=""
  if test "$GTK_CONFIG" = "no" ; then
    no_gtk=yes
  else
    GTK_CFLAGS=`$GTK_CONFIG $gtk_config_args --cflags`
    GTK_LIBS=`$GTK_CONFIG $gtk_config_args --libs`
    gtk_config_major_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtk_config_minor_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtk_config_micro_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtktest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GTK_CFLAGS"
      LIBS="$GTK_LIBS $LIBS"
dnl
dnl Now check if the installed GTK is sufficiently new. (Also sanity
dnl checks the results of gtk-config to some extent
dnl
      rm -f conf.gtktest
      AC_TRY_RUN([
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtktest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtk_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtk_version");
     exit(1);
   }

  if ((gtk_major_version != $gtk_config_major_version) ||
      (gtk_minor_version != $gtk_config_minor_version) ||
      (gtk_micro_version != $gtk_config_micro_version))
    {
      printf("\n*** 'gtk-config --version' returned %d.%d.%d, but GTK+ (%d.%d.%d)\n", 
             $gtk_config_major_version, $gtk_config_minor_version, $gtk_config_micro_version,
             gtk_major_version, gtk_minor_version, gtk_micro_version);
      printf ("*** was found! If gtk-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK+. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gtk-config was wrong, set the environment variable GTK_CONFIG\n");
      printf("*** to point to the correct copy of gtk-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
#if defined (GTK_MAJOR_VERSION) && defined (GTK_MINOR_VERSION) && defined (GTK_MICRO_VERSION)
  else if ((gtk_major_version != GTK_MAJOR_VERSION) ||
	   (gtk_minor_version != GTK_MINOR_VERSION) ||
           (gtk_micro_version != GTK_MICRO_VERSION))
    {
      printf("*** GTK+ header files (version %d.%d.%d) do not match\n",
	     GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     gtk_major_version, gtk_minor_version, gtk_micro_version);
    }
#endif /* defined (GTK_MAJOR_VERSION) ... */
  else
    {
      if ((gtk_major_version > major) ||
        ((gtk_major_version == major) && (gtk_minor_version > minor)) ||
        ((gtk_major_version == major) && (gtk_minor_version == minor) && (gtk_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK+ (%d.%d.%d) was found.\n",
               gtk_major_version, gtk_minor_version, gtk_micro_version);
        printf("*** You need a version of GTK+ newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK+ is always available from ftp://ftp.gtk.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gtk-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK+, but you can also set the GTK_CONFIG environment to point to the\n");
        printf("*** correct copy of gtk-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtk=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtk" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GTK_CONFIG" = "no" ; then
       echo "*** The gtk-config script installed by GTK could not be found"
       echo "*** If GTK was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GTK_CONFIG environment variable to the"
       echo "*** full path to gtk-config."
     else
       if test -f conf.gtktest ; then
        :
       else
          echo "*** Could not run GTK test program, checking why..."
          CFLAGS="$CFLAGS $GTK_CFLAGS"
          LIBS="$LIBS $GTK_LIBS"
          AC_TRY_LINK([
#include <gtk/gtk.h>
#include <stdio.h>
],      [ return ((gtk_major_version) || (gtk_minor_version) || (gtk_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK or finding the wrong"
          echo "*** version of GTK. If it is not finding GTK, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the GTK package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps gtk gtk-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GTK was incorrectly installed"
          echo "*** or that you have moved GTK since it was installed. In the latter case, you"
          echo "*** may want to edit the gtk-config script: $GTK_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTK_CFLAGS=""
     GTK_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
  rm -f conf.gtktest
])
# aclocal-include.m4
# 
# This macro adds the name macrodir to the set of directories
# that `aclocal' searches for macros.  

# serial 1

dnl AM_ACLOCAL_INCLUDE(macrodir)
AC_DEFUN([AM_ACLOCAL_INCLUDE],
[
	AM_CONDITIONAL(INSIDE_GNOME_COMMON, test x = y)

	test -n "$ACLOCAL_FLAGS" && ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

	for k in $1 ; do ACLOCAL="$ACLOCAL -I $k" ; done
])
# Configure paths for GNOME-PRINT
# Chris Lahey	99-2-5
# stolen from Manish Singh again
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_GNOME_PRINT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for GNOME-PRINT, and define GNOME_PRINT_CFLAGS and GNOME_PRINT_LIBS
dnl
AC_DEFUN(AM_PATH_GNOME_PRINT,
[dnl 
dnl Get the cflags and libraries from the gnome-config script
dnl
AC_ARG_WITH(gnome-print-prefix,[  --with-gnome-print-prefix=PFX   Prefix where GNOME-PRINT is installed (optional)],
            gnome_print_prefix="$withval", gnome_print_prefix="")
AC_ARG_WITH(gnome-print-exec-prefix,[  --with-gnome-print-exec-prefix=PFX Exec prefix where GNOME-PRINT is installed (optional)],
            gnome_print_exec_prefix="$withval", gnome_print_exec_prefix="")
AC_ARG_ENABLE(gnome-printtest, [  --disable-gnome-printtest       Do not try to compile and run a test GNOME-PRINT program],
		    , enable_gnome_printtest=yes)

  if test x$gnome_print_exec_prefix != x ; then
     gnome_print_args="$gnome_print_args --exec-prefix=$gnome_print_exec_prefix"
     if test x${GNOME_CONFIG+set} != xset ; then
        GNOME_CONFIG=$gnome_print_exec_prefix/bin/gnome-config
     fi
  fi
  if test x$gnome_print_prefix != x ; then
     gnome_print_args="$gnome_print_args --prefix=$gnome_print_prefix"
     if test x${GNOME_CONFIG+set} != xset ; then
        GNOME_CONFIG=$gnome_print_prefix/bin/gnome-config
     fi
  fi

  AC_PATH_PROG(GNOME_CONFIG, gnome-config, no)
  min_gnome_print_version=ifelse([$1], ,0.1.0,$1)
  AC_MSG_CHECKING(for GNOME-PRINT - version >= $min_gnome_print_version)
  no_gnome_print=""
  if test "$GNOME_CONFIG" = "no" ; then
    no_gnome_print=yes
  else
    GNOME_PRINT_CFLAGS=`$GNOME_CONFIG $gnome_printconf_args --cflags print`
    GNOME_PRINT_LIBS=`$GNOME_CONFIG $gnome_printconf_args --libs print`

    gnome_print_major_version=`$GNOME_CONFIG $gnome_print_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gnome_print_minor_version=`$GNOME_CONFIG $gnome_print_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gnome_print_micro_version=`$GNOME_CONFIG $gnome_print_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gnome_printtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GNOME_PRINT_CFLAGS"
      LIBS="$LIBS $GNOME_PRINT_LIBS"
dnl
dnl Now check if the installed GNOME-PRINT is sufficiently new. (Also sanity
dnl checks the results of gnome-config to some extent
dnl
      rm -f conf.gnome_printtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgnomeprint/gnome-print.h>

static char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gnome_printtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_gnome_print_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gnome_print_version");
     exit(1);
   }
  return 0;
#if 0
   if (($gnome_print_major_version > major) ||
      (($gnome_print_major_version == major) && ($gnome_print_minor_version > minor)) ||
      (($gnome_print_major_version == major) && ($gnome_print_minor_version == minor) && ($gnome_print_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'gnome-config print --version' returned %d.%d.%d, but the minimum version\n", $gnome_print_major_version, $gnome_print_minor_version, $gnome_print_micro_version);
      printf("*** of GNOME-PRINT required is %d.%d.%d. If gnome-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If gnome-config was wrong, set the environment variable GNOME_CONFIG\n");
      printf("*** to point to the correct copy of gnome-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
#endif
}

],, no_gnome_print=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gnome_print" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GNOME_CONFIG" = "no" ; then
       echo "*** The gnome-config script installed by GNOME-LIBS could not be found"
       echo "*** If GNOME-PRINT was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GNOME_CONFIG environment variable to the"
       echo "*** full path to gnome-config."
     else
       if test -f conf.gnome_printtest ; then
        :
       else
          echo "*** Could not run GNOME-PRINT test program, checking why..."
          CFLAGS="$CFLAGS $GNOME_PRINT_CFLAGS"
          LIBS="$LIBS $GNOME_PRINT_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <libgnomeprint/gnome-print.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GNOME-PRINT or finding the wrong"
          echo "*** version of GNOME-PRINT. If it is not finding GNOME-PRINT, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GNOME-PRINT was incorrectly installed"
          echo "*** or that you have moved GNOME-PRINT since it was installed. In the latter case, you"
          echo "*** may want to edit the gnome-config script: $GNOME_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GNOME_PRINT_CFLAGS=""
     GNOME_PRINT_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GNOME_PRINT_CFLAGS)
  AC_SUBST(GNOME_PRINT_LIBS)
  rm -f conf.gnome_printtest
])

AC_DEFUN([GNOME_PRINT_CHECK], [
	AM_PATH_GNOME_PRINT(0.1.0,,[AC_MSG_ERROR(GNOME-PRINT not found)])
])
dnl GNOME_COMPILE_WARNINGS
dnl Turn on many useful compiler warnings
dnl For now, only works on GCC
AC_DEFUN([GNOME_COMPILE_WARNINGS],[
  AC_ARG_ENABLE(compile-warnings, 
    [  --enable-compile-warnings=[no/minimum/yes]	Turn on compiler warnings.],,enable_compile_warnings=minimum)

  AC_MSG_CHECKING(what warning flags to pass to the C compiler)
  warnCFLAGS=
  if test "x$GCC" != xyes; then
    enable_compile_warnings=no
  fi

  if test "x$enable_compile_warnings" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CFLAGS " in
      *[\ \	]-Wall[\ \	]*) ;;
      *) warnCFLAGS="-Wall -Wunused" ;;
      esac

      ## -W is not all that useful.  And it cannot be controlled
      ## with individual -Wno-xxx flags, unlike -Wall
      if test "x$enable_compile_warnings" = "xyes"; then
	warnCFLAGS="$warnCFLAGS -Wmissing-prototypes -Wmissing-declarations"
      fi
    fi
  fi
  AC_MSG_RESULT($warnCFLAGS)

  AC_ARG_ENABLE(iso-c,
    [  --enable-iso-c          Try to warn if code is not ISO C ],,
    enable_iso_c=no)

  AC_MSG_CHECKING(what language compliance flags to pass to the C compiler)
  complCFLAGS=
  if test "x$enable_iso_c" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CFLAGS " in
      *[\ \	]-ansi[\ \	]*) ;;
      *) complCFLAGS="$complCFLAGS -ansi" ;;
      esac

      case " $CFLAGS " in
      *[\ \	]-pedantic[\ \	]*) ;;
      *) complCFLAGS="$complCFLAGS -pedantic" ;;
      esac
    fi
  fi
  AC_MSG_RESULT($complCFLAGS)
  if test "x$cflags_set" != "xyes"; then
    CFLAGS="$CFLAGS $warnCFLAGS $complCFLAGS"
    cflags_set=yes
    AC_SUBST(cflags_set)
  fi
])

dnl For C++, do basically the same thing.

AC_DEFUN([GNOME_CXX_WARNINGS],[
  AC_ARG_ENABLE(cxx-warnings, 
    [  --enable-cxx-warnings=[no/minimum/yes]	Turn on compiler warnings.],,enable_cxx_warnings=minimum)

  AC_MSG_CHECKING(what warning flags to pass to the C++ compiler)
  warnCXXFLAGS=
  if test "x$GCC" != xyes; then
    enable_compile_warnings=no
  fi
  if test "x$enable_cxx_warnings" != "xno"; then
    if test "x$GCC" = "xyes"; then
      case " $CXXFLAGS " in
      *[\ \	]-Wall[\ \	]*) ;;
      *) warnCXXFLAGS="-Wall -Wno-unused" ;;
      esac

      ## -W is not all that useful.  And it cannot be controlled
      ## with individual -Wno-xxx flags, unlike -Wall
      if test "x$enable_cxx_warnings" = "xyes"; then
	warnCXXFLAGS="$warnCXXFLAGS -Wmissing-prototypes -Wmissing-declarations -Wshadow -Woverloaded-virtual"
      fi
    fi
  fi
  AC_MSG_RESULT($warnCXXFLAGS)

   AC_ARG_ENABLE(iso-cxx,
     [  --enable-iso-cxx          Try to warn if code is not ISO C++ ],,
     enable_iso_cxx=no)

   AC_MSG_CHECKING(what language compliance flags to pass to the C++ compiler)
   complCXXFLAGS=
   if test "x$enable_iso_cxx" != "xno"; then
     if test "x$GCC" = "xyes"; then
      case " $CXXFLAGS " in
      *[\ \	]-ansi[\ \	]*) ;;
      *) complCXXFLAGS="$complCXXFLAGS -ansi" ;;
      esac

      case " $CXXFLAGS " in
      *[\ \	]-pedantic[\ \	]*) ;;
      *) complCXXFLAGS="$complCXXFLAGS -pedantic" ;;
      esac
     fi
   fi
  AC_MSG_RESULT($complCXXFLAGS)
  if test "x$cxxflags_set" != "xyes"; then
    CXXFLAGS="$CXXFLAGS $warnCXXFLAGS $complCXXFLAGS"
    cxxflags_set=yes
    AC_SUBST(cxxflags_set)
  fi
])
dnl
dnl And better, use gthreads instead...
dnl

AC_DEFUN([GNOME_PTHREAD_CHECK],[
	PTHREAD_LIB=""
	AC_CHECK_LIB(pthread, pthread_create, PTHREAD_LIB="-lpthread",
		[AC_CHECK_LIB(pthreads, pthread_create, PTHREAD_LIB="-lpthreads",
		    [AC_CHECK_LIB(c_r, pthread_create, PTHREAD_LIB="-lc_r",
			[AC_CHECK_FUNC(pthread_create)]
		    )]
		)]
	)
	AC_SUBST(PTHREAD_LIB)
	AC_PROVIDE([GNOME_PTHREAD_CHECK])
])
dnl Curses detection: Munged from Midnight Commander's configure.in
dnl
dnl What it does:
dnl =============
dnl
dnl - Determine which version of curses is installed on your system
dnl   and set the -I/-L/-l compiler entries and add a few preprocessor
dnl   symbols 
dnl - Do an AC_SUBST on the CURSES_INCLUDEDIR and CURSES_LIBS so that
dnl   @CURSES_INCLUDEDIR@ and @CURSES_LIBS@ will be available in
dnl   Makefile.in's
dnl - Modify the following configure variables (these are the only
dnl   curses.m4 variables you can access from within configure.in)
dnl   CURSES_INCLUDEDIR - contains -I's and possibly -DRENAMED_CURSES if
dnl                       an ncurses.h that's been renamed to curses.h
dnl                       is found.
dnl   CURSES_LIBS       - sets -L and -l's appropriately
dnl   CFLAGS            - if --with-sco, add -D_SVID3 
dnl   has_curses        - exports result of tests to rest of configure
dnl
dnl Usage:
dnl ======
dnl 1) Add lines indicated below to acconfig.h
dnl 2) call AC_CHECK_CURSES after AC_PROG_CC in your configure.in
dnl 3) Instead of #include <curses.h> you should use the following to
dnl    properly locate ncurses or curses header file
dnl
dnl    #if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
dnl    #include <ncurses.h>
dnl    #else
dnl    #include <curses.h>
dnl    #endif
dnl
dnl 4) Make sure to add @CURSES_INCLUDEDIR@ to your preprocessor flags
dnl 5) Make sure to add @CURSES_LIBS@ to your linker flags or LIBS
dnl
dnl Notes with automake:
dnl - call AM_CONDITIONAL(HAS_CURSES, test "$has_curses" = true) from
dnl   configure.in
dnl - your Makefile.am can look something like this
dnl   -----------------------------------------------
dnl   INCLUDES= blah blah blah $(CURSES_INCLUDEDIR) 
dnl   if HAS_CURSES
dnl   CURSES_TARGETS=name_of_curses_prog
dnl   endif
dnl   bin_PROGRAMS = other_programs $(CURSES_TARGETS)
dnl   other_programs_SOURCES = blah blah blah
dnl   name_of_curses_prog_SOURCES = blah blah blah
dnl   other_programs_LDADD = blah
dnl   name_of_curses_prog_LDADD = blah $(CURSES_LIBS)
dnl   -----------------------------------------------
dnl
dnl
dnl The following lines should be added to acconfig.h:
dnl ==================================================
dnl
dnl /*=== Curses version detection defines ===*/
dnl /* Found some version of curses that we're going to use */
dnl #undef HAS_CURSES
dnl    
dnl /* Use SunOS SysV curses? */
dnl #undef USE_SUNOS_CURSES
dnl 
dnl /* Use old BSD curses - not used right now */
dnl #undef USE_BSD_CURSES
dnl 
dnl /* Use SystemV curses? */
dnl #undef USE_SYSV_CURSES
dnl 
dnl /* Use Ncurses? */
dnl #undef USE_NCURSES
dnl 
dnl /* If you Curses does not have color define this one */
dnl #undef NO_COLOR_CURSES
dnl 
dnl /* Define if you want to turn on SCO-specific code */
dnl #undef SCO_FLAVOR
dnl 
dnl /* Set to reflect version of ncurses *
dnl  *   0 = version 1.*
dnl  *   1 = version 1.9.9g
dnl  *   2 = version 4.0/4.1 */
dnl #undef NCURSES_970530
dnl
dnl /*=== End new stuff for acconfig.h ===*/
dnl 


AC_DEFUN(AC_CHECK_CURSES,[
	search_ncurses=true
	screen_manager=""
	has_curses=false

	CFLAGS=${CFLAGS--O}

	AC_SUBST(CURSES_LIBS)
	AC_SUBST(CURSES_INCLUDEDIR)

	AC_ARG_WITH(sco,
	  [  --with-sco              Use this to turn on SCO-specific code],[
	  if test x$withval = xyes; then
		AC_DEFINE(SCO_FLAVOR)
		CFLAGS="$CFLAGS -D_SVID3"
	  fi
	])

	AC_ARG_WITH(sunos-curses,
	  [  --with-sunos-curses     Used to force SunOS 4.x curses],[
	  if test x$withval = xyes; then
		AC_USE_SUNOS_CURSES
	  fi
	])

	AC_ARG_WITH(osf1-curses,
	  [  --with-osf1-curses      Used to force OSF/1 curses],[
	  if test x$withval = xyes; then
		AC_USE_OSF1_CURSES
	  fi
	])

	AC_ARG_WITH(vcurses,
	  [  --with-vcurses[=incdir] Used to force SysV curses],
	  if test x$withval != xyes; then
		CURSES_INCLUDEDIR="-I$withval"
	  fi
	  AC_USE_SYSV_CURSES
	)

	AC_ARG_WITH(ncurses,
	  [  --with-ncurses[=dir]  Compile with ncurses/locate base dir],
	  if test x$withval = xno ; then
		search_ncurses=false
	  elif test x$withval != xyes ; then
		CURSES_LIBS="$LIBS -L$withval/lib -lncurses"
		CURSES_INCLUDEDIR="-I$withval/include"
		search_ncurses=false
		screen_manager="ncurses"
		AC_DEFINE(USE_NCURSES)
		AC_DEFINE(HAS_CURSES)
		has_curses=true
	  fi
	)

	if $search_ncurses
	then
		AC_SEARCH_NCURSES()
	fi


])


AC_DEFUN(AC_USE_SUNOS_CURSES, [
	search_ncurses=false
	screen_manager="SunOS 4.x /usr/5include curses"
	AC_MSG_RESULT(Using SunOS 4.x /usr/5include curses)
	AC_DEFINE(USE_SUNOS_CURSES)
	AC_DEFINE(HAS_CURSES)
	has_curses=true
	AC_DEFINE(NO_COLOR_CURSES)
	AC_DEFINE(USE_SYSV_CURSES)
	CURSES_INCLUDEDIR="-I/usr/5include"
	CURSES_LIBS="/usr/5lib/libcurses.a /usr/5lib/libtermcap.a"
	AC_MSG_RESULT(Please note that some screen refreshs may fail)
])

AC_DEFUN(AC_USE_OSF1_CURSES, [
       AC_MSG_RESULT(Using OSF1 curses)
       search_ncurses=false
       screen_manager="OSF1 curses"
       AC_DEFINE(HAS_CURSES)
       has_curses=true
       AC_DEFINE(NO_COLOR_CURSES)
       AC_DEFINE(USE_SYSV_CURSES)
       CURSES_LIBS="-lcurses"
])

AC_DEFUN(AC_USE_SYSV_CURSES, [
	AC_MSG_RESULT(Using SysV curses)
	AC_DEFINE(HAS_CURSES)
	has_curses=true
	AC_DEFINE(USE_SYSV_CURSES)
	search_ncurses=false
	screen_manager="SysV/curses"
	CURSES_LIBS="-lcurses"
])

dnl AC_ARG_WITH(bsd-curses,
dnl [--with-bsd-curses         Used to compile with bsd curses, not very fancy],
dnl 	search_ncurses=false
dnl	screen_manager="Ultrix/cursesX"
dnl	if test $system = ULTRIX
dnl	then
dnl	    THIS_CURSES=cursesX
dnl        else
dnl	    THIS_CURSES=curses
dnl	fi
dnl
dnl	CURSES_LIBS="-l$THIS_CURSES -ltermcap"
dnl	AC_DEFINE(HAS_CURSES)
dnl	has_curses=true
dnl	AC_DEFINE(USE_BSD_CURSES)
dnl	AC_MSG_RESULT(Please note that some screen refreshs may fail)
dnl	AC_WARN(Use of the bsdcurses extension has some)
dnl	AC_WARN(display/input problems.)
dnl	AC_WARN(Reconsider using xcurses)
dnl)

	
dnl
dnl Parameters: directory filename cureses_LIBS curses_INCLUDEDIR nicename
dnl
AC_DEFUN(AC_NCURSES, [
    if $search_ncurses
    then
        if test -f $1/$2
	then
	    AC_MSG_RESULT(Found ncurses on $1/$2)
 	    CURSES_LIBS="$3"
	    CURSES_INCLUDEDIR="$4"
	    search_ncurses=false
	    screen_manager=$5
            AC_DEFINE(HAS_CURSES)
            has_curses=true
	    AC_DEFINE(USE_NCURSES)
	fi
    fi
])

AC_DEFUN(AC_SEARCH_NCURSES, [
    AC_CHECKING("location of ncurses.h file")

    AC_NCURSES(/usr/include, ncurses.h, -lncurses,, "ncurses on /usr/include")
    AC_NCURSES(/usr/include/ncurses, ncurses.h, -lncurses, -I/usr/include/ncurses, "ncurses on /usr/include/ncurses")
    AC_NCURSES(/usr/local/include, ncurses.h, -L/usr/local/lib -lncurses, -I/usr/local/include, "ncurses on /usr/local")
    AC_NCURSES(/usr/local/include/ncurses, ncurses.h, -L/usr/local/lib -L/usr/local/lib/ncurses -lncurses, -I/usr/local/include/ncurses, "ncurses on /usr/local/include/ncurses")

    AC_NCURSES(/usr/local/include/ncurses, curses.h, -L/usr/local/lib -lncurses, -I/usr/local/include/ncurses -DRENAMED_NCURSES, "renamed ncurses on /usr/local/.../ncurses")

    AC_NCURSES(/usr/include/ncurses, curses.h, -lncurses, -I/usr/include/ncurses -DRENAMED_NCURSES, "renamed ncurses on /usr/include/ncurses")

    dnl
    dnl We couldn't find ncurses, try SysV curses
    dnl
    if $search_ncurses 
    then
        AC_EGREP_HEADER(init_color, /usr/include/curses.h,
	    AC_USE_SYSV_CURSES)
	AC_EGREP_CPP(USE_NCURSES,[
#include <curses.h>
#ifdef __NCURSES_H
#undef USE_NCURSES
USE_NCURSES
#endif
],[
	CURSES_INCLUDEDIR="$CURSES_INCLUDEDIR -DRENAMED_NCURSES"
        AC_DEFINE(HAS_CURSES)
	has_curses=true
        AC_DEFINE(USE_NCURSES)
        search_ncurses=false
        screen_manager="ncurses installed as curses"
])
    fi

    dnl
    dnl Try SunOS 4.x /usr/5{lib,include} ncurses
    dnl The flags USE_SUNOS_CURSES, USE_BSD_CURSES and BUGGY_CURSES
    dnl should be replaced by a more fine grained selection routine
    dnl
    if $search_ncurses
    then
	if test -f /usr/5include/curses.h
	then
	    AC_USE_SUNOS_CURSES
        fi
    else
        # check for ncurses version, to properly ifdef mouse-fix
	AC_MSG_CHECKING(for ncurses version)
	ncurses_version=unknown
cat > conftest.$ac_ext <<EOF
[#]line __oline__ "configure"
#include "confdefs.h"
#ifdef RENAMED_NCURSES
#include <curses.h>
#else
#include <ncurses.h>
#endif
#undef VERSION
VERSION:NCURSES_VERSION
EOF
        if (eval "$ac_cpp conftest.$ac_ext") 2>&AC_FD_CC |
  egrep "VERSION:" >conftest.out 2>&1; then
changequote(,)dnl
            ncurses_version=`cat conftest.out|sed -e 's/^[^"]*"//' -e 's/".*//'`
changequote([,])dnl
	fi
	rm -rf conftest*
        AC_MSG_RESULT($ncurses_version)
	case "$ncurses_version" in
changequote(,)dnl
	4.[01])
changequote([,])dnl
            AC_DEFINE(NCURSES_970530,2)
            ;;
	1.9.9g)
            AC_DEFINE(NCURSES_970530,1)
            ;;
	1*)
            AC_DEFINE(NCURSES_970530,0)
            ;;
	esac
    fi
])





dnl GNOME_SUPPORT_CHECKS
dnl    Check for various support functions needed by the standard
dnl    Gnome libraries.  Sets LIBOBJS, might define some macros.
dnl    This should only be used when building the Gnome libs; 
dnl    Gnome clients should not need this macro.
AC_DEFUN([GNOME_SUPPORT_CHECKS],[
  # we need an `awk' to build `gnomesupport.h'
  AC_REQUIRE([AC_PROG_AWK])

  # this should go away soon
  need_gnome_support=yes

  save_LIBOBJS="$LIBOBJS"
  LIBOBJS=

  AC_CHECK_FUNCS(getopt_long,,LIBOBJS="$LIBOBJS getopt.o getopt1.o")

  # for `scandir'
  AC_HEADER_DIRENT

  # copied from `configure.in' of `libiberty'
  vars="program_invocation_short_name program_invocation_name sys_errlist"
  for v in $vars; do
    AC_MSG_CHECKING([for $v])
    AC_CACHE_VAL(gnome_cv_var_$v,
      [AC_TRY_LINK([int *p;], [extern int $v; p = &$v;],
		   [eval "gnome_cv_var_$v=yes"],
		   [eval "gnome_cv_var_$v=no"])])
    if eval "test \"`echo '$gnome_cv_var_'$v`\" = yes"; then
      AC_MSG_RESULT(yes)
      n=HAVE_`echo $v | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
      AC_DEFINE_UNQUOTED($n)
    else
      AC_MSG_RESULT(no)
    fi
  done

  AC_REPLACE_FUNCS(memmove mkstemp scandir strcasecmp strerror strndup strnlen)
  AC_REPLACE_FUNCS(strtok_r strtod strtol strtoul vasprintf vsnprintf)

  AC_CHECK_FUNCS(realpath,,LIBOBJS="$LIBOBJS canonicalize.o")

  # to include `error.c' error.c has some HAVE_* checks
  AC_CHECK_FUNCS(vprintf doprnt strerror_r)
  AM_FUNC_ERROR_AT_LINE

  # This is required if we declare setreuid () and setregid ().
  AC_TYPE_UID_T

  # see if we need to declare some functions.  Solaris is notorious for
  # putting functions into the `libc' but not listing them in the headers
  AC_CHECK_HEADERS(string.h strings.h stdlib.h unistd.h dirent.h)
  GCC_NEED_DECLARATIONS(gethostname setreuid setregid getpagesize)
  GCC_NEED_DECLARATION(scandir,[
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
])

  # Turn our LIBOBJS into libtool objects.  This is gross, but it
  # requires changes to autoconf before it goes away.
  LTLIBOBJS=`echo "$LIBOBJS" | sed 's/\.o/.lo/g'`
  AC_SUBST(need_gnome_support)
  AC_SUBST(LTLIBOBJS)

  LIBOBJS="$save_LIBOBJS"
  AM_CONDITIONAL(BUILD_GNOME_SUPPORT, test "$need_gnome_support" = yes)
])
# Configure paths for Bonobo
# Miguel de Icaza, 99-04-12
# Stolen from Chris Lahey	99-2-5
# stolen from Manish Singh again
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_BONOBO ([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for Bonobo, and define BONOBO_CFLAGS and BONOBO_LIBS
dnl
AC_DEFUN([AM_PATH_BONOBO],
[
dnl 
dnl Get the cflags and libraries from the gnome-config script
dnl
AC_ARG_WITH(bonobo-prefix,[  --with-bonobo-prefix=PFX   Prefix where Bonobo is installed (optional)],
            bonobo_prefix="$withval", bonobo_prefix="")
AC_ARG_WITH(bonobo-exec-prefix,[  --with-bonobo-exec-prefix=PFX Exec prefix where Bonobo is installed (optional)],
            bonobo_exec_prefix="$withval", bonobo_exec_prefix="")
AC_ARG_ENABLE(bonobotest, [  --disable-bonobotest       Do not try to compile and run a test Bonobo program],
		    , enable_bonobotest=yes)

  if test x$bonobo_exec_prefix != x ; then
     bonobo_args="$bonobo_args --exec-prefix=$bonobo_exec_prefix"
     if test x${GNOME_CONFIG+set} != xset ; then
        GNOME_CONFIG=$bonobo_exec_prefix/bin/gnome-config
     fi
  fi
  if test x$bonobo_prefix != x ; then
     bonobo_args="$bonobo_args --prefix=$bonobo_prefix"
     if test x${GNOME_CONFIG+set} != xset ; then
        GNOME_CONFIG=$bonobo_prefix/bin/gnome-config
     fi
  fi

  AC_PATH_PROG(GNOME_CONFIG, gnome-config, no)
  min_bonobo_version=ifelse([$1], ,0.1.0,$1)
  AC_MSG_CHECKING(for BONOBO - version >= $min_bonobo_version)
  no_bonobo=""
  if test "$GNOME_CONFIG" = "no" ; then
    no_bonobo=yes
  else
    BONOBO_CFLAGS=`$GNOME_CONFIG $bonoboconf_args --cflags bonobo`
    BONOBO_LIBS=`$GNOME_CONFIG $bonoboconf_args --libs bonobo`

    bonobo_major_version=`$GNOME_CONFIG $bonobo_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    bonobo_minor_version=`$GNOME_CONFIG $bonobo_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    bonobo_micro_version=`$GNOME_CONFIG $bonobo_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_bonobotest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $BONOBO_CFLAGS"
      LIBS="$LIBS $BONOBO_LIBS"
dnl
dnl Now check if the installed BONOBO is sufficiently new. (Also sanity
dnl checks the results of gnome-config to some extent
dnl
      rm -f conf.bonobotest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bonobo.h>

static char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.bonobotest");
  bonobo_object_get_type ();
  return 0;
}

],, no_bonobo=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_bonobo" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GNOME_CONFIG" = "no" ; then
       echo "*** The gnome-config script installed by GNOME-LIBS could not be found"
       echo "*** If BONOBO was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GNOME_CONFIG environment variable to the"
       echo "*** full path to gnome-config."
     else
       if test -f conf.bonobotest ; then
        :
       else
          echo "*** Could not run BONOBO test program, checking why..."
          CFLAGS="$CFLAGS $BONOBO_CFLAGS"
          LIBS="$LIBS $BONOBO_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <bonobo/gnome-object.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding BONOBO or finding the wrong"
          echo "*** version of BONOBO. If it is not finding BONOBO, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means BONOBO was incorrectly installed"
          echo "*** or that you have moved BONOBO since it was installed. In the latter case, you"
          echo "*** may want to edit the gnome-config script: $GNOME_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     BONOBO_CFLAGS=""
     BONOBO_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(BONOBO_CFLAGS)
  AC_SUBST(BONOBO_LIBS)
  rm -f conf.bonobotest
])

AC_DEFUN([BONOBO_CHECK], [
	AM_PATH_BONOBO(0.1.0,,[AC_MSG_ERROR(BONOBO not found)])
])

dnl GNOME_UNDELFS_CHECKS
dnl    Check for ext2fs undel support.
dnl    Set shell variable ext2fs_undel to "yes" if we have it,
dnl    "no" otherwise.  May define USE_EXT2FSLIB for cpp.
dnl    Will set EXT2FS_UNDEL_LIBS to required libraries.

AC_DEFUN([GNOME_UNDELFS_CHECKS], [
  AC_CHECK_HEADERS(ext2fs/ext2fs.h linux/ext2_fs.h)
  ext2fs_undel=no
  EXT2FS_UNDEL_LIBS=
  if test x$ac_cv_header_ext2fs_ext2fs_h = xyes
  then
    if test x$ac_cv_header_linux_ext2_fs_h = xyes
    then
      AC_DEFINE(USE_EXT2FSLIB)
      ext2fs_undel=yes
      EXT2FS_UNDEL_LIBS="-lext2fs -lcom_err"
    fi
  fi
])
# gnome-common.m4
# 
# This only for packages that are not in the GNOME CVS tree.

dnl GNOME_COMMON_INIT

AC_DEFUN([GNOME_COMMON_INIT],
[
	GNOME_ACLOCAL_DIR=`$ACLOCAL --print-ac-dir`/gnome
	AC_SUBST(GNOME_ACLOCAL_DIR)

	ACLOCAL="$ACLOCAL -I $GNOME_ACLOCAL_DIR"
])

dnl GNOME_VFS_CHECKS
dnl   Check for various functions needed by libvfs.
dnl   This has various effects:
dnl     Sets GNOME_VFS_LIBS to libraries required
dnl     Sets termnet  to true or false depending on whether it is required.
dnl        If yes, defines USE_TERMNET.
dnl     Sets vfs_flags to "pretty" list of vfs implementations we include.
dnl     Sets shell variable use_vfs to yes (default, --with-vfs) or
dnl        "no" (--without-vfs).
dnl     Calls AC_SUBST(mcserv), which is either empty or "mcserv".

dnl Private define
AC_DEFUN([GNOME_WITH_VFS],[
  dnl FIXME: network checks should probably be in their own macro.
  AC_CHECK_LIB(nsl, t_accept)
  AC_CHECK_LIB(socket, socket)

  have_socket=no
  AC_CHECK_FUNCS(socket, have_socket=yes)
  if test $have_socket = no; then
    # socket is not in the default libraries.  See if it's in some other.
    for lib in bsd socket inet; do
      AC_CHECK_LIB($lib, socket, [
	  LIBS="$LIBS -l$lib"
	  have_socket=yes
	  AC_DEFINE(HAVE_SOCKET)
	  break])
    done
  fi

  have_gethostbyname=no
  AC_CHECK_FUNC(gethostbyname, have_gethostbyname=yes)
  if test $have_gethostbyname = no; then
    # gethostbyname is not in the default libraries.  See if it's in some other.
    for lib in bsd socket inet; do
      AC_CHECK_LIB($lib, gethostbyname, [LIBS="$LIBS -l$lib"; have_gethostbyname=yes; break])
    done
  fi

  vfs_flags="tarfs"
  use_net_code=false
  if test $have_socket = yes; then
      AC_STRUCT_LINGER
      AC_CHECK_FUNCS(pmap_set, , [
	 AC_CHECK_LIB(rpc, pmap_set, [
	   LIBS="-lrpc $LIBS"
	  AC_DEFINE(HAVE_PMAP_SET)
	  ])])
      AC_CHECK_FUNCS(pmap_getport pmap_getmaps rresvport)
      dnl add for source routing support setsockopt
      AC_CHECK_HEADERS(rpc/pmap_clnt.h)
      vfs_flags="$vfs_flags, mcfs, ftpfs, fish"
      use_net_code=true
  fi

  dnl
  dnl Samba support
  dnl
  smbfs=""
  SAMBAFILES=""
  AC_ARG_WITH(samba,
  	  [--with-samba	            Support smb virtual file system],[
  	  if test "x$withval != xno"; then
  		  AC_DEFINE(WITH_SMBFS)
	          vfs_flags="$vfs_flags, smbfs"
		  smbfs="smbfs.o"
		  SAMBAFILES="\$(SAMBAFILES)"
  	  fi
  ])
  AC_SUBST(smbfs)
  AC_SUBST(SAMBAFILES)
  
  dnl
  dnl The termnet support
  dnl
  termnet=false
  AC_ARG_WITH(termnet,
	  [--with-termnet             If you want a termified net support],[
	  if test x$withval = xyes; then
		  AC_DEFINE(USE_TERMNET)
		  termnet=true		
	  fi
  ])

  TERMNET=""
  AC_DEFINE(USE_VFS)
  if $use_net_code; then
     AC_DEFINE(USE_NETCODE)
  fi
  mcserv=
  if test $have_socket = yes; then
     mcserv="mcserv"
     if $termnet; then
	TERMNET="-ltermnet"
     fi
  fi

  AC_SUBST(TERMNET)
  AC_SUBST(mcserv)

dnl FIXME:
dnl GNOME_VFS_LIBS=

])

AC_DEFUN([GNOME_VFS_CHECKS],[
	use_vfs=yes
	AC_ARG_WITH(vfs,
		[--with-vfs		   Compile with the VFS code],
		use_vfs=$withval
	)
	case $use_vfs in
		yes) 	GNOME_WITH_VFS;;
		no) 	use_vfs=no;;
		*)   	use_vfs=no;;
			dnl Should we issue a warning?
	esac
])


dnl
dnl GNOME_FILEUTILS_CHECKS
dnl
dnl checks that are needed for the diskusage applet.
dnl

AC_DEFUN([GNOME_FILEUTILS_CHECKS],
[	
AC_CHECK_HEADERS(fcntl.h sys/param.h sys/statfs.h sys/fstyp.h \
mnttab.h mntent.h sys/statvfs.h sys/vfs.h sys/mount.h \
sys/filsys.h sys/fs_types.h sys/fs/s5param.h)

AC_CHECK_FUNCS(bcopy endgrent endpwent fchdir ftime ftruncate \
getcwd getmntinfo gettimeofday isascii lchown \
listmntent memcpy mkfifo strchr strerror strrchr vprintf)

dnl Set some defaults when cross-compiling

if test x$cross_compiling = xyes ; then
	case "$host_os" in
	linux*)
	  fu_cv_sys_mounted_getmntent1=yes
	  fu_cv_sys_stat_statfs2_bsize=yes
	  ;;
	sunos*)
	  fu_cv_sys_stat_statfs4=yes
	  ;;
	freebsd*)
	  fu_cv_sys_stat_statfs2_bsize=yes
	  ;;
	osf*)
	  fu_cv_sys_stat_statfs3_osf1=yes
	  ;;
	esac
fi

# Determine how to get the list of mounted filesystems.
list_mounted_fs=

# If the getmntent function is available but not in the standard library,
# make sure LIBS contains -lsun (on Irix4) or -lseq (on PTX).
AC_FUNC_GETMNTENT

# This test must precede the ones for getmntent because Unicos-9 is
# reported to have the getmntent function, but its support is incompatible
# with other getmntent implementations.

# NOTE: Normally, I wouldn't use a check for system type as I've done for
# `CRAY' below since that goes against the whole autoconf philosophy.  But
# I think there is too great a chance that some non-Cray system has a
# function named listmntent to risk the false positive.

if test -z "$list_mounted_fs"; then
# Cray UNICOS 9
AC_MSG_CHECKING([for listmntent of Cray/Unicos-9])
AC_CACHE_VAL(fu_cv_sys_mounted_cray_listmntent,
[fu_cv_sys_mounted_cray_listmntent=no
AC_EGREP_CPP(yes,
[#ifdef _CRAY
yes
#endif
], [test $ac_cv_func_listmntent = yes \
&& fu_cv_sys_mounted_cray_listmntent=yes]
)
]
)
AC_MSG_RESULT($fu_cv_sys_mounted_cray_listmntent)
if test $fu_cv_sys_mounted_cray_listmntent = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_LISTMNTENT)
fi
fi

if test $ac_cv_func_getmntent = yes; then

# This system has the getmntent function.
# Determine whether it's the one-argument variant or the two-argument one.

if test -z "$list_mounted_fs"; then
# 4.3BSD, SunOS, HP-UX, Dynix, Irix
AC_MSG_CHECKING([for one-argument getmntent function])
AC_CACHE_VAL(fu_cv_sys_mounted_getmntent1,
[test $ac_cv_header_mntent_h = yes \
&& fu_cv_sys_mounted_getmntent1=yes \
|| fu_cv_sys_mounted_getmntent1=no])
AC_MSG_RESULT($fu_cv_sys_mounted_getmntent1)
if test $fu_cv_sys_mounted_getmntent1 = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_GETMNTENT1)
fi
fi

if test -z "$list_mounted_fs"; then
# SVR4
AC_MSG_CHECKING([for two-argument getmntent function])
AC_CACHE_VAL(fu_cv_sys_mounted_getmntent2,
[AC_EGREP_HEADER(getmntent, sys/mnttab.h,
fu_cv_sys_mounted_getmntent2=yes,
fu_cv_sys_mounted_getmntent2=no)])
AC_MSG_RESULT($fu_cv_sys_mounted_getmntent2)
if test $fu_cv_sys_mounted_getmntent2 = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_GETMNTENT2)
fi
fi

if test -z "$list_mounted_fs"; then
AC_MSG_ERROR([could not determine how to read list of mounted filesystems])
fi

fi

if test -z "$list_mounted_fs"; then
# DEC Alpha running OSF/1.
AC_MSG_CHECKING([for getfsstat function])
AC_CACHE_VAL(fu_cv_sys_mounted_getsstat,
[AC_TRY_LINK([
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/fs_types.h>],
[struct statfs *stats;
int numsys = getfsstat ((struct statfs *)0, 0L, MNT_WAIT); ],
fu_cv_sys_mounted_getsstat=yes,
fu_cv_sys_mounted_getsstat=no)])
AC_MSG_RESULT($fu_cv_sys_mounted_getsstat)
if test $fu_cv_sys_mounted_getsstat = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_GETFSSTAT)
fi
fi

if test -z "$list_mounted_fs"; then
# AIX.
AC_MSG_CHECKING([for mntctl function and struct vmount])
AC_CACHE_VAL(fu_cv_sys_mounted_vmount,
[AC_TRY_CPP([#include <fshelp.h>],
fu_cv_sys_mounted_vmount=yes,
fu_cv_sys_mounted_vmount=no)])
AC_MSG_RESULT($fu_cv_sys_mounted_vmount)
if test $fu_cv_sys_mounted_vmount = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_VMOUNT)
fi
fi

if test -z "$list_mounted_fs"; then
# SVR3
AC_MSG_CHECKING([for FIXME existence of three headers])
AC_CACHE_VAL(fu_cv_sys_mounted_fread_fstyp,
[AC_TRY_CPP([
#include <sys/statfs.h>
#include <sys/fstyp.h>
#include <mnttab.h>],
fu_cv_sys_mounted_fread_fstyp=yes,
fu_cv_sys_mounted_fread_fstyp=no)])
AC_MSG_RESULT($fu_cv_sys_mounted_fread_fstyp)
if test $fu_cv_sys_mounted_fread_fstyp = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_FREAD_FSTYP)
fi
fi

if test -z "$list_mounted_fs"; then
# 4.4BSD and DEC OSF/1.
AC_MSG_CHECKING([for getmntinfo function])
AC_CACHE_VAL(fu_cv_sys_mounted_getmntinfo,
[
ok=
if test $ac_cv_func_getmntinfo = yes; then
AC_EGREP_HEADER(f_type;, sys/mount.h,
ok=yes)
fi
test -n "$ok" \
&& fu_cv_sys_mounted_getmntinfo=yes \
|| fu_cv_sys_mounted_getmntinfo=no
])
AC_MSG_RESULT($fu_cv_sys_mounted_getmntinfo)
if test $fu_cv_sys_mounted_getmntinfo = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_GETMNTINFO)
fi
fi

# FIXME: add a test for netbsd-1.1 here

if test -z "$list_mounted_fs"; then
# Ultrix
AC_MSG_CHECKING([for getmnt function])
AC_CACHE_VAL(fu_cv_sys_mounted_getmnt,
[AC_TRY_CPP([
#include <sys/fs_types.h>
#include <sys/mount.h>],
fu_cv_sys_mounted_getmnt=yes,
fu_cv_sys_mounted_getmnt=no)])
AC_MSG_RESULT($fu_cv_sys_mounted_getmnt)
if test $fu_cv_sys_mounted_getmnt = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_GETMNT)
fi
fi

if test -z "$list_mounted_fs"; then
# SVR2
AC_MSG_CHECKING([whether it is possible to resort to fread on /etc/mnttab])
AC_CACHE_VAL(fu_cv_sys_mounted_fread,
[AC_TRY_CPP([#include <mnttab.h>],
fu_cv_sys_mounted_fread=yes,
fu_cv_sys_mounted_fread=no)])
AC_MSG_RESULT($fu_cv_sys_mounted_fread)
if test $fu_cv_sys_mounted_fread = yes; then
list_mounted_fs=found
AC_DEFINE(MOUNTED_FREAD)
fi
fi

if test -z "$list_mounted_fs"; then
AC_MSG_ERROR([could not determine how to read list of mounted filesystems])
# FIXME -- no need to abort building the whole package
# Can't build mountlist.c or anything that needs its functions
fi

AC_CHECKING(how to get filesystem space usage)
space=no

# Perform only the link test since it seems there are no variants of the
# statvfs function.  This check is more than just AC_CHECK_FUNCS(statvfs)
# because that got a false positive on SCO OSR5.  Adding the declaration
# of a `struct statvfs' causes this test to fail (as it should) on such
# systems.  That system is reported to work fine with STAT_STATFS4 which
# is what it gets when this test fails.
if test $space = no; then
# SVR4
AC_CACHE_CHECK([statvfs function (SVR4)], fu_cv_sys_stat_statvfs,
[AC_TRY_LINK([#include <sys/types.h>
#include <sys/statvfs.h>],
[struct statvfs fsd; statvfs (0, &fsd);],
fu_cv_sys_stat_statvfs=yes,
fu_cv_sys_stat_statvfs=no)])
if test $fu_cv_sys_stat_statvfs = yes; then
space=yes
AC_DEFINE(STAT_STATVFS)
fi
fi

if test $space = no; then
# DEC Alpha running OSF/1
AC_MSG_CHECKING([for 3-argument statfs function (DEC OSF/1)])
AC_CACHE_VAL(fu_cv_sys_stat_statfs3_osf1,
[AC_TRY_RUN([
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mount.h>
main ()
{
struct statfs fsd;
fsd.f_fsize = 0;
exit (statfs (".", &fsd, sizeof (struct statfs)));
}],
fu_cv_sys_stat_statfs3_osf1=yes,
fu_cv_sys_stat_statfs3_osf1=no,
fu_cv_sys_stat_statfs3_osf1=no)])
AC_MSG_RESULT($fu_cv_sys_stat_statfs3_osf1)
if test $fu_cv_sys_stat_statfs3_osf1 = yes; then
space=yes
AC_DEFINE(STAT_STATFS3_OSF1)
fi
fi

if test $space = no; then
# AIX
AC_MSG_CHECKING([for two-argument statfs with statfs.bsize dnl
member (AIX, 4.3BSD)])
AC_CACHE_VAL(fu_cv_sys_stat_statfs2_bsize,
[AC_TRY_RUN([
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
main ()
{
struct statfs fsd;
fsd.f_bsize = 0;
exit (statfs (".", &fsd));
}],
fu_cv_sys_stat_statfs2_bsize=yes,
fu_cv_sys_stat_statfs2_bsize=no,
fu_cv_sys_stat_statfs2_bsize=no)])
AC_MSG_RESULT($fu_cv_sys_stat_statfs2_bsize)
if test $fu_cv_sys_stat_statfs2_bsize = yes; then
space=yes
AC_DEFINE(STAT_STATFS2_BSIZE)
fi
fi

if test $space = no; then
# SVR3
AC_MSG_CHECKING([for four-argument statfs (AIX-3.2.5, SVR3)])
AC_CACHE_VAL(fu_cv_sys_stat_statfs4,
[AC_TRY_RUN([#include <sys/types.h>
#include <sys/statfs.h>
main ()
{
struct statfs fsd;
exit (statfs (".", &fsd, sizeof fsd, 0));
}],
fu_cv_sys_stat_statfs4=yes,
fu_cv_sys_stat_statfs4=no,
fu_cv_sys_stat_statfs4=no)])
AC_MSG_RESULT($fu_cv_sys_stat_statfs4)
if test $fu_cv_sys_stat_statfs4 = yes; then
space=yes
AC_DEFINE(STAT_STATFS4)
fi
fi

if test $space = no; then
# 4.4BSD and NetBSD
AC_MSG_CHECKING([for two-argument statfs with statfs.fsize dnl
member (4.4BSD and NetBSD)])
AC_CACHE_VAL(fu_cv_sys_stat_statfs2_fsize,
[AC_TRY_RUN([#include <sys/types.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
main ()
{
struct statfs fsd;
fsd.f_fsize = 0;
exit (statfs (".", &fsd));
}],
fu_cv_sys_stat_statfs2_fsize=yes,
fu_cv_sys_stat_statfs2_fsize=no,
fu_cv_sys_stat_statfs2_fsize=no)])
AC_MSG_RESULT($fu_cv_sys_stat_statfs2_fsize)
if test $fu_cv_sys_stat_statfs2_fsize = yes; then
space=yes
AC_DEFINE(STAT_STATFS2_FSIZE)
fi
fi

if test $space = no; then
# Ultrix
AC_MSG_CHECKING([for two-argument statfs with struct fs_data (Ultrix)])
AC_CACHE_VAL(fu_cv_sys_stat_fs_data,
[AC_TRY_RUN([#include <sys/types.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_FS_TYPES_H
#include <sys/fs_types.h>
#endif
main ()
{
struct fs_data fsd;
/* Ultrix's statfs returns 1 for success,
0 for not mounted, -1 for failure.  */
exit (statfs (".", &fsd) != 1);
}],
fu_cv_sys_stat_fs_data=yes,
fu_cv_sys_stat_fs_data=no,
fu_cv_sys_stat_fs_data=no)])
AC_MSG_RESULT($fu_cv_sys_stat_fs_data)
if test $fu_cv_sys_stat_fs_data = yes; then
space=yes
AC_DEFINE(STAT_STATFS2_FS_DATA)
fi
fi

if test $space = no; then
# SVR2
AC_TRY_CPP([#include <sys/filsys.h>],
AC_DEFINE(STAT_READ_FILSYS) space=yes)
fi

if test -n "$list_mounted_fs" && test $space != no; then
DF_PROG="df"
# LIBOBJS="$LIBOBJS fsusage.o"
# LIBOBJS="$LIBOBJS mountlist.o"
fi

# Check for SunOS statfs brokenness wrt partitions 2GB and larger.
# If <sys/vfs.h> exists and struct statfs has a member named f_spare,
# enable the work-around code in fsusage.c.
AC_MSG_CHECKING([for statfs that truncates block counts])
AC_CACHE_VAL(fu_cv_sys_truncating_statfs,
[AC_TRY_COMPILE([
#if !defined(sun) && !defined(__sun)
choke -- this is a workaround for a Sun-specific problem
#endif
#include <sys/types.h>
#include <sys/vfs.h>],
[struct statfs t; long c = *(t.f_spare);],
fu_cv_sys_truncating_statfs=yes,
fu_cv_sys_truncating_statfs=no,
)])
if test $fu_cv_sys_truncating_statfs = yes; then
AC_DEFINE(STATFS_TRUNCATES_BLOCK_COUNTS)
fi
AC_MSG_RESULT($fu_cv_sys_truncating_statfs)

AC_CHECKING(for AFS)
test -d /afs && AC_DEFINE(AFS)
])
dnl GNOME_X_CHECKS
dnl
dnl Basic X11 related checks for X11.  At the end, the following will be
dnl defined/changed:
dnl   GTK_{CFLAGS,LIBS}      From AM_PATH_GTK
dnl   CPPFLAGS		     Will include $X_CFLAGS
dnl   GNOME_HAVE_SM	     `true' or `false' depending on whether session
dnl                          management is available.  It is available if
dnl                          both -lSM and X11/SM/SMlib.h exist.  (Some
dnl                          Solaris boxes have the library but not the header)
dnl   XPM_LIBS               -lXpm if Xpm library is present, otherwise ""
dnl
dnl The following configure cache variables are defined (but not used):
dnl   gnome_cv_passdown_{x_libs,X_LIBS,X_CFLAGS}
dnl
AC_DEFUN([GNOME_X_CHECKS],
[
	AM_PATH_GTK(1.2.0,,AC_MSG_ERROR(GTK not installed, or gtk-config not in path))
	dnl Hope that GTK_CFLAGS have only -I and -D.  Otherwise, we could
	dnl   test -z "$x_includes" || CPPFLAGS="$CPPFLAGS -I$x_includes"
	dnl
	dnl Use CPPFLAGS instead of CFLAGS because AC_CHECK_HEADERS uses
	dnl CPPFLAGS, not CFLAGS
        CPPFLAGS="$CPPFLAGS $GTK_CFLAGS"

        saved_ldflags="$LDFLAGS"
        LDFLAGS="$LDFLAGS $GTK_LIBS"

	gnome_cv_passdown_x_libs="$GTK_LIBS"
	gnome_cv_passdown_X_LIBS="$GTK_LIBS"
	gnome_cv_passdown_X_CFLAGS="$GTK_CFLAGS"
	gnome_cv_passdown_GTK_LIBS="$GTK_LIBS"

        LDFLAGS="$saved_ldflags $GTK_LIBS"

dnl We are requiring GTK >= 1.1.1, which means this will be fine anyhow.
	USE_DEVGTK=true

dnl	AC_MSG_CHECKING([whether to use features from (unstable) GTK+ 1.1.x])
dnl	AC_EGREP_CPP(answer_affirmatively,
dnl	[#include <gtk/gtkfeatures.h>
dnl	#ifdef GTK_HAVE_FEATURES_1_1_0
dnl	   answer_affirmatively
dnl	#endif
dnl	], dev_gtk=yes, dev_gtk=no)
dnl	if test "$dev_gtk" = "yes"; then
dnl	   USE_DEVGTK=true
dnl	fi
dnl	AC_MSG_RESULT("$dev_gtk")

	GNOME_HAVE_SM=true
	case "$GTK_LIBS" in
	 *-lSM*)
	    dnl Already found it.
	    ;;
	 *)
	    dnl Assume that if we have -lSM then we also have -lICE.
	    AC_CHECK_LIB(SM, SmcSaveYourselfDone,
	        [GTK_LIBS="-lSM -lICE $GTK_LIBS"],GNOME_HAVE_SM=false,
		$x_libs -lICE)
	    ;;
	esac

	if test "$GNOME_HAVE_SM" = true; then
	   AC_CHECK_HEADERS(X11/SM/SMlib.h,,GNOME_HAVE_SM=false)
	fi

	if test "$GNOME_HAVE_SM" = true; then
	   AC_DEFINE(HAVE_LIBSM)
	fi

	XPM_LIBS=""
	AC_CHECK_LIB(Xpm, XpmFreeXpmImage, [XPM_LIBS="-lXpm"], , $x_libs)
	AC_SUBST(XPM_LIBS)

	AC_REQUIRE([GNOME_PTHREAD_CHECK])
        LDFLAGS="$saved_ldflags"

	AC_PROVIDE([GNOME_X_CHECKS])
])
AC_DEFUN([GNOME_GHTTP_CHECK],[
	AC_REQUIRE([GNOME_INIT_HOOK])
	GHTTP_LIB=
	AC_CHECK_FUNC(connect,,[
	  AC_CHECK_LIB(socket,connect,
		GHTTP_LIB="-lsocket $GHTTP_LIB",,$GHTTP_LIB)])
	AC_CHECK_FUNC(gethostbyname,,[
 	  AC_CHECK_LIB(nsl,gethostbyname,
		GHTTP_LIB="-lnsl $GHTTP_LIB",,$GHTTP_LIB)])
	AC_CHECK_LIB(ghttp, ghttp_request_new, 
		GHTTP_LIB="-lghttp $GHTTP_LIB",GHTTP_LIB="",-L$gnome_prefix $GHTTP_LIB)
	AC_SUBST(GHTTP_LIB)
	AC_PROVIDE([GNOME_GHTTP_CHECK])
])
dnl
dnl GNOME_XML_HOOK (script-if-xml-found, failflag)
dnl
dnl If failflag is "failure", script aborts due to lack of XML
dnl 
dnl Check for availability of the libxml library
dnl the XML parser uses libz if available too
dnl

AC_DEFUN([GNOME_XML_HOOK],[
	AC_PATH_PROG(GNOME_CONFIG,gnome-config,no)
	if test "$GNOME_CONFIG" = no; then
		if test x$2 = xfailure; then
			AC_MSG_ERROR(Could not find gnome-config)
		fi
	fi
	AC_CHECK_LIB(xml, xmlNewDoc, [
		$1
		GNOME_XML_LIB=`$GNOME_CONFIG --libs xml`
	], [
		if test x$2 = xfailure; then 
			AC_MSG_ERROR(Could not link sample xml program)
		fi
	], `$GNOME_CONFIG --libs xml`)
	AC_SUBST(GNOME_XML_LIB)
])

AC_DEFUN([GNOME_XML_CHECK], [
	GNOME_XML_HOOK([],failure)
])
dnl
dnl GNOME_GNORBA_HOOK (script-if-gnorba-found, failflag)
dnl
dnl if failflag is "failure" it aborts if gnorba is not found.
dnl

AC_DEFUN([GNOME_GNORBA_HOOK],[
	GNOME_ORBIT_HOOK([],$2)
	AC_CACHE_CHECK([for gnorba libraries],gnome_cv_gnorba_found,[
		gnome_cv_gnorba_found=no
		if test x$gnome_cv_orbit_found = xyes; then
			GNORBA_CFLAGS="`gnome-config --cflags gnorba gnomeui`"
			GNORBA_LIBS="`gnome-config --libs gnorba gnomeui`"
			if test -n "$GNORBA_LIBS"; then
				gnome_cv_gnorba_found=yes
			fi
		fi
	])
	AM_CONDITIONAL(HAVE_GNORBA, test x$gnome_cv_gnorba_found = xyes)
	if test x$gnome_cv_orbit_found = xyes; then
		$1
		GNORBA_CFLAGS="`gnome-config --cflags gnorba gnomeui`"
		GNORBA_LIBS="`gnome-config --libs gnorba gnomeui`"
		AC_SUBST(GNORBA_CFLAGS)
		AC_SUBST(GNORBA_LIBS)
	else
	    	if test x$2 = xfailure; then
			AC_MSG_ERROR(gnorba library not installed or installation problem)
	    	fi
	fi
])

AC_DEFUN([GNOME_GNORBA_CHECK], [
	GNOME_GNORBA_HOOK([],failure)
])
dnl
dnl GNOME_INIT_HOOK (script-if-gnome-enabled, [failflag], [additional-inits])
dnl
dnl if failflag is "fail" then GNOME_INIT_HOOK will abort if gnomeConf.sh
dnl is not found. 
dnl

AC_DEFUN([GNOME_INIT_HOOK],[
	AC_SUBST(GNOME_LIBS)
	AC_SUBST(GNOMEUI_LIBS)
	AC_SUBST(GNOMEGNORBA_LIBS)
	AC_SUBST(GTKXMHTML_LIBS)
	AC_SUBST(ZVT_LIBS)
	AC_SUBST(GNOME_LIBDIR)
	AC_SUBST(GNOME_INCLUDEDIR)

	AC_ARG_WITH(gnome-includes,
	[  --with-gnome-includes   Specify location of GNOME headers],[
	CFLAGS="$CFLAGS -I$withval"
	])
	
	AC_ARG_WITH(gnome-libs,
	[  --with-gnome-libs       Specify location of GNOME libs],[
	LDFLAGS="$LDFLAGS -L$withval"
	gnome_prefix=$withval
	])

	AC_ARG_WITH(gnome,
	[  --with-gnome            Specify prefix for GNOME files],
		if test x$withval = xyes; then
	    		want_gnome=yes
	    		dnl Note that an empty true branch is not
			dnl valid sh syntax.
	    		ifelse([$1], [], :, [$1])
        	else
	    		if test "x$withval" = xno; then
	        		want_gnome=no
	    		else
	        		want_gnome=yes
	    			LDFLAGS="$LDFLAGS -L$withval/lib"
	    			CFLAGS="$CFLAGS -I$withval/include"
	    			gnome_prefix=$withval/lib
	    		fi
  		fi,
		want_gnome=yes)

	if test "x$want_gnome" = xyes; then

	    AC_PATH_PROG(GNOME_CONFIG,gnome-config,no)
	    if test "$GNOME_CONFIG" = "no"; then
	      no_gnome_config="yes"
	    else
	      AC_MSG_CHECKING(if $GNOME_CONFIG works)
	      if $GNOME_CONFIG --libs-only-l gnome >/dev/null 2>&1; then
	        AC_MSG_RESULT(yes)
	        GNOME_GNORBA_HOOK([],$2)
	        GNOME_LIBS="`$GNOME_CONFIG --libs-only-l gnome`"
	        GNOMEUI_LIBS="`$GNOME_CONFIG --libs-only-l gnomeui`"
	        GNOMEGNORBA_LIBS="`$GNOME_CONFIG --libs-only-l gnorba gnomeui`"
	        GTKXMHTML_LIBS="`$GNOME_CONFIG --libs-only-l gtkxmhtml`"
		ZVT_LIBS="`$GNOME_CONFIG --libs-only-l zvt`"
	        GNOME_LIBDIR="`$GNOME_CONFIG --libs-only-L gnorba gnomeui`"
	        GNOME_INCLUDEDIR="`$GNOME_CONFIG --cflags gnorba gnomeui`"
                $1
	      else
	        AC_MSG_RESULT(no)
	        no_gnome_config="yes"
              fi
            fi

	    if test x$exec_prefix = xNONE; then
	        if test x$prefix = xNONE; then
		    gnome_prefix=$ac_default_prefix/lib
	        else
 		    gnome_prefix=$prefix/lib
	        fi
	    else
	        gnome_prefix=`eval echo \`echo $libdir\``
	    fi
	
	    if test "$no_gnome_config" = "yes"; then
              AC_MSG_CHECKING(for gnomeConf.sh file in $gnome_prefix)
	      if test -f $gnome_prefix/gnomeConf.sh; then
	        AC_MSG_RESULT(found)
	        echo "loading gnome configuration from" \
		     "$gnome_prefix/gnomeConf.sh"
	        . $gnome_prefix/gnomeConf.sh
	        $1
	      else
	        AC_MSG_RESULT(not found)
 	        if test x$2 = xfail; then
	          AC_MSG_ERROR(Could not find the gnomeConf.sh file that is generated by gnome-libs install)
 	        fi
	      fi
            fi
	fi

	if test -n "$3"; then
	  n="$3"
	  for i in $n; do
	    AC_MSG_CHECKING(extra library \"$i\")
	    case $i in 
	      applets)
		AC_SUBST(GNOME_APPLETS_LIBS)
		GNOME_APPLETS_LIBS=`$GNOME_CONFIG --libs-only-l applets`
		AC_MSG_RESULT($GNOME_APPLETS_LIBS);;
	      docklets)
		AC_SUBST(GNOME_DOCKLETS_LIBS)
		GNOME_DOCKLETS_LIBS=`$GNOME_CONFIG --libs-only-l docklets`
		AC_MSG_RESULT($GNOME_DOCKLETS_LIBS);;
	      capplet)
		AC_SUBST(GNOME_CAPPLET_LIBS)
		GNOME_CAPPLET_LIBS=`$GNOME_CONFIG --libs-only-l capplet`
		AC_MSG_RESULT($GNOME_CAPPLET_LIBS);;
	      *)
		AC_MSG_RESULT(unknown library)
	    esac
	  done
	fi
])

dnl
dnl GNOME_INIT ([additional-inits])
dnl

AC_DEFUN([GNOME_INIT],[
	GNOME_INIT_HOOK([],fail,$1)
])
dnl
dnl GNOME_CHECK_GUILE (failflag)
dnl
dnl if failflag is "fail" then GNOME_CHECK_GUILE will abort if guile is not found.
dnl

AC_DEFUN([GNOME_CHECK_GUILE],
[
	saved_ldflags="$LDFLAGS"
	saved_cppflags="$CPPFLAGS"
	LDFLAGS="$LDFLAGS $GNOME_LIBDIR"

	AC_CHECK_LIB(qthreads,qt_null,[
		QTTHREADS_LIB="-lqthreads"
	],[
		AC_CHECK_LIB(qt, qt_null, QTTHREADS_LIB="-lqt")
	],$LIBS)
	AC_SUBST(QTTHREADS_LIB)

	AC_CHECK_LIB(termcap,main,TERMCAP_LIB="-ltermcap")
	AC_CHECK_LIB(readline,main,READLINE_LIB="-lreadline",,$TERMCAP_LIB)

	AC_SUBST(TERMCAP_LIB)
	AC_SUBST(READLINE_LIB)

	if test "x$cross_compiling" = "xyes" ; then
	  name_build_guile="$target_alias-guile-config"
	else
	  name_build_guile="guile-config"
	fi

	AC_CHECK_PROG(BUILD_GUILE, $name_build_guile, yes, no)

	if test "x$BUILD_GUILE" = "xyes"; then
	    AC_MSG_CHECKING(whether $name_build_guile works)
	    if test x`$name_build_guile --version >/dev/null 2>&1 || \
		echo no` = xno; then
		BUILD_GUILE=no
	    fi
	    AC_MSG_RESULT($BUILD_GUILE)
	else

	    if test "x$cross_compiling" = "xyes" ; then
		name_build_guile="$target_alias-build-guile"
	    else	
		name_build_guile="build-guile"
	    fi

	    AC_CHECK_PROG(BUILD_GUILE, $name_build_guile, yes, no)

	    if test "x$BUILD_GUILE" = "xyes"; then
		AC_MSG_CHECKING(whether $name_build_guile works)
		if test x`$name_build_guile --version >/dev/null 2>&1 || \
	 	    echo no` = xno; then
		    BUILD_GUILE=no
		fi
		AC_MSG_RESULT($BUILD_GUILE)
	    fi
	fi

	AC_CHECK_LIB(m, sin)

	if test "x$BUILD_GUILE" = "xyes"; then
		AC_MSG_CHECKING(for guile libraries)
		GUILE_LIBS="`$name_build_guile link`"
		AC_MSG_RESULT($GUILE_LIBS)
		AC_MSG_CHECKING(for guile headers)
		GUILE_INCS="`$name_build_guile compile`"
		AC_MSG_RESULT($GUILE_INCS)
	else
		GUILE_LIBS="$GNOME_LIBDIR"
		GUILE_INCS="$GNOME_INCLUDEDIR"
		AC_CHECK_LIB(rx, main, GUILE_LIBS="-lrx $GUILE_LIBS")
		AC_CHECK_LIB(qt, qt_null, GUILE_LIBS="-lqt $GUILE_LIBS")
		AC_CHECK_LIB(dl, dlopen, GUILE_LIBS="-ldl $GUILE_LIBS")
		AC_CHECK_LIB(nsl, t_accept, GUILE_LIBS="$GUILE_LIBS -lnsl")
		AC_CHECK_LIB(socket, socket, GUILE_LIBS="$GUILE_LIBS -lsocket")
		GUILE_LIBS="-lguile $GUILE_LIBS $QTTHREADS_LIB $READLINE_LIB $TERMCAP_LIB"
	fi

	AC_SUBST(GUILE_LIBS)
	AC_SUBST(GUILE_INCS)

	saved_LIBS="$LIBS"
	LIBS="$LIBS $GUILE_LIBS"
	CPPFLAGS="$saved_cppflags $GUILE_INCS"

	AC_MSG_CHECKING(whether guile works)
	AC_TRY_LINK([
		#include <libguile.h>
		#include <guile/gh.h>
	],[
		gh_eval_str("(newline)");
		scm_boot_guile(0,NULL,NULL,NULL);
	],[
		ac_cv_guile_found=yes
		AC_DEFINE(HAVE_GUILE)
	],[
		ac_cv_guile_found=no
	])
	AC_MSG_RESULT($ac_cv_guile_found)

	if test x$ac_cv_guile_found = xno ; then
		if test x$1 = xfail ; then
		  AC_MSG_ERROR(Can not find Guile on this system)
		else
		  AC_MSG_WARN(Can not find Guile on this system)
		fi
		ac_cv_guile_found=no
		GUILE_LIBS= GUILE_INCS=
	fi

	LIBS="$saved_LIBS"
	LDFLAGS="$saved_ldflags"
	CPPFLAGS="$saved_cppflags"

	AC_SUBST(GUILE_LIBS)
	AM_CONDITIONAL(GUILE, test x$ac_cv_guile_found = xyes)
])
dnl
dnl AC_PROG_GPERF (MINIMUM-VERSION)
dnl
dnl Check for availability of gperf.
dnl Abort if not found or if current version is not up to par.
dnl

AC_DEFUN([AC_PROG_GPERF],[
	AC_PATH_PROG(GPERF, gperf, no)
	if test "$GPERF" = no; then
		AC_MSG_ERROR(Could not find gperf)
	fi
	min_gperf_version=ifelse([$1], ,2.7,$1)
	AC_MSG_CHECKING(for gperf - version >= $min_gperf_version)
	gperf_major_version=`$GPERF --version | \
		sed 's/GNU gperf \([[0-9]]*\).\([[0-9]]*\)/\1/'`
	gperf_minor_version=`$GPERF --version | \
		sed 's/GNU gperf \([[0-9]]*\).\([[0-9]]*\)/\2/'`
	no_gperf=""
dnl
dnl Now check if the installed gperf is sufficiently new.
dnl
	AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int 
main ()
{
  char  *tmp_version;
  
  int    major;
  int    minor;

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_gperf_version");
  if (sscanf(tmp_version, "%d.%d", &major, &minor) != 2) {
    printf ("%s, bad version string\n", "$min_gperf_version");
    exit (1);
  }

  if (($gperf_major_version > major) ||
      (($gperf_major_version == major) && ($gperf_minor_version >= minor))) {
    return 0;
  } else {
    printf ("\n");
    printf ("*** An old version of gperf ($gperf_major_version.$gperf_minor_version) was found.\n");
    printf ("*** You need a version of gperf newer than %d.%d.%d.  The latest version of\n",
	       major, minor);
    printf ("*** gperf is always available from ftp://ftp.gnu.org.\n");
    printf ("***\n");
    return 1;
  }
}
],,no_gperf=yes,[/bin/true])
	if test "x$no_gperf" = x ; then
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
	fi

])
dnl
dnl LIBGTOP_CHECK_TYPE
dnl
dnl Improved version of AC_CHECK_TYPE which takes into account
dnl that we need to #include some other header files on some
dnl systems to get some types.

dnl AC_LIBGTOP_CHECK_TYPE(TYPE, DEFAULT)
AC_DEFUN(AC_LIBGTOP_CHECK_TYPE,
[AC_REQUIRE([AC_HEADER_STDC])dnl
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(ac_cv_type_$1,
[AC_EGREP_CPP(dnl
changequote(<<,>>)dnl
<<(^|[^a-zA-Z_0-9])$1[^a-zA-Z_0-9]>>dnl
changequote([,]), [#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif

/* For Tru64 */
#ifdef HAVE_SYS_BITYPES_H
#include <sys/bitypes.h>
#endif
], ac_cv_type_$1=yes, ac_cv_type_$1=no)])dnl
AC_MSG_RESULT($ac_cv_type_$1)
if test $ac_cv_type_$1 = no; then
  AC_DEFINE($1, $2)
fi
])

dnl
dnl GNOME_LIBGTOP_TYPES
dnl
dnl some typechecks for libgtop.
dnl

AC_DEFUN([GNOME_LIBGTOP_TYPES],
[
	AC_CHECK_HEADERS(sys/bitypes.h)
	AC_LIBGTOP_CHECK_TYPE(u_int64_t, unsigned long long int)
	AC_LIBGTOP_CHECK_TYPE(int64_t, signed long long int)
])

dnl
dnl GNOME_LIBGTOP_HOOK (minversion, script-if-libgtop-enabled, failflag)
dnl
dnl if failflag is "fail" then GNOME_LIBGTOP_HOOK will abort if LibGTop
dnl is not found. 
dnl

AC_DEFUN([GNOME_LIBGTOP_HOOK],
[	
	AC_REQUIRE([GNOME_LIBGTOP_TYPES])

	AC_SUBST(LIBGTOP_LIBDIR)
	AC_SUBST(LIBGTOP_INCLUDEDIR)
	AC_SUBST(LIBGTOP_EXTRA_LIBS)
	AC_SUBST(LIBGTOP_LIBS)
	AC_SUBST(LIBGTOP_INCS)
	AC_SUBST(LIBGTOP_NAMES_LIBS)
	AC_SUBST(LIBGTOP_NAMES_INCS)
	AC_SUBST(LIBGTOP_MAJOR_VERSION)
	AC_SUBST(LIBGTOP_MINOR_VERSION)
	AC_SUBST(LIBGTOP_MICRO_VERSION)
	AC_SUBST(LIBGTOP_VERSION)
	AC_SUBST(LIBGTOP_VERSION_CODE)
	AC_SUBST(LIBGTOP_SERVER_VERSION)
	AC_SUBST(LIBGTOP_INTERFACE_AGE)
	AC_SUBST(LIBGTOP_BINARY_AGE)
	AC_SUBST(LIBGTOP_BINDIR)
	AC_SUBST(LIBGTOP_SERVER)

	dnl Get the cflags and libraries from the libgtop-config script
	dnl
	AC_ARG_WITH(libgtop,
	[  --with-libgtop=PFX      Prefix where LIBGTOP is installed (optional)],
	libgtop_config_prefix="$withval", libgtop_config_prefix="")
	AC_ARG_WITH(libgtop-exec,
	[  --with-libgtop-exec=PFX Exec prefix where LIBGTOP is installed (optional)],
	libgtop_config_exec_prefix="$withval", libgtop_config_exec_prefix="")

	if test x$libgtop_config_exec_prefix != x ; then
	  libgtop_config_args="$libgtop_config_args --exec-prefix=$libgtop_config_exec_prefix"
	  if test x${LIBGTOP_CONFIG+set} != xset ; then
	    LIBGTOP_CONFIG=$libgtop_config_exec_prefix/bin/libgtop-config
	  fi
	fi
	if test x$libgtop_config_prefix != x ; then
	  libgtop_config_args="$libgtop_config_args --prefix=$libgtop_config_prefix"
	  if test x${LIBGTOP_CONFIG+set} != xset ; then
	    LIBGTOP_CONFIG=$libgtop_config_prefix/bin/libgtop-config
	  fi
	fi

	AC_PATH_PROG(LIBGTOP_CONFIG, libgtop-config, no)
	dnl IMPORTANT NOTICE:
	dnl   If you increase this number here, this means that *ALL*
	dnl   modules will require the new version, even if they explicitly
	dnl   give a lower number in their `configure.in' !!!
	real_min_libgtop_version=1.0.0
	min_libgtop_version=ifelse([$1], ,$real_min_libgtop_version,$1)
	dnl I know, the following code looks really ugly, but if you want
	dnl to make changes, please test it with a brain-dead /bin/sh and
	dnl with a brain-dead /bin/test (not all shells/tests support the
	dnl `<' operator to compare strings, that's why I convert everything
	dnl into numbers and test them).
	min_libgtop_major=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	min_libgtop_minor=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	min_libgtop_micro=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	test x$min_libgtop_micro = x && min_libgtop_micro=0
	real_min_libgtop_major=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	real_min_libgtop_minor=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	real_min_libgtop_micro=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	test x$real_min_libgtop_micro = x && real_min_libgtop_micro=0
	dnl You cannot require a version less then $real_min_libgtop_version,
	dnl so you don't need to update each `configure.in' when it's increased.
	if test $real_min_libgtop_major -gt $min_libgtop_major ; then
	  min_libgtop_major=$real_min_libgtop_major
	  min_libgtop_minor=$real_min_libgtop_minor
	  min_libgtop_micro=$real_min_libgtop_micro
	elif test $real_min_libgtop_major = $min_libgtop_major ; then
	  if test $real_min_libgtop_minor -gt $min_libgtop_minor ; then
	    min_libgtop_minor=$real_min_libgtop_minor
	    min_libgtop_micro=$real_min_libgtop_micro
	  elif test $real_min_libgtop_minor = $min_libgtop_minor ; then
	    if test $real_min_libgtop_micro -gt $min_libgtop_micro ; then
	      min_libgtop_micro=$real_min_libgtop_micro
	    fi
	  fi
	fi
	min_libgtop_version="$min_libgtop_major.$min_libgtop_minor.$min_libgtop_micro"
	AC_MSG_CHECKING(for libgtop - version >= $min_libgtop_version)
	no_libgtop=""
	if test "$LIBGTOP_CONFIG" = "no" ; then
	  no_libgtop=yes
	else
	  configfile=`$LIBGTOP_CONFIG --config`
	  libgtop_major_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	  libgtop_minor_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	  libgtop_micro_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	  if test $libgtop_major_version != $min_libgtop_major ; then 
	    no_libgtop=mismatch
	  else 
 	    test $libgtop_minor_version -lt $min_libgtop_minor && no_libgtop=yes
	    if test $libgtop_minor_version = $min_libgtop_minor ; then
	      test $libgtop_micro_version -lt $min_libgtop_micro && no_libgtop=yes
	    fi
	  fi
	  . $configfile
	fi
	if test x$no_libgtop = x ; then
	  AC_DEFINE(HAVE_LIBGTOP)
	  AC_DEFINE_UNQUOTED(LIBGTOP_VERSION, "$LIBGTOP_VERSION")
	  AC_DEFINE_UNQUOTED(LIBGTOP_VERSION_CODE, $LIBGTOP_VERSION_CODE)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MAJOR_VERSION, $LIBGTOP_MAJOR_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MINOR_VERSION, $LIBGTOP_MINOR_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MICRO_VERSION, $LIBGTOP_MICRO_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_SERVER_VERSION, $LIBGTOP_SERVER_VERSION)
	  AC_MSG_RESULT(yes)
	  dnl Note that an empty true branch is not valid sh syntax.
	  ifelse([$2], [], :, [$2])
	else
	  AC_MSG_RESULT(no)
	  if test "$no_libgtop"x = mismatchx; then
	    AC_MSG_ERROR(LibGTop major version mismatch $libgtop_major_version != $min_libgtop_major)
	  fi
	  if test "x$3" = "xfail"; then
	    AC_MSG_ERROR(LibGTop >= $min_libgtop_version not found)
	  else
	    AC_MSG_WARN(LibGTop >= $min_libgtop_version not found)
	  fi
	fi

	AM_CONDITIONAL(HAVE_LIBGTOP, test x$no_libgtop != xyes)
])

AC_DEFUN([GNOME_INIT_LIBGTOP],[
	GNOME_LIBGTOP_HOOK($1,[ifelse([$3], [], :, [$3])],$2)
])

dnl
dnl GNOME_LIBGTOP_DOCU
dnl
dnl checks whether the documentation of LibGTop is installed
dnl

AC_DEFUN([GNOME_LIBGTOP_DOCU],
[
	AC_REQUIRE([GNOME_LIBGTOP_HOOK])

	helpdir="$LIBGTOP_DATADIR/gnome/help/libgtop"

	AC_MSG_CHECKING(whether you have the LibGTop Documentation)

	if test -f "$helpdir/C/topic.dat" ; then
	  have_libgtop_docu=yes
	  AC_DEFINE(HAVE_LIBGTOP_DOCU)
	else
	  have_libgtop_docu=no
	fi

	AC_MSG_RESULT($have_libgtop_docu)

	AM_CONDITIONAL(HAVE_LIBGTOP_DOCU, test x$have_libgtop_docu = xyes)
])

dnl
dnl Check for struct linger
dnl
AC_DEFUN(AC_STRUCT_LINGER, [
av_struct_linger=no
AC_MSG_CHECKING(struct linger is available)
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/socket.h>

struct linger li;

main ()
{
    li.l_onoff = 1;
    li.l_linger = 120;
    exit (0);
}
],[
AC_DEFINE(HAVE_STRUCT_LINGER)
av_struct_linger=yes
],[
av_struct_linger=no
],[
av_struct_linger=no
])
AC_MSG_RESULT($av_struct_linger)
])
AC_DEFUN([GNOME_CHECK_OBJC],
[
dnl Look for an ObjC compiler.
dnl FIXME: extend list of possible names of ObjC compilers.
  AC_CHECK_PROGS(OBJC, $OBJC egcs, "")
  if test "x$OBJC" = "x" ; then
    AC_CHECK_PROGS(OBJC, $OBJC egcc, "")
    if test "x$OBJC" = "x" ; then
      AC_CHECK_PROGS(OBJC, $OBJC gcc, "")
    fi
  fi

  AC_REQUIRE([GNOME_PTHREAD_CHECK])

  OBJC_LIBS="-lobjc $PTHREAD_LIB"
  AC_CHECK_FUNC(sched_yield,,[
    AC_CHECK_LIB(rt,sched_yield,
      OBJC_LIBS="$OBJC_LIBS -lrt",[
      AC_CHECK_LIB(posix4,sched_yield,
        OBJC_LIBS="$OBJC_LIBS -lposix4",, 
        $OBJC_LIBS)],
      $OBJC_LIBS)])
  AC_SUBST(OBJC_LIBS)

  AC_CACHE_CHECK([if Objective C compiler ($OBJC) works],
		 ac_cv_prog_objc_works, [
    if test -n "$OBJC"; then
      cat > conftest.m <<EOF
#include <objc/Object.h>
@interface myRandomObj : Object
{
}
@end
@implementation myRandomObj
@end
int main () {
  /* No, you are not seeing double.  Remember that square brackets
     are the autoconf m4 quotes.  */
  id myid = [[myRandomObj alloc]];
  [[myid free]];
  return 0;
}
EOF

      $OBJC $CFLAGS -o conftest $LDFLAGS conftest.m $OBJC_LIBS 1>&AC_FD_CC 2>&1
      result=$?
      rm -f conftest*

      if test $result -eq 0; then
        ac_cv_prog_objc_works=yes
      fi
    else
      ac_cv_prog_objc_works=no
    fi
  ])

  AM_CONDITIONAL(OBJECTIVE_C, test x$ac_cv_prog_objc_works = xyes)
  dnl Also set the shell variable OBJECTIVE_C to "yes" or "no".
  OBJECTIVE_C=$ac_cv_prog_objc_works
])

AC_DEFUN([GNOME_INIT_OBJC],
[
	AC_MSG_CHECKING(for an obGnomeConf.sh)
	my_gnome_libdir=`$GNOME_CONFIG --libdir`
	if test -f $my_gnome_libdir/obGnomeConf.sh; then
	    . $my_gnome_libdir/obGnomeConf.sh
	    AC_MSG_RESULT(found $my_gnome_libdir)
	    ac_cv_have_gnome_objc=yes
	else
	    AC_MSG_RESULT(not found)
	    AC_MSG_WARN(Could not find the obGnomeConf.sh file that is generated by gnome-objc install)
	    ac_cv_have_gnome_objc=no
	fi
	
	dnl Add a conditional on whether or not we have gnome-objc
	AM_CONDITIONAL(HAVE_GNOME_OBJC, test x$ac_cv_have_gnome_objc = xyes)
	HAVE_GNOME_OBJC=$ac_cv_have_gnome_objc

	AC_SUBST(OBGNOME_INCLUDEDIR)
	AC_SUBST(OBGNOME_LIBS)
	AC_SUBST(OBGTK_LIBS)
])
dnl See whether we need a declaration for a function.
dnl GCC_NEED_DECLARATION(FUNCTION [, EXTRA-HEADER-FILES])
AC_DEFUN(GCC_NEED_DECLARATION,
[AC_MSG_CHECKING([whether $1 must be declared])
AC_CACHE_VAL(gcc_cv_decl_needed_$1,
[AC_TRY_COMPILE([
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
$2],
[char *(*pfn) = (char *(*)) $1],
eval "gcc_cv_decl_needed_$1=no", eval "gcc_cv_decl_needed_$1=yes")])
if eval "test \"`echo '$gcc_cv_decl_needed_'$1`\" = yes"; then
  AC_MSG_RESULT(yes)
  gcc_need_declarations="$gcc_need_declarations $1"
  gcc_tr_decl=NEED_DECLARATION_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
  AC_DEFINE_UNQUOTED($gcc_tr_decl)
else
  AC_MSG_RESULT(no)
fi
])dnl

dnl Check multiple functions to see whether each needs a declaration.
dnl GCC_NEED_DECLARATIONS(FUNCTION... [, EXTRA-HEADER-FILES])
AC_DEFUN(GCC_NEED_DECLARATIONS,
[for ac_func in $1
do
GCC_NEED_DECLARATION($ac_func, $2)
done
]
)
dnl
dnl GNOME_ORBIT_HOOK (script-if-orbit-found, failflag)
dnl
dnl if failflag is "failure" it aborts if orbit is not found.
dnl

AC_DEFUN([GNOME_ORBIT_HOOK],[
	AC_PATH_PROG(ORBIT_CONFIG,orbit-config,no)
	AC_PATH_PROG(ORBIT_IDL,orbit-idl,no)
	AC_CACHE_CHECK([for working ORBit environment],gnome_cv_orbit_found,[
		if test x$ORBIT_CONFIG = xno -o x$ORBIT_IDL = xno; then
			gnome_cv_orbit_found=no
		else
			gnome_cv_orbit_found=yes
		fi
	])
	AM_CONDITIONAL(HAVE_ORBIT, test x$gnome_cv_orbit_found = xyes)
	if test x$gnome_cv_orbit_found = xyes; then
		$1
		ORBIT_CFLAGS=`orbit-config --cflags client server`
		ORBIT_LIBS=`orbit-config --use-service=name --libs client server`
		AC_SUBST(ORBIT_CFLAGS)
		AC_SUBST(ORBIT_LIBS)
	else
    		if test x$2 = xfailure; then
			AC_MSG_ERROR(ORBit not installed or installation problem)
    		fi
	fi
])

AC_DEFUN([GNOME_ORBIT_CHECK], [
	GNOME_ORBIT_HOOK([],failure)
])
# Configure paths for GTK--
# Erik Andersen	30 May 1998
# Modified by Tero Pulkkinen (added the compiler checks... I hope they work..)
# Modified by Thomas Langen 16 Jan 2000 (corrected CXXFLAGS)

dnl Test for GTKMM, and define GTKMM_CFLAGS and GTKMM_LIBS
dnl   to be used as follows:
dnl AM_PATH_GTKMM([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN(AM_PATH_GTKMM,
[dnl 
dnl Get the cflags and libraries from the gtkmm-config script
dnl
AC_ARG_WITH(gtkmm-prefix,[  --with-gtkmm-prefix=PREFIX
                          Prefix where GTK-- is installed (optional)],
            gtkmm_config_prefix="$withval", gtkmm_config_prefix="")
AC_ARG_WITH(gtkmm-exec-prefix,[  --with-gtkmm-exec-prefix=PREFIX
                          Exec prefix where GTK-- is installed (optional)],
            gtkmm_config_exec_prefix="$withval", gtkmm_config_exec_prefix="")
AC_ARG_ENABLE(gtkmmtest, [  --disable-gtkmmtest     Do not try to compile and run a test GTK-- program],
		    , enable_gtkmmtest=yes)

  if test x$gtkmm_config_exec_prefix != x ; then
     gtkmm_config_args="$gtkmm_config_args --exec-prefix=$gtkmm_config_exec_prefix"
     if test x${GTKMM_CONFIG+set} != xset ; then
        GTKMM_CONFIG=$gtkmm_config_exec_prefix/bin/gtkmm-config
     fi
  fi
  if test x$gtkmm_config_prefix != x ; then
     gtkmm_config_args="$gtkmm_config_args --prefix=$gtkmm_config_prefix"
     if test x${GTKMM_CONFIG+set} != xset ; then
        GTKMM_CONFIG=$gtkmm_config_prefix/bin/gtkmm-config
     fi
  fi

  AC_PATH_PROG(GTKMM_CONFIG, gtkmm-config, no)
  min_gtkmm_version=ifelse([$1], ,0.10.0,$1)

  AC_MSG_CHECKING(for GTK-- - version >= $min_gtkmm_version)
  no_gtkmm=""
  if test "$GTKMM_CONFIG" = "no" ; then
    no_gtkmm=yes
  else
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS

    GTKMM_CFLAGS=`$GTKMM_CONFIG $gtkmm_config_args --cflags`
    GTKMM_LIBS=`$GTKMM_CONFIG $gtkmm_config_args --libs`
    gtkmm_config_major_version=`$GTKMM_CONFIG $gtkmm_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtkmm_config_minor_version=`$GTKMM_CONFIG $gtkmm_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtkmm_config_micro_version=`$GTKMM_CONFIG $gtkmm_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtkmmtest" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $GTKMM_CFLAGS"
      LIBS="$LIBS $GTKMM_LIBS"
dnl
dnl Now check if the installed GTK-- is sufficiently new. (Also sanity
dnl checks the results of gtkmm-config to some extent
dnl
      rm -f conf.gtkmmtest
      AC_TRY_RUN([
#include <gtk--.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtkmmtest");

  /* HP/UX 0 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtkmm_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtkmm_version");
     exit(1);
   }

  if ((gtkmm_major_version != $gtkmm_config_major_version) ||
      (gtkmm_minor_version != $gtkmm_config_minor_version) ||
      (gtkmm_micro_version != $gtkmm_config_micro_version))
    {
      printf("\n*** 'gtkmm-config --version' returned %d.%d.%d, but GTK-- (%d.%d.%d)\n", 
             $gtkmm_config_major_version, $gtkmm_config_minor_version, $gtkmm_config_micro_version,
             gtkmm_major_version, gtkmm_minor_version, gtkmm_micro_version);
      printf ("*** was found! If gtkmm-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK--. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gtkmm-config was wrong, set the environment variable GTKMM_CONFIG\n");
      printf("*** to point to the correct copy of gtkmm-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
/* GTK-- does not have the GTKMM_*_VERSION constants */
/* 
  else if ((gtkmm_major_version != GTKMM_MAJOR_VERSION) ||
	   (gtkmm_minor_version != GTKMM_MINOR_VERSION) ||
           (gtkmm_micro_version != GTKMM_MICRO_VERSION))
    {
      printf("*** GTK-- header files (version %d.%d.%d) do not match\n",
	     GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     gtkmm_major_version, gtkmm_minor_version, gtkmm_micro_version);
    }
*/
  else
    {
      if ((gtkmm_major_version > major) ||
        ((gtkmm_major_version == major) && (gtkmm_minor_version > minor)) ||
        ((gtkmm_major_version == major) && (gtkmm_minor_version == minor) && (gtkmm_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK-- (%d.%d.%d) was found.\n",
               gtkmm_major_version, gtkmm_minor_version, gtkmm_micro_version);
        printf("*** You need a version of GTK-- newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK-- is always available from ftp://ftp.gtk.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gtkmm-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK--, but you can also set the GTKMM_CONFIG environment to point to the\n");
        printf("*** correct copy of gtkmm-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtkmm=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtkmm" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GTKMM_CONFIG" = "no" ; then
       echo "*** The gtkmm-config script installed by GTK-- could not be found"
       echo "*** If GTK-- was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GTKMM_CONFIG environment variable to the"
       echo "*** full path to gtkmm-config."
       echo "*** The gtkmm-config script was not available in GTK-- versions"
       echo "*** prior to 0.9.12. Perhaps you need to update your installed"
       echo "*** version to 0.9.12 or later"
     else
       if test -f conf.gtkmmtest ; then
        :
       else
          echo "*** Could not run GTK-- test program, checking why..."
          CXXFLAGS="$CXXFLAGS $GTKMM_CFLAGS"
          LIBS="$LIBS $GTKMM_LIBS"
          AC_TRY_LINK([
#include <gtk--.h>
#include <stdio.h>
],      [ return ((gtkmm_major_version) || (gtkmm_minor_version) || (gtkmm_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK-- or finding the wrong"
          echo "*** version of GTK--. If it is not finding GTK--, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GTK-- was incorrectly installed"
          echo "*** or that you have moved GTK-- since it was installed. In the latter case, you"
          echo "*** may want to edit the gtkmm-config script: $GTKMM_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTKMM_CFLAGS=""
     GTKMM_LIBS=""
     ifelse([$3], , :, [$3])
     AC_LANG_RESTORE
  fi
  AC_SUBST(GTKMM_CFLAGS)
  AC_SUBST(GTKMM_LIBS)
  rm -f conf.gtkmmtest
])

# Configure paths for GNOME--
# Modified from GTK--.m4

dnl Test for GNOMEMM, and define GNOMEMM_INCLUDEDIR, GNOMEMM_LIBDIR, GNOMEMM_LIBS
dnl   to be used as follows:
dnl AM_PATH_GNOMEMM([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
AC_DEFUN(AM_PATH_GNOMEMM,
[
dnl 
dnl Get the cflags and libraries from the gtkmm-config script
dnl
AC_ARG_WITH(gnomemm-config-path,[  --with-gnomemm-config-path=PREFIX
                          Path to GNOME-- configuration file gnomemmConf.sh (optional)],
            gnomemm_config_prefix="$withval", gnome_config_prefix="")

  AC_MSG_CHECKING(for GNOME-- gnomemmConf.sh)

  if test x$gnomemm_config_prefix != x; then
    if test -r $gnomemm_config_prefix; then
	source $gnomemm_config_prefix
    else 
        no_gnomemm=yes
    fi
  else
    if test -r /usr/lib/gnomemmConf.sh ; then
	source /usr/lib/gnomemmConf.sh
    elif test -r /opt/gnome/lib/gnomemmConf.sh ; then
	source /opt/gnome/lib/gnomemmConf.sh
    elif test -r /usr/local/lib/gnomemmConf.sh ; then
	source /usr/local/lib/gnomemmConf.sh
    else 
        no_gnomemm=yes
    fi
  fi

  if test "x$no_gnomemm" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     AC_MSG_ERROR(Cannot find GNOME-- configuration file gnomemmConf.sh)
  fi

  AC_SUBST(GNOMEMM_INCLUDEDIR)
  AC_SUBST(GNOMEMM_LIBDIR)
  AC_SUBST(GNOMEMM_LIBS)
])

