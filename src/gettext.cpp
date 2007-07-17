/**
 * \file src/gettext.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "gettext.h"
#include "Messages.h"

#include "support/environment.h"
#include "support/lstrings.h"

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

using std::string;


namespace lyx {

using support::setEnv;


docstring const _(string const & str)
{
	return getGuiMessages().get(str);
}


#ifdef ENABLE_NLS

void locale_init()
{
	// Disable, as otherwise it overrides everything else incl. the doc language
	setEnv("LANGUAGE", "");
#  ifdef HAVE_LC_MESSAGES
	setlocale(LC_MESSAGES, "");
#  endif
	setlocale(LC_CTYPE, "");
	setlocale(LC_NUMERIC, "C");
}

#else // ENABLE_NLS

void locale_init()
{
	setlocale(LC_NUMERIC, "C");
}

#endif


docstring const translateIfPossible(docstring const & name)
{
	if (support::isAscii(name))
		// Probably from a standard configuration file, try to
		// translate
		return _(to_ascii(name));
	else
		// This must be from a user defined configuration file. We
		// cannot translate this, since gettext accepts only ascii
		// keys.
		return name;
}


} // namespace lyx
