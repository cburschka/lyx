# Macro to add for using aspell spellchecker libraries!     -*- sh -*-
# Only checks for "new" aspell, > 0.50
AC_DEFUN([CHECK_WITH_ASPELL],
[
	lyx_use_aspell=true
	AC_ARG_WITH(aspell, AC_HELP_STRING([--without-aspell],[do not check for ASpell library]))
	test "$with_aspell" = "no" && lyx_use_aspell=false

	if $lyx_use_aspell ; then
	AC_CHECK_HEADERS(aspell.h,
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

# Macro to add for using enchant spellchecker libraries!     -*- sh -*-
AC_DEFUN([CHECK_WITH_ENCHANT],
[
	lyx_use_enchant=true
	AC_ARG_WITH(enchant, AC_HELP_STRING([--without-enchant],[do not check for Enchant library]))
	test "$with_enchant" = "no" && lyx_use_enchant=false

	if $lyx_use_enchant; then
	PKG_CHECK_MODULES([ENCHANT], [enchant], [], [lyx_use_enchant=false])
	AC_MSG_CHECKING([whether to use enchant])
	if $lyx_use_enchant ; then
	    AC_MSG_RESULT(yes)
	    AC_DEFINE(USE_ENCHANT, 1, [Define as 1 to use the enchant library])
	    lyx_flags="$lyx_flags use-enchant"
	else
	    AC_MSG_RESULT(no)
	fi
    fi
    ])

# Macro to add for using hunspell spellchecker libraries!     -*- sh -*-
AC_DEFUN([CHECK_WITH_HUNSPELL],
[
	lyx_use_hunspell=true
	AC_ARG_WITH(hunspell, AC_HELP_STRING([--without-hunspell],[do not check for Hunspell library]))
	test "$with_hunspell" = "no" && lyx_use_hunspell=false

	if $lyx_use_hunspell ; then
	PKG_CHECK_MODULES([HUNSPELL], [hunspell], [], [
		  AC_CHECK_HEADERS(hunspell/hunspell.hxx,
		    [lyx_use_hunspell=true; break;],
			 [lyx_use_hunspell=false])
			 AC_CHECK_LIB(hunspell, main, LIBS="-lhunspell $LIBS", lyx_use_hunspell=false)
		])
	AC_MSG_CHECKING([whether to use hunspell])
	if $lyx_use_hunspell ; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_HUNSPELL, 1, [Define as 1 to use the hunspell library])
		lyx_flags="$lyx_flags use-hunspell"
	else
		AC_MSG_RESULT(no)
	fi
    fi
    ])

dnl Usage: LYX_USE_INCLUDED_HUNSPELL : select if the included hunspell should
dnl        be used.
AC_DEFUN([LYX_USE_INCLUDED_HUNSPELL],[
	AC_MSG_CHECKING([whether to use included hunspell library])
	AC_ARG_WITH(included-hunspell,
		[AC_HELP_STRING([--without-included-hunspell], [do not use the hunspell lib supplied with LyX, try to find one in the system directories - compilation will abort if nothing suitable is found])],
		[lyx_cv_with_included_hunspell=$withval],
		[lyx_cv_with_included_hunspell=no])
	AM_CONDITIONAL(USE_INCLUDED_HUNSPELL, test x$lyx_cv_with_included_hunspell = xyes)
	AC_MSG_RESULT([$lyx_cv_with_included_hunspell])
	if test x$lyx_cv_with_included_hunspell = xyes ; then
		HUNSPELL_CFLAGS='-I$(top_srcdir)/3rdparty/hunspell/1.3.3/src'
		HUNSPELL_LIBS='$(top_builddir)/3rdparty/hunspell/liblyxhunspell.a'
		AC_SUBST(HUNSPELL_CFLAGS)
		AC_SUBST(HUNSPELL_LIBS)
	fi
	])


### Check if we want spell libraries, prefer new aspell or hunspell
AC_DEFUN([LYX_CHECK_SPELL_ENGINES],
[
	LYX_USE_INCLUDED_HUNSPELL
	if test x$lyx_cv_with_included_hunspell = xyes ; then
dnl the user wanted to use the included hunspell, so do not check for the other spell checkers
		lyx_use_aspell=false
		lyx_use_enchant=false
		lyx_use_hunspell=true
		lyx_flags="$lyx_flags use-hunspell"
	else
		CHECK_WITH_ASPELL
		CHECK_WITH_ENCHANT
		CHECK_WITH_HUNSPELL
	fi

	AM_CONDITIONAL(USE_ASPELL, $lyx_use_aspell)
	AM_CONDITIONAL(USE_ENCHANT, $lyx_use_enchant)
	AM_CONDITIONAL(USE_HUNSPELL, $lyx_use_hunspell)
	])

