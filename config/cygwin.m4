# Macro added for some Cygwin-specific support     -*- sh -*-
# @author@: Kayvan Sylvan

AC_DEFUN([CHECK_WITH_CYGWIN],
[
  case $host_os in
  cygwin* | mingw* | pw32* )
    # TODO 1: pass -DQ_CYGWIN_WIN" *when* the option
    # --without-x is passed to configure *and* platform is cygwin
    # TODO 2: see how to pass -mno-cygwin when building native
    # mingw within cygwin
    ;;
  esac
])
