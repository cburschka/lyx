// -*- C++ -*-
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

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ControlCommand.h"
#include "BufferView.h"
#include "ButtonController.h"
#include "Dialogs.h"
#include "lyxfunc.h"
#include "LyXView.h"
#include "support/LAssert.h"
#include "ViewBase.h"

ControlCommand::ControlCommand(LyXView & lv, Dialogs & d, kb_action ac)
	: ControlConnectInset<InsetCommand>(lv, d),
	  params_(0), action_(ac)
{}


void ControlCommand::showInset(InsetCommand * inset)
{
	if (inset == 0) return;  // maybe we should Assert this?

	connectInset(inset);
	show(inset->params());
}


void ControlCommand::createInset(string const & arg)
{
	connectInset();

	if ( !arg.empty() )
		bc().valid(); // so that the user can press Ok

	InsetCommandParams params;
	params.setFromString(arg);
	show(params);
}


void ControlCommand::update()
{
	if (params_) delete params_;

	if (inset_)
		params_ = new InsetCommandParams(inset_->params());
	else
		params_ = new InsetCommandParams;

	bc().readOnly(isReadonly());
	view().update();
}


void ControlCommand::show(InsetCommandParams const & params)
{
	if (params_) delete params_;
	params_ = new InsetCommandParams(params);

	bc().readOnly(isReadonly());
	view().show();
}


void ControlCommand::hide()
{
	if (params_) {
		delete params_;
		params_ = 0;
	}

	disconnect();
	view().hide();
}

InsetCommandParams & ControlCommand::params() const
{
	Assert(params_);
	return *params_;
}


void ControlCommand::apply()
{
	view().apply();

	if (inset_) {
		// Only update if contents have changed
		if (params() != inset_->params()) {
			inset_->setParams(params());
			lv_.view()->updateInset(inset_, true);
		}
	} else if (action_ != LFUN_NOACTION){
		lv_.getLyXFunc()->Dispatch(action_, params().getAsString());
	}
}
