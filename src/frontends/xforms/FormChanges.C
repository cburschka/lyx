/**
 * \file FormChanges.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "xformsBC.h"
#include "ControlChanges.h"
#include "FormChanges.h"
#include "forms/form_changes.h"
#include "xforms_helpers.h"

#include FORMS_H_LOCATION

typedef FormController<ControlChanges, FormView<FD_changes> > base_class;

FormChanges::FormChanges(Dialog & parent)
	: base_class(parent, _("Merge changes"))
{}


void FormChanges::build()
{
	dialog_.reset(build_changes(this));

	// Manage the cancel/close buttons
	bcview().setCancel(dialog_->button_close);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->button_accept);
	bcview().addReadOnly(dialog_->button_reject);
}


void FormChanges::update()
{
	input(dialog_->button_next, 0);
}


ButtonPolicy::SMInput FormChanges::input(FL_OBJECT * obj, long)
{
	if (obj == dialog_->button_accept) {
		controller().accept();

	} else if (obj == dialog_->button_reject) {
		controller().reject();

	} else if (obj == dialog_->button_next) {

		bool const exist = controller().find();
		setEnabled(dialog_->button_accept, exist);
		setEnabled(dialog_->button_reject, exist);
		setEnabled(dialog_->button_next, exist);

		string const author = exist ? controller().getChangeAuthor() : "";
		fl_set_object_label(dialog_->text_author, author.c_str());

		string const date = exist ? controller().getChangeDate() : "";
		fl_set_object_label(dialog_->text_date, date.c_str());

		// Yes, this is needed.
		fl_redraw_form(form());
	}

	return ButtonPolicy::SMI_VALID;
}
