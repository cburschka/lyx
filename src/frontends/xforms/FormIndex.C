/**
 * \file xforms/FormIndex.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlIndex.h"
#include "FormIndex.h"
#include "forms/form_index.h"
#include FORMS_H_LOCATION

typedef FormCB<ControlIndex, FormDB<FD_index> > base_class;

FormIndex::FormIndex()
	: base_class(_("Index"))
{}


void FormIndex::build()
{
	dialog_.reset(build_index(this));

	fl_set_input_return(dialog_->input_key, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input_key);

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->input_key);

}


void FormIndex::update()
{
	fl_set_input(dialog_->input_key,
		     controller().params().getContents().c_str());
}


void FormIndex::apply()
{
	controller().params().setContents(fl_get_input(dialog_->input_key));
}
