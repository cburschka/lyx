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

