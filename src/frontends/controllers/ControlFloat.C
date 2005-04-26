/**
 * \file ControlFloat.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlFloat.h"
#include "funcrequest.h"
#include "insets/insetfloat.h"


using std::string;

namespace lyx {
namespace frontend {

ControlFloat::ControlFloat(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlFloat::initialiseParams(string const & data)
{
	InsetFloatParams params;
	InsetFloatMailer::string2params(data, params);
	params_.reset(new InsetFloatParams(params));
	return true;
}


void ControlFloat::clearParams()
{
	params_.reset();
}


void ControlFloat::dispatchParams()
{
	string const lfun = InsetFloatMailer::params2string(params());
	kernel().dispatch(FuncRequest(getLfun(), lfun));
}

} // namespace frontend
} // namespace lyx
