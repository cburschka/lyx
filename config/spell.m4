# Macro to add for using aspell spellchecker libraries!     -*- sh -*-
# Only checks for "new" aspell, > 0.50
AC_DEFUN([CHECK_WITH_ASPELL],
[
    lyx_use_aspell=true
    AC_ARG_WITH(aspell,	AC_HELP_STRING([--with-aspell],[use ASpell libraries]))
    test "$with_aspell" = "no" && lyx_use_aspell=false

    if $lyx_use_aspell ; then
	AC_CHECK_HEADERS(aspell.h aspell/aspell.h, 
	    [lyx_use_aspell=true; break;], 
	    [lyx_use_aspell=false])
	AC_CHECK_LIB(aspell, new_aspell_config, LIBS="-laspell $LIBS", lyx_use_aspell=false)

	AC_MSG_CHECKING([whether to use aspell])
	if $lyx_use_aspell ; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(USE_ASPELL, 1, [Define as 1 to use the aspell library])
	    lyx_flags="$lyx_flags use-aspell"
	else
	    AC_MSG_RESULT(no)
	fi
    fi
    ])


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

### Check if we want spell libraries, prefer new aspell
AC_DEFUN([LYX_CHECK_SPELL_ENGINES],
[
    lyx_use_aspell=false
    lyx_use_pspell=false
    lyx_use_ispell=false

    dnl Prefer use of the aspell library over pspell.
    CHECK_WITH_ASPELL
    if $lyx_use_aspell ; then : ; else
        CHECK_WITH_PSPELL
    fi

    dnl check for the ability to communicate through unix pipes
    dnl with an external ispell process.
    dnl Do this independent of the existence of the aspell, pspell libraries.
    lyx_use_ispell=true
    AC_LANG_PUSH(C)
    AC_CHECK_FUNCS(select dup2,, [lyx_use_ispell=false])
    AC_LANG_POP(C)
    AC_MSG_CHECKING([whether to use ispell])
    if $lyx_use_ispell ; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(USE_ISPELL, 1, [Define as 1 to use an external ispell process for spell-checking])
      lyx_flags="$lyx_flags use-ispell"
    else
      AC_MSG_RESULT(no)
    fi

    AM_CONDITIONAL(USE_ASPELL, $lyx_use_aspell)
    AM_CONDITIONAL(USE_PSPELL, $lyx_use_pspell)
    AM_CONDITIONAL(USE_ISPELL, $lyx_use_ispell)
    ])
