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
#include "support/Package.h"

using namespace std;

namespace lyx {

docstring const _(string const & str)
{
	return getGuiMessages().get(str);
}


docstring const translateIfPossible(docstring const & name)
{
	if (support::isAscii(name) && !name.empty())
		// Probably from a standard configuration file, try to
		// translate
		return _(to_ascii(name));
	else
		// This must be from a user defined configuration file. We
		// cannot translate this, since gettext accepts only ascii
		// keys.
		return name;
}


docstring const translateIfPossible(docstring const & name, std::string const & language)
{
	if (support::isAscii(name) && !name.empty())
		// Probably from a standard configuration file, try to
		// translate
		return getMessages(language).get(to_ascii(name));
	else
		// This must be from a user defined configuration file. We
		// cannot translate this, since gettext accepts only ascii
		// keys.
		return name;
}


} // namespace lyx
