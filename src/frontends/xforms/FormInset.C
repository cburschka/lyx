/**
 * \file FormInset.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "frontends/LyXView.h"
#include "FormInset.h"
#include "xformsBC.h"

using SigC::slot;

FormInset::FormInset(LyXView * lv, Dialogs * d, string const & t)
	: FormBaseBD(lv, d, t), ih_(0)
{}


void FormInset::connect()
{
	u_ = d_->updateBufferDependent.
		connect(slot(this, &FormInset::updateSlot));
	h_ = d_->hideBufferDependent.
		connect(slot(this, &FormInset::hide));
	FormBaseDeprecated::connect();
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


FormCommand::FormCommand(LyXView * lv, Dialogs * d, string const & t)
	: FormInset(lv, d, t),
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
	ih_ = inset->hideDialog.connect(slot(this, &FormCommand::hide));
	show();
}


void FormCommand::createInset(string const & arg)
{
	if (inset_) {
		ih_.disconnect();
		inset_ = 0;
	}

	params.setFromString(arg);
	if ( !arg.empty())
		bc().valid(); // so that the user can press Ok
	show();
}
