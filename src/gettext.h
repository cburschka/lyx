// -*- C++ -*-
/**
 * \file gettext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GETTEXT_H
#define GETTEXT_H

#include <string>

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


//#ifdef ENABLE_NLS

///
std::string const _(std::string const &);

//#else // ENABLE_NLS

///
//#  define _(str) (str)

//#endif

#  define N_(str) (str)              // for detecting static strings

///
void locale_init();

#endif
