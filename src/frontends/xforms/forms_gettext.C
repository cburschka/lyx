/**
 * \file forms_gettext.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "forms_gettext.h"

#include "support/lstrings.h"

namespace support = lyx::support;


// Extract shortcut from "<identifier>|<shortcut>" string
string const scex(string const & str)
{
	return support::split(str, '|');
}


// Extract identifier from "<identifier>|<shortcut>" string
string const idex(string const & str)
{
	string identifier;
	support::split(str, identifier, '|');
	return identifier;
}
