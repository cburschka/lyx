/**
 * \file ControlVSpace.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlVSpace.h"

#include "funcrequest.h"
#include "insets/insetvspace.h"


using std::istringstream;
using std::string;


ControlVSpace::ControlVSpace(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlVSpace::initialiseParams(string const & data)
{
	InsetVSpaceMailer::string2params(data, params_);
	return true;
}


void ControlVSpace::clearParams()
{
	params_ = VSpace();
}


void ControlVSpace::dispatchParams()
{
	string const str = InsetVSpaceMailer::params2string(params_);
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, str));
}


VSpace & ControlVSpace::params()
{
	return params_;
}


VSpace const & ControlVSpace::params() const
{
	return params_;
}
