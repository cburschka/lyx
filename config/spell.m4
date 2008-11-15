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


### Check if we want spell libraries, prefer new aspell
AC_DEFUN([LYX_CHECK_SPELL_ENGINES],
[
    lyx_use_aspell=false
    CHECK_WITH_ASPELL

    dnl check for the ability to communicate through unix pipes
    dnl with an external ispell process.
    dnl Do this independent of the existence of the aspell libraries.
    lyx_use_ispell=true

    AM_CONDITIONAL(USE_ASPELL, $lyx_use_aspell)
    ])
