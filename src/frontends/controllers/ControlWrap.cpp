/**
 * \file ControlWrap.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlWrap.h"
#include "FuncRequest.h"


using std::string;

namespace lyx {
namespace frontend {

ControlWrap::ControlWrap(Dialog & parent)
	: Controller(parent)
{}


bool ControlWrap::initialiseParams(string const & data)
{
	InsetWrapMailer::string2params(data, params_);
	return true;
}


void ControlWrap::clearParams()
{
	params_ = InsetWrapParams();
}


void ControlWrap::dispatchParams()
{
	string const lfun = InsetWrapMailer::params2string(params());
	dispatch(FuncRequest(getLfun(), lfun));
}

} // namespace frontend
} // namespace lyx
