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


### Check if we want spell libraries, prefer new aspell or hunspell
AC_DEFUN([LYX_CHECK_SPELL_ENGINES],
[
	CHECK_WITH_ASPELL
	AM_CONDITIONAL(USE_ASPELL, $lyx_use_aspell)

	CHECK_WITH_ENCHANT
	AM_CONDITIONAL(USE_ENCHANT, $lyx_use_enchant)

	CHECK_WITH_HUNSPELL
	AM_CONDITIONAL(USE_HUNSPELL, $lyx_use_hunspell)
	])
