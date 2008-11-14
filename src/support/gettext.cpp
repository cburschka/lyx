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

#include "support/gettext.h"

#include "support/lstrings.h"
#include "support/Messages.h"

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

using namespace std;

namespace lyx {

docstring const _(string const & str)
{
	return getGuiMessages().get(str);
}


void locale_init()
{
#ifdef ENABLE_NLS
#  ifdef HAVE_LC_MESSAGES
	setlocale(LC_MESSAGES, "");
#  endif
	setlocale(LC_CTYPE, "");
	Messages::init();
#endif
	setlocale(LC_NUMERIC, "C");
}


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
