/**
 * \file ControlThesaurus.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlThesaurus.h"

#include "gettext.h"
#include "lyxfind.h"

#include "frontends/LyXView.h"

ControlThesaurus::ControlThesaurus(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlThesaurus::initialiseParams(string const & data)
{
	oldstr_ = data;
	return true;
}


void ControlThesaurus::clearParams()
{
	oldstr_.erase();
}


void ControlThesaurus::replace(string const & newstr)
{
	/* FIXME: this is not suitable ! We need to have a "lock"
	 * on a particular charpos in a paragraph that is broken on
	 * deletion/change !
	 */
	int const replace_count =
		lyxfind::LyXReplace(kernel().bufferview(), oldstr_, newstr,
				    true, true, true, false, true);

	oldstr_ = newstr;

	if (replace_count == 0)
		kernel().lyxview().message(_("String not found!"));
	else
		kernel().lyxview().message(_("String has been replaced."));
}


Thesaurus::Meanings const &
ControlThesaurus::getMeanings(string const & str)
{
	if (str != laststr_)
		meanings_ = thesaurus.lookup(str);

	return meanings_;
}
