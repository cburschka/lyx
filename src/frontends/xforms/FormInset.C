// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif


#include "Dialogs.h"
#include "LyXView.h"
#include "FormInset.h"

FormInset::FormInset(LyXView * lv, Dialogs * d, string const & t,
		     ButtonPolicy * bp,
		     char const * close, char const * cancel)
	: FormBaseBD(lv, d, t, bp, close, cancel), ih_(0)
{}


void FormInset::connect()
{
	u_ = d_->updateBufferDependent.
		 connect(slot(this, &FormInset::updateSlot));
	h_ = d_->hideBufferDependent.
		 connect(slot(this, &FormInset::hide));
	FormBase::connect();
}


void FormInset::disconnect()
{
	ih_.disconnect();
	FormBaseBD::disconnect();
}


void FormInset::updateSlot(bool switched)
{
	if (switched)
		hide();
	else
		update();
}


FormCommand::FormCommand(LyXView * lv, Dialogs * d, string const & t,
			 ButtonPolicy * bp,
			 char const * close, char const * cancel)
	: FormInset(lv, d, t, bp, close, cancel),
	  inset_(0)
{}


void FormCommand::disconnect()
{
	inset_ = 0;
	params = InsetCommandParams(string());
	FormInset::disconnect();
}

       
void FormCommand::showInset(InsetCommand * inset)
{
	if (inset == 0) return;  // maybe we should Assert this?

	// If connected to another inset, disconnect from it.
	if (inset_)
		ih_.disconnect();

	inset_    = inset;
	params    = inset->params();
	ih_ = inset->hide.connect(slot(this, &FormCommand::hide));
	show();
}


void FormCommand::createInset(string const & arg)
{
	if (inset_) {
		ih_.disconnect();
		inset_ = 0;
	}

	params.setFromString(arg);
	if (! (params.getContents().empty() && params.getOptions().empty()) )
		bc_.valid(); // so that the user can press Ok
	show();
}
