/**
 * \file ControlBranch.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlBranch.h"
#include "funcrequest.h"
#include "insets/insetbranch.h"

ControlBranch::ControlBranch(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlBranch::initialiseParams(string const & data)
{
	InsetBranchParams params;
	InsetBranchMailer::string2params(data, params);
	params_.reset(new InsetBranchParams(params));
	
	return true;
}


void ControlBranch::clearParams()
{
	params_.reset();
}

void ControlBranch::dispatchParams()
{
	string const lfun = InsetBranchMailer::params2string(string("branch"), params());
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}

