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

///
char const * _(char const *);
///
string const _(string const &);

#else // ENABLE_NLS

///
#  define _(str) (str)
///
#  define S_(str) (str)

#endif

#  define N_(str) (str)              // for detecting static strings

///
void locale_init();
///
void gettext_init(string const & localedir);

#endif
