# Macro to add for using pspell spellchecker libraries!     -*- sh -*-
# @author@: Jürgen Vigna

AC_DEFUN(CHECK_WITH_PSPELL,
[
    AC_MSG_CHECKING([for pspell support])
    AC_ARG_WITH(pspell,
	AC_HELP_STRING([--with-pspell],[use PSpell libraries]),
	[
	    case "$withval" in
		y*) USE_PSPELL="yes";;
		*) USE_PSPELL="no";;
	    esac
	    ],
	[
	    USE_PSPELL="yes"
	    ])

    if test "$USE_PSPELL" = "yes" ; then
	AC_CHECK_HEADERS(pspell/pspell.h, break, USE_PSPELL=no)
	AC_CHECK_LIB(pspell, new_pspell_config)

	if test "$USE_PSPELL" = "yes"; then
	    AC_DEFINE(USE_PSPELL, 1, [Define as 1 to use the pspell library])
	    lyx_flags="$lyx_flags use-pspell"
	fi
    fi
    AC_MSG_RESULT($USE_PSPELL)
    ])
