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

static int minh, minw;

FormError::FormError(LyXView * lv, Dialogs * d)
	: FormBase(lv, d, _("LaTeX Error")),
	  dialog_(0), ih_(0), message_("")
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


void FormError::showInset( InsetError * const inset )
{
	if( dialogIsOpen || inset == 0 ) return;

	ih_ = inset->hide.connect(slot(this, &FormError::hide));

	message_ = inset->getContents();
	show();
}


void FormError::update()
{
	fl_set_form_minsize(form(), minw, minh);
	fl_set_object_label(dialog_->message, message_.c_str());
}


void FormError::build()
{
	dialog_ = build_error();

	// XFORMS bug workaround
	// Define the min/max dimensions. Actually applied in update()
	minw = form()->w; minh = form()->h;
}


void FormError::clearStore()
{
	ih_.disconnect();
	message_.empty();
}


FL_FORM * const FormError::form() const
{
	if( dialog_ ) // no need to test for dialog_->form
		return dialog_->form;
	else
		return 0;
}
