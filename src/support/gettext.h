// -*- C++ -*-
/**
 * \file src/support/gettext.h
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

#include "support/strfwd.h"


namespace lyx {

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
 *	#include "support/gettext.h"
 *
 * Remember to mention each of these files in "po/POFILES.in"
 *
 * The main() needs a locale_init() and a gettext_init() in the beginning.
 *
 * The various *_() methods accept only ASCII input, so they must not be used
 * if the input may come from user supplied  files.
 * translateIfPossible() should be used in that case.
 */

/*
 * General translation notes:
 *   Commands/options are not translated
 *   Debug messages are not translated
 *   Panic/fatal (that should not happen) messages need not be translated
 */


//#ifdef ENABLE_NLS

///
docstring const _(std::string const &);

//#else // ENABLE_NLS

///
//#  define _(str) (str)

//#endif

#  define N_(str) (str)              // for detecting static strings

/**
 * Translate \p name to the GUI language if it is possible.
 * This should be used to translate strings that come from configuration
 * files like .ui files. These strings could already be in the native
 * language if they come from a file in the personal directory. */
docstring const translateIfPossible(docstring const & name);
/**
 * Translate \p name to \p language if it is possible.
 * This should be used to translate strings that come from configuration
 * files like .ui files. These strings could already be in the native
 * language if they come from a file in the personal directory. */
docstring const translateIfPossible(docstring const & name, std::string const & language);

} // namespace lyx

#endif
