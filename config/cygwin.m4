# Macro added for some Cygwin-specific support     -*- sh -*-
# @author@: Kayvan Sylvan

AC_DEFUN([CHECK_WITH_CYGWIN],
[
  case $host_os in
  cygwin* | mingw* | pw32* )
    if test "$GCC" = yes; then
      # Ensure MSVC-compatible struct packing convention.
      # Depends on GCC version. gcc2 uses -fnative-struct while
      # gcc3 uses -mms-bitfields.
      #
      msnative_struct=''
      AC_MSG_CHECKING([how to get MSVC-compatible struct packing])
      case `$CC --version | sed -e 's,\..*,.,' -e q` in
      2.)
        if $CC -v --help 2>/dev/null | grep fnative-struct > /dev/null; then
        msnative_struct='-fnative-struct'
        fi
        ;;
      *)
        if $CC -v --help 2>/dev/null | grep ms-bitfields > /dev/null; then
        msnative_struct='-mms-bitfields'
        fi
        ;;
      esac

      if test x"$msnative_struct" = x; then
        AC_MSG_RESULT([no way])
        AC_MSG_WARN([produced libraries might be incompatible with MSVC libs])
      else
        CXXFLAGS="$CXXFLAGS $msnative_struct"
        AC_MSG_RESULT([${msnative_struct}])
      fi
    fi

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
