/**
 * \file ControlThesaurus.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlThesaurus.h"

#include "gettext.h"
#include "lyxfind.h"

#include "frontends/Liason.h"


using Liason::setMinibuffer;


ControlThesaurus::ControlThesaurus(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


void ControlThesaurus::showEntry(string const & entry)
{
	oldstr_ = entry;
	show();
}


void ControlThesaurus::replace(string const & newstr)
{
	/* FIXME: this is not suitable ! We need to have a "lock"
	 * on a particular charpos in a paragraph that is broken on
	 * deletion/change !
	 */
	int const replace_count =
		lyxfind::LyXReplace(bufferview(), oldstr_, newstr,
				    true, true, true, false, true);

	oldstr_ = newstr;

	if (replace_count == 0)
		setMinibuffer(&lv_, _("String not found!"));
	else
		setMinibuffer(&lv_, _("String has been replaced."));
}


Thesaurus::Meanings const & ControlThesaurus::getMeanings(string const & str)
{
	if (str != laststr_)
		meanings_ = thesaurus.lookup(str);

	return meanings_;
}
