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

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "FormError.h"
#include "form_error.h"
#include "insets/inseterror.h"

FormError::FormError( LyXView * lv, Dialogs * d )
	: FormInset( lv, d, _("LaTeX Error") ),
	  dialog_(0), inset_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showError.connect(slot(this, &FormError::showInset));
}


FormError::~FormError()
{
	delete dialog_;
}


FL_FORM * FormError::form() const
{
	if (dialog_ ) return dialog_->form;
	return 0;
}


void FormError::disconnect()
{
	inset_ = 0;
	message_.erase();
	FormInset::disconnect();
}


void FormError::showInset( InsetError * inset )
{
	if (inset == 0) return;

	// If connected to another inset, disconnect from it.
	if (inset_)
		ih_.disconnect();

	inset_ = inset;
	message_ = inset->getContents();
	ih_ = inset->hide.connect(slot(this, &FormError::hide));
	show();
}


void FormError::update()
{
	fl_set_object_label(dialog_->message, message_.c_str());
}


void FormError::build()
{
	dialog_ = build_error();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;
}
