/**
 * \file tex2lyx/gettext.cpp
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
#include "../support/docstring.h"

using namespace std;

namespace lyx {


docstring const _(string const & str)
{
	return from_ascii(str);
}


void locale_init()
{}


} // namespace lyx
