# Macro to add for using pspell spellchecker libraries!     -*- sh -*-
# @author@: Jürgen Vigna

AC_DEFUN(CHECK_WITH_PSPELL,
[
    AC_ARG_WITH(pspell,
	AC_HELP_STRING([--with-pspell],[use PSpell libraries]),
	[
	    case "$withval" in
		y*) USING_PSPELL="yes";;
		*) USING_PSPELL="no";;
	    esac
	    ],
	[
	    USING_PSPELL="yes"
	    ])

    if test "$USING_PSPELL" = "yes" ; then
	AC_CHECK_HEADERS(pspell/pspell.h, USING_PSPELL="yes", USING_PSPELL="no")
	AC_CHECK_LIB(pspell, main, LIBS="-lpspell $LIBS"; USING_PSPELL="yes", USING_PSPELL="no")

	if test "$USING_PSPELL" = "yes"; then
	    AC_DEFINE(USE_PSPELL, 1, [Define as 1 to use the pspell library])
	    lyx_flags="$lyx_flags use-pspell"
	fi
    fi
    AC_MSG_CHECKING([whether to use pspell])
    AC_MSG_RESULT($USING_PSPELL)
    ])
