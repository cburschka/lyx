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
#include "FormUrl.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_url.h"
#include "lyxfunc.h"

FormUrl::FormUrl(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Url"), new NoRepeatedApplyReadOnlyPolicy),
	  dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showUrl.connect(slot(this, &FormUrl::showInset));
	d->createUrl.connect(slot(this, &FormUrl::createInset));
}


FormUrl::~FormUrl()
{
	delete dialog_;
}


FL_FORM * FormUrl::form() const
{
	if (dialog_) return dialog_->form;
	return 0;
}


void FormUrl::connect()
{
	fl_set_form_maxsize(form(), 2 * minw_, minh_);
	FormCommand::connect();
}
	

void FormUrl::build()
{
	dialog_ = build_url();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_input_return(dialog_->name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->url,  FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_cancel);
	bc_.setUndoAll(dialog_->button_restore);
	bc_.refresh();

	bc_.addReadOnly(dialog_->name);
	bc_.addReadOnly(dialog_->url);
	bc_.addReadOnly(dialog_->radio_html);
}


void FormUrl::update()
{
	fl_set_input(dialog_->url,  params.getContents().c_str());
	fl_set_input(dialog_->name, params.getOptions().c_str());

	if (params.getCmdName() == "url")
		fl_set_button(dialog_->radio_html, 0);
	else
		fl_set_button(dialog_->radio_html, 1);

	bc_.readOnly(lv_->buffer()->isReadonly());
}


void FormUrl::apply()
{
	if (lv_->buffer()->isReadonly()) return;

	params.setContents(fl_get_input(dialog_->url));
	params.setOptions(fl_get_input(dialog_->name));

	if (fl_get_button(dialog_->radio_html))
		params.setCmdName("htmlurl");
	else
		params.setCmdName("url");

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else {
		lv_->getLyXFunc()->Dispatch(LFUN_INSERT_URL,
					    params.getAsString());
	}
}
