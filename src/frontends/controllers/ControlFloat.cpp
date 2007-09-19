/**
 * \file ControlFloat.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlFloat.h"
#include "FuncRequest.h"


using std::string;

namespace lyx {
namespace frontend {

ControlFloat::ControlFloat(Dialog & parent)
	: Controller(parent)
{}


bool ControlFloat::initialiseParams(string const & data)
{
	InsetFloatMailer::string2params(data, params_);
	return true;
}


void ControlFloat::clearParams()
{
	params_ = InsetFloatParams();
}


void ControlFloat::dispatchParams()
{
	string const lfun = InsetFloatMailer::params2string(params());
	dispatch(FuncRequest(getLfun(), lfun));
}

} // namespace frontend
} // namespace lyx
