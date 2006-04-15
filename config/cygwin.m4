# Macro added for some Cygwin-specific support     -*- sh -*-
# @author@: Kayvan Sylvan

AC_DEFUN([CHECK_WITH_CYGWIN],
[
  case $host_os in
  cygwin* )
    for frontend in $FRONTENDS ; do
      case "$frontend" in
      qt* )
	if test "$lyx_use_packaging" = posix; then
	  AC_MSG_CHECKING([window system for Qt])
	  if test "x$with_x" = xno; then
	    CPPFLAGS="${CPPFLAGS} -DQ_CYGWIN_WIN"
	    AC_MSG_RESULT([Windows])
	  else
	    AC_MSG_RESULT([X11])
	  fi
	fi
        ;;
      esac
    done
    ;;
  esac
])
