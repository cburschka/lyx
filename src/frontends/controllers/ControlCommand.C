/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlCommand.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlCommand.h"
#include "buffer.h"
#include "Dialogs.h"
#include "lyxfunc.h"
#include "BufferView.h"
#include "funcrequest.h"


ControlCommand::ControlCommand(LyXView & lv, Dialogs & d, kb_action ac)
	: ControlInset<InsetCommand, InsetCommandParams>(lv, d),
	  action_(ac)
{}


InsetCommandParams const ControlCommand::getParams(string const & arg)
{
	InsetCommandParams params;
	params.setFromString(arg);
	return params;
}


InsetCommandParams const ControlCommand::getParams(InsetCommand const & inset)
{
	return inset.params();
}


void ControlCommand::applyParamsToInset()
{
	inset()->setParams(params());
	bufferview()->updateInset(inset(), true);
}


void ControlCommand::applyParamsNoInset()
{
	if (action_ == LFUN_NOACTION)
		return;
	lyxfunc().dispatch(FuncRequest(action_, params().getAsString()));
}
