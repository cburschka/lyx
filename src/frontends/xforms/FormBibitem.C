/**
 * \file FormBibitem.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "xformsBC.h"
#include "ControlBibitem.h"
#include "FormBibitem.h"
#include "forms/form_bibitem.h"
#include FORMS_H_LOCATION
#include "gettext.h"
#include "support/lstrings.h" // compare

typedef FormCB<ControlBibitem, FormDB<FD_bibitem> > base_class;

FormBibitem::FormBibitem()
	: base_class(_("Bibliography Entry"))
{}


void FormBibitem::build()
{
	dialog_.reset(build_bibitem(this));

	fl_set_input_return(dialog_->input_key,   FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_label, FL_RETURN_CHANGED);

	setPrehandler(dialog_->input_key);
	setPrehandler(dialog_->input_label);

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->input_key);
	bc().addReadOnly(dialog_->input_label);
}


ButtonPolicy::SMInput FormBibitem::input(FL_OBJECT *, long)
{
	// minimal validation
	if (!compare(fl_get_input(dialog_->input_key), ""))
		return ButtonPolicy::SMI_NOOP;

	return ButtonPolicy::SMI_VALID;
}


void FormBibitem::update()
{
	fl_set_input(dialog_->input_key,
		     controller().params().getContents().c_str());
	fl_set_input(dialog_->input_label,
		     controller().params().getOptions().c_str());
}


void FormBibitem::apply()
{
	controller().params().setContents(fl_get_input(dialog_->input_key));
	controller().params().setOptions(fl_get_input(dialog_->input_label));
}
