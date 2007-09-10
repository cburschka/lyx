/**
 * \file ControlVSpace.cpp
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

#include "FuncRequest.h"
#include "insets/InsetVSpace.h"


using std::istringstream;
using std::string;

namespace lyx {
namespace frontend {

ControlVSpace::ControlVSpace(Dialog & parent)
	: Controller(parent)
{}


bool ControlVSpace::initialiseParams(string const & data)
{
	InsetVSpaceMailer::string2params(data, params_);
	dialog().setButtonsValid(true);

	return true;
}


void ControlVSpace::clearParams()
{
	params_ = VSpace();
}


void ControlVSpace::dispatchParams()
{
	string const str = InsetVSpaceMailer::params2string(params_);
	dispatch(FuncRequest(getLfun(), str));
}


VSpace & ControlVSpace::params()
{
	return params_;
}


VSpace const & ControlVSpace::params() const
{
	return params_;
}

} // namespace frontend
} // namespace lyx
