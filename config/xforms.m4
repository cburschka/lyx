dnl some macros to test for xforms-related functionality  -*- sh -*-

dnl Usage LYX_PATH_XPM: Checks for xpm library and header
AC_DEFUN(LYX_PATH_XPM,[
### Check for Xpm library
AC_CHECK_LIB(Xpm, XpmCreateBufferFromImage, XPM_LIB="-lXpm",
	[LYX_LIB_ERROR(libXpm,Xpm)])
### Check for Xpm headers
lyx_cv_xpm_h_location="<xpm.h>"
AC_CHECK_HEADER(X11/xpm.h,[
  ac_cv_header_xpm_h=yes
  lyx_cv_xpm_h_location="<X11/xpm.h>"],[
AC_CHECK_HEADER(xpm.h,[],[
LYX_LIB_ERROR(xpm.h,Xpm)])])
AC_SUBST(XPM_H_LOCATION,$lyx_cv_xpm_h_location)
### Test for the header version
if test $ac_cv_header_xpm_h = yes; then
  AC_CACHE_CHECK([xpm header version],lyx_cv_xpmversion,
  [ cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"

#include $lyx_cv_xpm_h_location
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


dnl Usage LYX_PATH_XFORMS: Checks for xforms library and flags
dnl   If it is found, the variable XFORMS_LIB is set to the relevant -l flags,
dnl and FORMS_H_LOCATION / FLIMAGE_H_LOCATION is also set
AC_DEFUN(LYX_PATH_XFORMS,[
AC_REQUIRE([LYX_PATH_XPM])

AC_CHECK_LIB(forms, fl_initialize, XFORMS_LIB="-lforms",
  [AC_CHECK_LIB(xforms, fl_initialize, XFORMS_LIB="-lxforms",
    [LYX_LIB_ERROR(libforms or libxforms,xforms)], $XPM_LIB)], $XPM_LIB)

### Check for xforms headers
lyx_cv_forms_h_location="<forms.h>"
AC_CHECK_HEADER(X11/forms.h,[
  ac_cv_header_forms_h=yes
  lyx_cv_forms_h_location="<X11/forms.h>"],[
AC_CHECK_HEADER(forms.h,[],[
LYX_LIB_ERROR(forms.h,forms)])])
AC_SUBST(FORMS_H_LOCATION,$lyx_cv_forms_h_location)

if test $ac_cv_header_forms_h = yes; then
  AC_CACHE_CHECK([xforms header version],lyx_cv_xfversion,
  [ cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"

#include $lyx_cv_forms_h_location
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

XFORMS_VERSION=$lyx_cv_xfversion
case "$lyx_cv_xfversion" in
"(unknown)"|0.8[[1-8]]*|0.89[[01234]])
	LYX_ERROR(dnl
Version $lyx_cv_xfversion of xforms is not compatible with LyX.
   This version of LyX works best with version 1.0 (recommended) and later.) ;;
0.89*|0.9999*)
	LYX_WARNING(dnl
LyX should work ok with version $lyx_cv_xfversion of xforms[,] but
   it contains many bugs that have been squashed in version 1.0.
   You should consider upgrading.) ;;
1.0*) ;;
esac
fi
])


dnl Check the details of the xforms image loader
AC_DEFUN([LYX_CHECK_XFORMS_IMAGE_LOADER],
[AC_REQUIRE([LYX_PATH_XFORMS])
save_LIBS=$LIBS
LIBS="$XFORMS_LIB $XPM_LIB $LIBS"
lyx_use_jpeg_image_loader=no

AC_LANG_SAVE
AC_LANG_C

dnl The image headers were split out of forms.h in xforms version 1.0.
AC_CHECK_HEADERS(flimage.h X11/flimage.h, break,,[#include $lyx_cv_forms_h_location])

dnl Only enable native loading of jpeg images if the jpeg library is installed.
dnl This test comes before that of flimage itself to ensure that the necessary
dnl libraries are linked into the "flimage_dup" test program below.
XFORMS_IMAGE_LIB=""
AC_CHECK_LIB(jpeg, jpeg_read_header,
  [lyx_use_jpeg_image_loader=yes
   XFORMS_IMAGE_LIB="-ljpeg"
   LIBS="-ljpeg $LIBS"])

AC_SEARCH_LIBS(flimage_dup, flimage,
  [if test "$ac_cv_search_flimage_dup" != "none required" ; then
     XFORMS_IMAGE_LIB="-lflimage $XFORMS_IMAGE_LIB"
  fi])

if test $lyx_use_jpeg_image_loader = yes ; then
  lyx_flags="$lyx_flags xforms-image-loader"
  AC_DEFINE(USE_JPEG_IMAGE_LOADER, 1,
	    [Define if you want to be able to load jpeg images natively])
fi

AC_LANG_RESTORE
LIBS=$save_LIBS])

dnl Do all check required to use xforms
AC_DEFUN([XFORMS_DO_IT_ALL],
[LYX_PATH_XPM
LYX_PATH_XFORMS
LYX_CHECK_XFORMS_IMAGE_LOADER
AC_SUBST(XFORMS_LIBS, ["$XFORMS_IMAGE_LIB $XFORMS_LIB $XPM_LIB"])])
