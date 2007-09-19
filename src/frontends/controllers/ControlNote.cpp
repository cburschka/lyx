/**
 * \file ControlNote.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlNote.h"
#include "FuncRequest.h"
#include "insets/InsetNote.h"
#include "gettext.h"


using std::string;

namespace lyx {
namespace frontend {

ControlNote::ControlNote(Dialog & parent)
	: Controller(parent)
{}


bool ControlNote::initialiseParams(string const & data)
{
	InsetNoteMailer::string2params(data, params_);
	return true;
}


void ControlNote::clearParams()
{
	params_ = InsetNoteParams();
}


void ControlNote::dispatchParams()
{
	string const lfun = InsetNoteMailer::params2string(params());
	dispatch(FuncRequest(getLfun(), lfun));
}

} // namespace frontend
} // namespace lyx
