# Macro to add for using pspell spellchecker libraries!
# @author@: Jürgen Vigna

AC_DEFUN(CHECK_WITH_PSPELL,
[
  USE_PSPELL="no"
  AC_MSG_CHECKING([for pspell support])
    AC_ARG_WITH(pspell,
      [  --with-pspell           use PSpell libraries],[
        if test "$withval" != "no"; then
          USE_PSPELL="yes"
        fi
      ])

    AC_ARG_WITH(pspell-includes, [  --with-pspell-include   where the pspell.h is located],
      pspell_use_include="$withval",
      pspell_use_include=NONE)

    if test "$pspell_use_include" = "" || \
      test "$pspell_use_include" = "NONE"; then
      pspell_includes="/usr/include /usr/local/include"
      AC_FIND_FILE(pspell/pspell.h,$pspell_includes,pspell_use_include)
    fi

    AC_ARG_WITH(pspell-libs, [  --with-pspell-lib       where the libpspell.a is located],
      pspell_use_lib="$withval",
      pspell_use_lib=NONE)

    if test -n "$pspell_use_lib" && \
      test "$pspell_use_lib" != "NONE"; then
      pspell_lib_found="Setting to $pspell_use_lib"
    else
      pspell_libs="/usr/lib /usr/lib/pspell /usr/local/lib /usr/local/lib/pspell"
      AC_FIND_FILE(libpspell.a,$pspell_libs,pspell_use_lib)
      pspell_lib_found="yes"
    fi

    if test "$pspell_use_include" = "NO" || \
       test "$pspell_use_include" = "NO"; then
      if test "$USE_PSPELL" = "yes"; then
	USE_PSPELL="not found"
      fi
    fi

    if test "$USE_PSPELL" = "yes"; then
      AC_DEFINE(USE_PSPELL)
      PSPELL_INCLUDES="-I$pspell_use_include"
      PSPELL_LIBS="-L$pspell_use_lib -lpspell"
      USE_PSPELL="yes ($pspell_use_include $pspell_use_lib)"
      AC_SUBST(PSPELL_INCLUDES)
      AC_SUBST(PSPELL_LIBS)
      lyx_flags="$lyx_flags use-pspell"
    fi
    AC_MSG_RESULT($USE_PSPELL)
  ])
