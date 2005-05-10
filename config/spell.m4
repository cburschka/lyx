# Macro to add for using pspell spellchecker libraries!     -*- sh -*-
# @author@: Jürgen Vigna
AC_DEFUN([CHECK_WITH_PSPELL],
[
    lyx_use_pspell=true
    AC_ARG_WITH(pspell,	AC_HELP_STRING([--with-pspell],[use PSpell libraries]))
    test "$with_pspell" = "no" && lyx_use_pspell=false

    if $lyx_use_pspell ; then
	AC_CHECK_HEADERS(pspell/pspell.h,, lyx_use_pspell=false)
	AC_CHECK_LIB(pspell, main, LIBS="-lpspell $LIBS", lyx_use_pspell=false)

	AC_MSG_CHECKING([whether to use pspell])
	if $lyx_use_pspell ; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(USE_PSPELL, 1, [Define as 1 to use the pspell library])
	    lyx_flags="$lyx_flags use-pspell"
	else
	    AC_MSG_RESULT(no)
	fi
    fi
    ])

### Check if we want spell libraries, prefer pspell
AC_DEFUN([LYX_CHECK_SPELL_ENGINES],
[
    lyx_use_pspell=false
    lyx_use_ispell=false

    CHECK_WITH_PSPELL

    dnl check for the ability to communicate through unix pipes
    dnl with an external ispell process.
    dnl Do this independent of the existence of the pspell library.
    lyx_use_ispell=true
    AC_LANG_PUSH(C)
    AC_CHECK_FUNCS(fork,, [lyx_use_ispell=false])
    AC_LANG_POP(C)
    AC_MSG_CHECKING([whether to use ispell])
    if $lyx_use_ispell ; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(USE_ISPELL, 1, [Define as 1 to use an external ispell process for spell-checking])
      lyx_flags="$lyx_flags use-ispell"
    else
      AC_MSG_RESULT(no)
    fi

    AM_CONDITIONAL(USE_PSPELL, $lyx_use_pspell)
    AM_CONDITIONAL(USE_ISPELL, $lyx_use_ispell)
    ])
