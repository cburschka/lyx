/**
 * \file SpellBase.C
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
#include "gettext.h"

using std::string;


bool SpellBase::alive()
{
	return false;
}


SpellBase::Result SpellBase::check(WordLangTuple const &)
{
	return UNKNOWN;
}


void SpellBase::insert(WordLangTuple const &)
{}


void SpellBase::accept(WordLangTuple const &)
{}


string const SpellBase::nextMiss()
{
	return string();
}


string const SpellBase::error()
{
	return _("Native OS API not yet supported.");
}
