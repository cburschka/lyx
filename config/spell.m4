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

AC_DEFUN([LYX_HAVE_ENCHANT2],
[
  AC_MSG_CHECKING([whether enchant is version 2.x at least])
  save_CXXFLAGS=$CXXFLAGS
  CXXFLAGS="$ENCHANT_CFLAGS $AM_CXXFLAGS $CXXFLAGS"

  AC_TRY_COMPILE([#include <enchant++.h>],
      [enchant::Broker broker;],
      [AC_MSG_RESULT(yes)
       AC_DEFINE(HAVE_ENCHANT2, 1, [Define to 1 if enchant 2.x is detected])
      ],
      [AC_MSG_RESULT(no)])
  CXXFLAGS=$save_CXXFLAGS
])

AC_DEFUN([CHECK_WITH_ENCHANT],
[
	lyx_use_enchant=true
	AC_ARG_WITH(enchant, AC_HELP_STRING([--without-enchant],[do not check for Enchant library]))
	test "$with_enchant" = "no" && lyx_use_enchant=false

	if $lyx_use_enchant; then
		PKG_CHECK_MODULES([ENCHANT], [enchant-2], [],
		    [PKG_CHECK_MODULES([ENCHANT], [enchant], [],
		        [lyx_use_enchant=false])])
		AC_MSG_CHECKING([whether to use enchant])
		if $lyx_use_enchant ; then
		    AC_MSG_RESULT(yes)
		    AC_DEFINE(USE_ENCHANT, 1, [Define as 1 to use the enchant library])
		    LYX_HAVE_ENCHANT2
		    lyx_flags="$lyx_flags use-enchant"
		else
		    AC_MSG_RESULT(no)
		fi
    	fi
    ])

AC_DEFUN([LYX_HAVE_HUNSPELL_CXXABI],
[
  AC_MSG_CHECKING([whether hunspell C++ (rather than C) ABI is provided])
  save_CXXFLAGS=$CXXFLAGS
  CXXFLAGS="$ENCHANT_CFLAGS $AM_CXXFLAGS $CXXFLAGS"

# in the C++ ABI, stem() returns a vector, in the C ABI, it returns an int
  AC_TRY_COMPILE([#include <hunspell/hunspell.hxx>],
      [Hunspell sp("foo", "bar");
       int i = sp.stem("test").size();],
      [AC_MSG_RESULT(yes)
       AC_DEFINE(HAVE_HUNSPELL_CXXABI, 1, [Define to 1 if hunspell C++ (rather than C) ABI is detected])
       have_hunspell_cxx_abi=yes
      ],
      [AC_MSG_RESULT(no)])
  CXXFLAGS=$save_CXXFLAGS
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
	fi
	AC_MSG_CHECKING([whether to use hunspell])
	if $lyx_use_hunspell ; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_HUNSPELL, 1, [Define as 1 to use the hunspell library])
		lyx_flags="$lyx_flags use-hunspell"
		LYX_HAVE_HUNSPELL_CXXABI
		if test $enable_stdlib_debug = "yes" -a -n "$have_hunspell_cxx_abi" ; then
		    LYX_WARNING([Compiling LyX with stdlib-debug and system hunspell libraries may lead to
   crashes. Consider using --disable-stdlib-debug or --with-included-hunspell.])
		fi
	else
		AC_MSG_RESULT(no)
	fi
    ])

dnl Usage: LYX_USE_INCLUDED_HUNSPELL : select if the included hunspell should
dnl        be used.
AC_DEFUN([LYX_USE_INCLUDED_HUNSPELL],[
	AC_MSG_CHECKING([whether to use included hunspell library])
	AC_ARG_WITH(included-hunspell,
		[AC_HELP_STRING([--with-included-hunspell], [use the hunspell lib supplied with LyX instead of the system one])],
		[lyx_cv_with_included_hunspell=$withval],
		[lyx_cv_with_included_hunspell=no])
	AM_CONDITIONAL(USE_INCLUDED_HUNSPELL, test x$lyx_cv_with_included_hunspell = xyes)
	AC_MSG_RESULT([$lyx_cv_with_included_hunspell])
	if test x$lyx_cv_with_included_hunspell = xyes ; then
		lyx_included_libs="$lyx_included_libs hunspell"
		HUNSPELL_CFLAGS='-I$(top_srcdir)/3rdparty/hunspell/1.7.0/src'
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
dnl the user wanted to use the included hunspell, so do not check for external hunspell
		lyx_use_hunspell=true
		AC_DEFINE(USE_HUNSPELL, 1, [Define as 1 to use the hunspell library])
		AC_DEFINE(HAVE_HUNSPELL_CXXABI, 1, [Define to 1 if hunspell C++ (rather than C) ABI is detected])
		lyx_flags="$lyx_flags use-hunspell"
	else
		CHECK_WITH_HUNSPELL
	fi
	CHECK_WITH_ASPELL
	CHECK_WITH_ENCHANT

	AM_CONDITIONAL(USE_ASPELL, $lyx_use_aspell)
	AM_CONDITIONAL(USE_ENCHANT, $lyx_use_enchant)
	AM_CONDITIONAL(USE_HUNSPELL, $lyx_use_hunspell)
	])

