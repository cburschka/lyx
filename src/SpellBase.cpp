/**
 * \file SpellBase.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "SpellBase.h"
#include "support/gettext.h"
#include "support/docstring.h"

using namespace std;

namespace lyx {


bool SpellBase::alive()
{
	return false;
}


SpellBase::Result SpellBase::check(WordLangTuple const &)
{
	return UNKNOWN_WORD;
}


void SpellBase::insert(WordLangTuple const &)
{}


void SpellBase::accept(WordLangTuple const &)
{}


docstring const SpellBase::nextMiss()
{
	return docstring();
}


docstring const SpellBase::error()
{
	return _("Native OS API not yet supported.");
}


} // namespace lyx
