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


ControlCommand::ControlCommand(Dialog & dialog, kb_action ac)
	: Dialog::Controller(dialog),
	  action_(ac)
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
	if (action_ == LFUN_NOACTION)
		return;

	FuncRequest fr(action_, InsetCommandMailer::params2string(params_));
	kernel().dispatch(fr);
}
