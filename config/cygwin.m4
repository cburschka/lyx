# Macro added for some Cygwin-specific support     -*- sh -*-
# @author@: Kayvan Sylvan

AC_DEFUN([CHECK_WITH_CYGWIN],
[
  case $host_os in
  cygwin* | mingw* | pw32* )
    # Export all symbols to Win32 DLL using MinGW 2.0 ld.
    WIN32_LD_EXPORT_ALL_SYMBOLS=''
    AC_MSG_CHECKING([whether ld accepts --export-all-symbols])
    if $LD --help 2>&1 | egrep 'export-all-symbols' > /dev/null; then
      WIN32_LD_EXPORT_ALL_SYMBOLS='-Wl,--export-all-symbols'
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
    fi
    if test x"$WIN32_LD_EXPORT_ALL_SYMBOLS" != x; then
      LDFLAGS="$LDFLAGS $WIN32_LD_EXPORT_ALL_SYMBOLS"
    fi
    ;;
  esac
])
