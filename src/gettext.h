// -*- C++ -*-
#ifndef _GETTEXT_H_
#define _GETTEXT_H_

/*
 * Native Language Support
 *
 * The general idea is that any string that should be translated is handled
 * as follows:
 *	_("string")
 *
 * Static strings are special, obviously and must be flagged as follows:
 *	static str = N_("string");
 *
 * And wherever they are used:
 *	_(str)
 *
 * Every file where there are strings needs:
 *	#include "gettext.h"
 *
 * Remember to mention each of these files in "po/POFILES.in"
 *
 * The main() needs a locale_init() and a gettext_init() in the beginning.
 */

/*
 * General translation notes:
 *   Commands/options are not translated
 *   Debug messages are not translated
 *   Panic/fatal (that should not happen) messages need not be translated
 */

#ifdef ENABLE_NLS

#include "LString.h"

#  if HAVE_GETTEXT
#    include <libintl.h>      // use the header already in the system *EK*
#    ifdef HAVE_LOCALE_H
#      include <locale.h>        // for LC_MESSAGES
#    endif
#  else
#    include "../intl/libintl.h"
#  endif

char const * _(char const *);

string const _(string const &);

//#  define _(str) gettext(str)
#  define N_(str) (str)              // for detecting static strings

#  ifdef HAVE_LC_MESSAGES
                                // LC_TIME, LC_CTYPE, even LC_ALL
#    define locale_init() { setlocale (LC_MESSAGES, ""); setlocale (LC_CTYPE, "");}
#  else
#    define locale_init()
#  endif
#  define gettext_init() { bindtextdomain (PACKAGE, lyx_localedir.c_str()); \
	textdomain (PACKAGE); }
#else
///
#  define _(str) (str)
///
#  define S_(str) (str)
///
#  define N_(str) (str)
///
#  define locale_init()
///
#  define gettext_init()
#endif

#endif
