/**
 * \file ControlThesaurus.cpp
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
#include "FuncRequest.h"

using std::string;

namespace lyx {
namespace frontend {

ControlThesaurus::ControlThesaurus(Dialog & parent)
	: Controller(parent)
{}


bool ControlThesaurus::initialiseParams(string const & data)
{
	oldstr_ = from_utf8(data);
	return true;
}


void ControlThesaurus::clearParams()
{
	oldstr_.erase();
}


void ControlThesaurus::replace(docstring const & newstr)
{
	/* FIXME: this is not suitable ! We need to have a "lock"
	 * on a particular charpos in a paragraph that is broken on
	 * deletion/change !
	 */
	docstring const data =
		replace2string(oldstr_, newstr,
				     true,  // case sensitive
				     true,  // match word
				     false, // all words
				     true); // forward
	dispatch(FuncRequest(LFUN_WORD_REPLACE, data));
}


Thesaurus::Meanings const & ControlThesaurus::getMeanings(docstring const & str)
{
	if (str != laststr_)
		meanings_ = thesaurus.lookup(str);
	return meanings_;
}

} // namespace frontend
} // namespace lyx
