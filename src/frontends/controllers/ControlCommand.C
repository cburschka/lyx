/**
 * \file ControlCommand.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlCommand.h"
#include "Kernel.h"

#include "funcrequest.h"
#include "insets/insetcommand.h"


ControlCommand::ControlCommand(Dialog & dialog, string const & lfun_name)
	: Dialog::Controller(dialog),
	  lfun_name_(lfun_name)
{}


void ControlCommand::initialiseParams(string const & data)
{
	InsetCommandMailer::string2params(data, params_);
}


void ControlCommand::clearParams()
{
	params_.setCmdName(string());
	params_.setOptions(string());
	params_.setContents(string());
}


void ControlCommand::dispatchParams()
{
	if (lfun_name_.empty())
		return;

	string const lfun = InsetCommandMailer::params2string(lfun_name_,
							      params_);
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}
