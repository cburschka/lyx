/**
 * \file FormBibtex.C
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
#include "FormBibtex.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_bibtex.h"
#include "lyxfunc.h"
#include "debug.h"

using std::endl;

FormBibtex::FormBibtex(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("BibTeX Database"))
{
	d->showBibtex.connect(slot(this, &FormBibtex::showInset));
}


FL_FORM * FormBibtex::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormBibtex::connect()
{
	fl_set_form_maxsize(form(), 2 * minw_, minh_);
	FormCommand::connect();
}
	

void FormBibtex::build()
{
	dialog_.reset(build_bibtex());

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_input_return(dialog_->database, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->style, FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_cancel);
	bc().refresh();

	bc().addReadOnly(dialog_->database);
	bc().addReadOnly(dialog_->style);
}


bool FormBibtex::input(FL_OBJECT *, long)
{
	// minimal validation 
	if (!compare(fl_get_input(dialog_->database),""))
		return false;

	return true;
}


void FormBibtex::update()
{
	fl_set_input(dialog_->database, params.getContents().c_str());
	fl_set_input(dialog_->style, params.getOptions().c_str());
	// Surely, this should reset the buttons to their original state?
	// It doesn't. Instead "Restore" becomes a "Close"
	//bc().refresh();
	bc().readOnly(lv_->buffer()->isReadonly());
}


void FormBibtex::apply()
{
	if (lv_->buffer()->isReadonly())
		return;

	params.setContents(fl_get_input(dialog_->database));
	params.setOptions(fl_get_input(dialog_->style));

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
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
		lyxerr[Debug::GUI] << "Editing non-existent bibtex inset !" << endl;
}
