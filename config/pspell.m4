# Macro to add for using pspell spellchecker libraries!     -*- sh -*-
# @author@: Jürgen Vigna

AC_DEFUN(CHECK_WITH_PSPELL,
[
    AC_ARG_WITH(pspell,
	AC_HELP_STRING([--with-pspell],[use PSpell libraries]),
	[
	    case "$withval" in
		y*) TRY_PSPELL="yes";;
		*) TRY_PSPELL="no";;
	    esac
	    ],
	[
	    TRY_PSPELL="yes"
	    ])

    if test "$TRY_PSPELL" = "yes" ; then
	AC_CHECK_HEADERS(pspell.h pspell/pspell.h, break, USE_PSPELL=no)
	AC_CHECK_LIB(pspell, main)

	if test "$TRY_PSPELL" = "yes"; then
	    AC_DEFINE(USE_PSPELL, 1, [Define as 1 to use the pspell library])
	    lyx_flags="$lyx_flags use-pspell"
	fi
    fi
    AC_MSG_CHECKING([whether to use pspell])
    AC_MSG_RESULT($TRY_PSPELL)
    ])
