/**
 * \file ControlMinipage.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlMinipage.h"
#include "funcrequest.h"


using std::string;


ControlMinipage::ControlMinipage(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlMinipage::initialiseParams(string const & data)
{
	InsetMinipage::Params params;
	InsetMinipageMailer::string2params(data, params);
	params_.reset(new InsetMinipage::Params(params));
	return true;
}


void ControlMinipage::clearParams()
{
	params_.reset();
}


void ControlMinipage::dispatchParams()
{
	string const lfun = InsetMinipageMailer::params2string(params());
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}
