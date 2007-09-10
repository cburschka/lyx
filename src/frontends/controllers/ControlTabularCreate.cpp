/**
 * \file ControlTabularCreate.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlTabularCreate.h"
#include "FuncRequest.h"

#include "support/convert.h"


using std::string;

namespace lyx {
namespace frontend {

ControlTabularCreate::ControlTabularCreate(Dialog & parent)
	: Controller(parent)
{}


bool ControlTabularCreate::initialiseParams(string const &)
{
	params_.first  = 5;
	params_.second = 5;
	return true;
}


void ControlTabularCreate::clearParams()
{
	params_.first  = 0;
	params_.second = 0;
}


void ControlTabularCreate::dispatchParams()
{
	string const data = convert<string>(params().first) + ' ' + convert<string>(params().second);
	dispatch(FuncRequest(getLfun(), data));
}

} // namespace frontend
} // namespace lyx
