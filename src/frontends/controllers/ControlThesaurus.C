/**
 * \file ControlThesaurus.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlThesaurus.h"

#include "lyxfind.h"
#include "funcrequest.h"

using std::string;


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
	string const data =
		lyx::find::replace2string(oldstr_, newstr,
					  true,  // case sensitive
					  true,  // match word
					  false, // all words
					  true); // forward
	FuncRequest const fr(kernel().bufferview(), LFUN_WORD_REPLACE, data);
	kernel().dispatch(fr);
}


Thesaurus::Meanings const &
ControlThesaurus::getMeanings(string const & str)
{
	if (str != laststr_)
		meanings_ = thesaurus.lookup(str);

	return meanings_;
}
