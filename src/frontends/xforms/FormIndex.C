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
#include "FormIndex.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_index.h"
#include "lyxfunc.h"

FormIndex::FormIndex(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Index"))
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showIndex.connect(slot(this, &FormIndex::showInset));
	d->createIndex.connect(slot(this, &FormIndex::createInset));
}


FL_FORM * FormIndex::form() const
{
	if (dialog_.get()) return dialog_->form;
	return 0;
}


void FormIndex::connect()
{
	fl_set_form_maxsize(form(), 2 * minw_, minh_);
	FormCommand::connect();
}
	

void FormIndex::build()
{
	dialog_.reset(build_index());

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_input_return(dialog_->input_key, FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setUndoAll(dialog_->button_restore);
	bc().refresh();

	bc().addReadOnly(dialog_->input_key);
}


void FormIndex::update()
{
	fl_set_input(dialog_->input_key, params.getContents().c_str());
	// Surely, this should reset the buttons to their original state?
	// It doesn't. Instead "Restore" becomes a "Close"
	//bc().refresh();
	bc().readOnly(lv_->buffer()->isReadonly());
}


void FormIndex::apply()
{
	if (lv_->buffer()->isReadonly()) return;

	params.setContents(fl_get_input(dialog_->input_key));

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else {
		lv_->getLyXFunc()->Dispatch(LFUN_INDEX_INSERT,
					    params.getAsString());
	}
}
