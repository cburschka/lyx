dnl Usage LYX_PATH_XFORMS: Checks for xforms library and flags
dnl   If it is found, the variable XFORMS_LIB is set to the relevant -l flags,
dnl and FORMS_H_LOCATION / FORMS_IMAGE_H_LOCATION is also set
AC_DEFUN(LYX_PATH_XFORMS,[
 
LIBS="$XPM_LIB $LIBS"
 
AC_CHECK_LIB(forms, fl_initialize, XFORMS_LIB="-lforms",
  [AC_CHECK_LIB(xforms, fl_initialize, XFORMS_LIB="-lxforms",
    [LYX_LIB_ERROR(libforms or libxforms,xforms)])])
AC_SUBST(XFORMS_LIB)
 
### Check for xforms headers
lyx_cv_forms_h_location="<forms.h>"
AC_CHECK_HEADER(X11/forms.h,[
  ac_cv_header_forms_h=yes
  lyx_cv_forms_h_location="<X11/forms.h>"],[
AC_CHECK_HEADER(forms.h,[],[
LYX_LIB_ERROR(forms.h,forms)])])
 
lyx_cv_flimage_h_location="<flimage.h>"
AC_CHECK_HEADER(X11/flimage.h,[
  ac_cv_header_flimage_h=yes
  lyx_cv_flimage_h_location="<X11/flimage.h>"],)
AC_DEFINE_UNQUOTED(FORMS_IMAGE_H_LOCATION,$lyx_cv_flimage_h_location)
 
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
 
XFORMS_VERSION=$lyx_cv_xfversion
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
    1.0*) ;;
       *) LYX_WARNING(dnl
Version $lyx_cv_xfversion of xforms might not be compatible with LyX[,]
 since it is newer than 0.89. You might have slight problems with it.);;
esac
fi])


dnl Check whether the xforms library has a viable image loader
AC_DEFUN(LYX_USE_XFORMS_IMAGE_LOADER,
[
save_LIBS=$LIBS
LIBS="-lflimage $XFORMS_LIB -ljpeg $LIBS"
lyx_use_xforms_image_loader=no
AC_CHECK_FUNCS(flimage_dup,[
  AC_CHECK_FUNCS(flimage_to_pixmap,[
    lyx_use_xforms_image_loader=yes
    AC_CHECK_FUNCS(flimage_enable_ps)])])
LIBS=$save_LIBS
test $lyx_use_xforms_image_loader = yes && lyx_flags="$lyx_flags xforms-image-loader" && XFORMS_IMAGE_LIB=-lflimage
 
# try without flimage 
if test $lyx_use_xforms_image_loader = no ; then
	LIBS="$XFORMS_LIB -ljpeg $LIBS"
	lyx_use_xforms_image_loader=no
	AC_CHECK_FUNCS(flimage_dup,[
	  AC_CHECK_FUNCS(flimage_to_pixmap,[
	    lyx_use_xforms_image_loader=yes
	    AC_CHECK_FUNCS(flimage_enable_ps)])])
	LIBS=$save_LIBS

	# try without -ljpeg
	if test $lyx_use_xforms_image_loader = no ; then
		LIBS="$XFORMS_LIB $LIBS"
		lyx_use_xforms_image_loader=no
		AC_CHECK_FUNCS(flimage_dup,[
		  AC_CHECK_FUNCS(flimage_to_pixmap,[
		    lyx_use_xforms_image_loader=yes
		    AC_CHECK_FUNCS(flimage_enable_ps)])])
		LIBS=$save_LIBS
	fi
 
fi
 
AC_SUBST(XFORMS_IMAGE_LIB)
 
### If the gui cannot load images itself, then we default to the
### very simple one in graphics/GraphicsImageXPM.[Ch]
AM_CONDITIONAL(USE_BASIC_IMAGE_LOADER,
	       test $lyx_use_xforms_image_loader = no)
])


dnl Check if the image loader needs libjpeg
AC_DEFUN(LYX_XFORMS_IMAGE_LOADER_NEEDS_JPEG,
[
    if test "$lyx_use_xforms_image_loader" = "yes" ; then
	AC_MSG_CHECKING([whether libjpeg is needed])
	AC_LANG_SAVE
	AC_LANG_C
	AC_TRY_LINK([#include FORMS_H_LOCATION
#include FORMS_IMAGE_H_LOCATION],
	    [jpeg_abort(0);],
	    [lyx_need_jpeg=no],[lyx_need_jpeg=yes])
	AC_LANG_RESTORE
	AC_MSG_RESULT($lyx_need_jpeg)
	if test "$lyx_need_jpeg" = "yes" ; then
	    LIBS="-ljpeg $LIBS"
	fi
    fi
])
