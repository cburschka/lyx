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

#include FORMS_H_LOCATION

typedef FormController<ControlChanges, FormView<FD_changes> > base_class;

FormChanges::FormChanges(Dialog & parent)
	: base_class(parent, _("LyX: Merge changes"))
{}


void FormChanges::build()
{
	dialog_.reset(build_changes(this));

	bcview().setCancel(dialog_->button_close);
	bcview().addReadOnly(dialog_->button_accept);
	bcview().addReadOnly(dialog_->button_reject);
}


void FormChanges::update()
{
	fl_set_object_label(dialog_->author, "");
	fl_set_object_label(dialog_->date, "");
	// FIXME: enable/disable accept/reject
}


ButtonPolicy::SMInput FormChanges::input(FL_OBJECT * obj, long)
{
	if (obj == dialog_->button_accept) {
		controller().accept();
		return ButtonPolicy::SMI_VALID;
	}

	if (obj == dialog_->button_reject) {
		controller().reject();
		return ButtonPolicy::SMI_VALID;
	}

	if (obj != dialog_->button_next)
		return ButtonPolicy::SMI_VALID;

	controller().find();

	string author(controller().getChangeAuthor());
	string date(controller().getChangeDate());
	if (!date.empty()) {
		date = _("Changed at : ") + date;
	}
	if (!author.empty()) {
		author = _("Change made by : ") + author;
	}
	fl_set_object_label(dialog_->author, author.c_str());
	fl_set_object_label(dialog_->date, date.c_str());

	// Yes, this is needed.
	fl_redraw_form(form());

	return ButtonPolicy::SMI_VALID;
}
