/**
 * \file FormBibitem.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming
 * \author John Levon
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif


#include "Dialogs.h"
#include "FormBibitem.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_bibitem.h"
#include "lyxfunc.h"
#include "debug.h"

using std::endl;

FormBibitem::FormBibitem(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Bibliography Entry"), new OkCancelReadOnlyPolicy),
	  dialog_(0)
{
	d->showBibitem.connect(slot(this, &FormBibitem::showInset));
}


FormBibitem::~FormBibitem()
{
	delete dialog_;
}


FL_FORM * FormBibitem::form() const
{
	if (dialog_)
		return dialog_->form;
	return 0;
}


void FormBibitem::connect()
{
	fl_set_form_maxsize(form(), 2 * minw_, minh_);
	FormCommand::connect();
}
	

void FormBibitem::build()
{
	dialog_ = build_bibitem();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_input_return(dialog_->key, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->label, FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setCancel(dialog_->button_cancel);
	bc_.refresh();

	bc_.addReadOnly(dialog_->key);
	bc_.addReadOnly(dialog_->label);
}


bool FormBibitem::input(FL_OBJECT *, long)
{
	// minimal validation 
	if (!strcmp(fl_get_input(dialog_->key),""))
		return false;

	return true;
}


void FormBibitem::update()
{
	fl_set_input(dialog_->key, params.getContents().c_str());
	fl_set_input(dialog_->label, params.getOptions().c_str());
	// Surely, this should reset the buttons to their original state?
	// It doesn't. Instead "Restore" becomes a "Close"
	//bc_.refresh();
	bc_.readOnly(lv_->buffer()->isReadonly());
}


void FormBibitem::apply()
{
	if (lv_->buffer()->isReadonly())
		return;

	params.setContents(fl_get_input(dialog_->key));
	params.setOptions(fl_get_input(dialog_->label));

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
			// FIXME: confirm, is this only necessary for FormBibTeX ???
			if (params.getContents() != inset_->params().getContents())
				lv_->view()->ChangeCitationsIfUnique(
					inset_->params().getContents(), params.getContents());

		inset_->setParams(params);
		lv_->view()->updateInset(inset_, true);

		// We need to do a redraw because the maximum
		// InsetBibKey width could have changed
		lv_->view()->redraw();
		lv_->view()->fitCursor(lv_->view()->getLyXText());
		}
	} else
		lyxerr[Debug::GUI] << "Editing non-existent bibitem !" << endl;
}
