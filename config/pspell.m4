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

    AC_ARG_WITH(pspell-includes,
	AC_HELP_STRING([--with-pspell-include],[where the pspell.h is located]),
	pspell_use_include="$withval",
	[])

    AC_ARG_WITH(pspell-libs,
	AC_HELP_STRING([--with-pspell-lib],[where the libpspell.a is located]),
	pspell_use_lib="$withval",
	[])

    if test "$USE_PSPELL" = "yes" ; then
	AC_CHECK_HEADER(pspell/pspell.h)
	AC_CHECK_LIB(pspell, delete_pspell_config)

	if test "$pspell_use_include" = "NO" || \
	    test "$pspell_use_lib" = "NO"; then
	    if test "$USE_PSPELL" = "yes"; then
		USE_PSPELL="not found"
	    fi
	fi

	if test "$USE_PSPELL" = "yes"; then
	    AC_DEFINE(USE_PSPELL, 1, [Define as 1 to use the pspell library])
#	PSPELL_INCLUDES="-I$pspell_use_include"
#	PSPELL_LIBS="-L$pspell_use_lib -lpspell"
#	USE_PSPELL="yes ($pspell_use_include $pspell_use_lib)"
	    AC_SUBST(PSPELL_INCLUDES)
	    AC_SUBST(PSPELL_LIBS)
	    lyx_flags="$lyx_flags use-pspell"
	fi
    fi
    AC_MSG_RESULT($USE_PSPELL)
    ])
