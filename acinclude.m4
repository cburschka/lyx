dnl Some useful functions for LyXs configure.in                 -*- sh -*-
dnl Author: Jean-Marc Lasgouttes (Jean-Marc.Lasgouttes@inria.fr)
dnl         Lars Gullik Bjønnes (larsbj@lyx.org)


dnl Usage LYX_GET_VERSION   Sets "lyx_version" to the version of LyX being 
dnl   built, displays it and sets variable "lyx_devel" to yes or no depending 
dnl   whether the version is a development release or not. 
AC_DEFUN(LYX_GET_VERSION,[
changequote(, ) dnl
# Get LyX version from $1 and guess wether is is stable or experimental.
VERSION=`grep '#define *LYX_VERSION' $1 | 
             sed -e 's/^.*"\([0-9.]*\)[^0-9.].*$/\1/' 2>/dev/null`
echo "configuring LyX version $VERSION"
if echo "$VERSION" | grep '[0-9]*\.[0-9]*[13579]\.[0-9]*' >/dev/null ; then
  lyx_devel_version=yes
  echo "WARNING: This is a development version. Expect bugs."
else
  lyx_devel_version=no
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
    IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
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
  [  --with-warnings         tell GNU C++ to display more warnings],,
  [if test $lyx_devel_version = yes && test $ac_cv_prog_gxx = yes ; then
     with_warnings=yes;
   else
     with_warnings=no;
   fi;])
if test "x$with_warnings" = xyes ; then
  lyx_flags="$lyx_flags warnings"
  AC_DEFINE(WITH_WARNINGS, 1,
  [Define this if you want to see the warning directives put here and
   there by the developpers to get attention])
fi

# optimize less for development versions
if test "$lyx_devel_version" = yes ; then
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
  gxx_version=`g++ --version`
  
  AC_PROG_CXX_G
  if test "$ac_test_CXXFLAGS" = set; then
    CXXFLAGS="$ac_save_CXXFLAGS"
  elif test $ac_cv_prog_cxx_g = yes; then
    case $gxx_version in
      2.7*) CXXFLAGS="$lyx_opt";;
      *)    CXXFLAGS="-g $lyx_opt -fno-exceptions -fno-rtti";;
    esac
  else
    CXXFLAGS="$lyx_opt"
  fi
  if test $with_warnings = yes ; then
    CXXFLAGS="$CXXFLAGS -ansi -Wall"
    # a small hack to avoid problems with headers
    echo '#undef __STRICT_ANSI__' >>confdefs.h
  fi
else
  GXX=
  test "${CXXFLAGS+set}" = set || CXXFLAGS="-g"
fi
])dnl

dnl We do not use this one anymore.
dnl AC_DEFUN(LYX_GXX_STRENGTH_REDUCE,[
dnl #check for the strength reduction bug of gcc
dnl if test x$GXX = xyes && test $cross_compiling = no ; then
dnl   AC_CACHE_CHECK( "for gcc strength-reduce bug", ac_cv_c_gcc_strength_bug,
dnl                   AC_TRY_RUN([
dnl int main(void) {
dnl   static int Array[[3]];
dnl   unsigned int B = 3;
dnl   int i;
dnl   for(i=0; i<B; i++) Array[[i]] = i - 3;
dnl   exit( Array[[1]] != -2 );
dnl }],
dnl     ac_cv_c_gcc_strength_bug="no",
dnl     ac_cv_c_gcc_strength_bug="yes",
dnl     ac_cv_c_gcc_strength_bug="yes") )
dnl   if test "$ac_cv_c_gcc_strength_bug" = "yes"
dnl   then
dnl     CXXFLAGS="$CXXFLAGS -fno-strength-reduce"
dnl   fi
dnl fi
dnl ])dnl

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


dnl Usage: LYX_CXX_STL_STRING : checks whether the C++ compiler
dnl   has a working stl string container
AC_DEFUN(LYX_CXX_STL_STRING,[
AC_ARG_WITH(included-string,
[  --with-included-string  use LyX string class instead of STL string],,
[AC_CHECK_HEADER(std/bastring.h,
  [with_included_string=yes],
  [with_included_string=no])])
if test x$with_included_string = xyes ; then
  AC_DEFINE(USE_INCLUDED_STRING, 1, 
    [Define to use the LString class bundled with LyX])
  lyx_flags="$lyx_flags included-string"
fi])


dnl LYX_CXX_MUTABLE
AC_DEFUN(LYX_CXX_MUTABLE, [
AC_REQUIRE([LYX_PROG_CXX])
AC_MSG_CHECKING(if C++ compiler supports mutable)
AC_TRY_COMPILE(
[
class k {       
        mutable char *c;
public:
   void foo() const { c=0; }
};
],[
],[
  ac_mutable=yes
  AC_DEFINE(HAVE_MUTABLE, 1, 
   [Defined if you compiler supports 'mutable'.])
],[
  ac_mutable=no
]) 
AC_MSG_RESULT([$ac_mutable])
])


dnl LYX_CXX_PARTIAL
AC_DEFUN(LYX_CXX_PARTIAL, [
AC_REQUIRE([LYX_PROG_CXX])
AC_MSG_CHECKING(if C++ compiler supports partial specialization)
AC_TRY_COMPILE(
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
  ac_partial_specialization=yes
  AC_DEFINE(HAVE_PARTIAL_SPECIALIZATION, 1, 
   [Defined if your compiler supports partial specialization.])
],[
  ac_partial_specialization=no
]) 
AC_MSG_RESULT([$ac_partial_specialization])
])

dnl Usage: LYX_CXX_NAMESPACES : checks whether the C++ compiler
dnl   has a correct namespace handling and define CXX_WORKING_NAMESPACES 
dnl   if true. This macro does not do a thourough test, but it should be 
dnl   good enough to suit our needs.
AC_DEFUN(LYX_CXX_NAMESPACES,[
AC_CACHE_CHECK(for correct namespaces support,lyx_cv_cxx_namespace,
 [AC_TRY_COMPILE([
  #include <vector>
  using std::vector;
],[
	vector<int> test;
	return 0;
],lyx_cv_cxx_namespace=yes,lyx_cv_cxx_namespace=no)
])
if test $lyx_cv_cxx_namespace = yes ; then
  AC_DEFINE(CXX_WORKING_NAMESPACES, 1, 
   [Define if your C++ compiler has correct support for namespaces])
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
  "(unknown)"|0.82|0.83|0.84|0.85) 
     LYX_ERROR(dnl
Version $lyx_cv_xfversion of xforms is not compatible with LyX. 
   This version of LyX works best with version 0.88[,] although it
   supports also versions 0.81[,] 0.86 and 0.87.) ;;
  0.81|0.86|0.87) 
     LYX_WARNING(dnl
While LyX is compatible with version $lyx_cv_xfversion of xforms[,] 
   it is recommended that you upgrade to version 0.88.) ;;
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


dnl Usage: LYX_WITH_DIR(dir-name,desc,dir-var-name,default-value, 
dnl                       [default-yes-value])  
dnl  Adds a --with-'dir-name' option (described by 'desc') and puts the 
dnl  resulting directory name in 'dir-var-name'.
AC_DEFUN(LYX_WITH_DIR,[
  AC_ARG_WITH($1,[  --with-$1     specify $2])
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
