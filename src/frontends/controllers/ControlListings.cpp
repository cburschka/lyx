/**
 * \file ControlListings.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlListings.h"
#include "FuncRequest.h"
#include "insets/InsetListings.h"

#include "debug.h"

using std::string;

namespace lyx {
namespace frontend {

ControlListings::ControlListings(Dialog & parent)
	: Controller(parent), params_()
{}


bool ControlListings::initialiseParams(string const & data)
{
	InsetListingsMailer::string2params(data, params_);
	return true;
}


void ControlListings::clearParams()
{
	params_.clear();
}


void ControlListings::dispatchParams()
{
	string const lfun = InsetListingsMailer::params2string(params());
	kernel().dispatch(FuncRequest(getLfun(), lfun));
}


void ControlListings::setParams(InsetListingsParams const & params)
{
	params_ = params;
}


} // namespace frontend
} // namespace lyx
