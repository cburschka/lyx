# Macro to add for using aspell spellchecker libraries!     -*- sh -*-

# Only checks for "new" aspell, > 0.50

AC_DEFUN(CHECK_WITH_ASPELL,
[
    AC_ARG_WITH(aspell,
	AC_HELP_STRING([--with-aspell],[use ASpell libraries]),
	[
	    case "$withval" in
		y*) USING_ASPELL="yes";;
		*) USING_ASPELL="no";;
	    esac
	    ],
	[
	    USING_ASPELL="yes"
	    ])

    if test "$USING_ASPELL" = "yes" ; then
	AC_CHECK_HEADERS(aspell.h aspell/aspell.h, USING_ASPELL="yes", USING_ASPELL="no")
	AC_CHECK_LIB(aspell, new_aspell_config, LIBS="-laspell $LIBS"; USING_ASPELL="yes", USING_ASPELL="no")

	if test "$USING_ASPELL" = "yes"; then
	    AC_DEFINE(USE_ASPELL, 1, [Define as 1 to use the aspell library])
	    lyx_flags="$lyx_flags use-aspell"
	fi
    fi
    AC_MSG_CHECKING([whether to use aspell])
    AC_MSG_RESULT($USING_ASPELL)
    ])
