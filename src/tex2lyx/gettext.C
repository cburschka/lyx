/**
 * \file gettext.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "gettext.h"

string const _(string const & str)
{
	return str;
}


void locale_init()
{}
