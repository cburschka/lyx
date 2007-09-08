/**
 * \file ControlCommand.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlCommand.h"

#include "FuncRequest.h"
#include "insets/InsetCommand.h"


using std::string;

namespace lyx {
namespace frontend {

ControlCommand::ControlCommand(Dialog & dialog, string const & command_name,
			       string const & lfun_name)
	: Controller(dialog), params_(command_name),
	  lfun_name_(lfun_name)
{}


bool ControlCommand::initialiseParams(string const & data)
{
	// The name passed with LFUN_INSET_APPLY is also the name
	// used to identify the mailer.
	InsetCommandMailer::string2params(lfun_name_, data, params_);
	return true;
}


void ControlCommand::clearParams()
{
	params_.clear();
}


void ControlCommand::dispatchParams()
{
	if (lfun_name_.empty())
		return;

	string const lfun = InsetCommandMailer::params2string(lfun_name_,
							      params_);
	kernel().dispatch(FuncRequest(getLfun(), lfun));
}

} // namespace frontend
} // namespace lyx
