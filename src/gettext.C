/**
 * \file src/gettext.C
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
#include "messages.h"
#include "support/environment.h"

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

using std::string;
using lyx::support::setEnv;


namespace {

Messages & getLyXMessages()
{
	static Messages lyx_messages;

	return lyx_messages;
}

} // anon namespace


string const _(string const & str)
{
	return getLyXMessages().get(str);
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
