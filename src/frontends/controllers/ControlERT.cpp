/**
 * \file ControlERT.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlERT.h"
#include "FuncRequest.h"


using std::string;

namespace lyx {
namespace frontend {

ControlERT::ControlERT(Dialog & parent)
	: Controller(parent), status_(InsetERT::Collapsed)
{}


bool ControlERT::initialiseParams(string const & data)
{
	InsetERTMailer::string2params(data, status_);
	return true;
}


void ControlERT::clearParams()
{
	status_ = InsetERT::Collapsed;
}


void ControlERT::dispatchParams()
{
	string const lfun = InsetERTMailer::params2string(status_);
	kernel().dispatch(FuncRequest(getLfun(), lfun));
}

} // namespace frontend
} // namespace lyx
