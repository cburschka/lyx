/**
 * \file FormBibtex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming
 * \author John Levon
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ControlBibtex.h"
#include "FormBibtex.h"
#include "gettext.h"
#include "xformsBC.h"
#include "debug.h"

FormBibtex::FormBibtex(ControlBibtex & c)
	: FormBase2<ControlBibtex, FD_form_bibtex>(c, _("BibTeX Database"))
{}


void FormBibtex::build()
{
	dialog_.reset(build_bibtex());

	fl_set_input_return(dialog_->database, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->style, FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_cancel);
	bc().refresh();

	bc().addReadOnly(dialog_->database);
	bc().addReadOnly(dialog_->style);
}


ButtonPolicy::SMInput FormBibtex::input(FL_OBJECT *, long)
{
	// minimal validation 
	if (!compare(fl_get_input(dialog_->database),""))
		return ButtonPolicy::SMI_NOOP;

	return ButtonPolicy::SMI_VALID;
}


void FormBibtex::update()
{
	fl_set_input(dialog_->database,
		     controller().params().getContents().c_str());
	fl_set_input(dialog_->style,
		     controller().params().getOptions().c_str());
}


void FormBibtex::apply()
{
	controller().params().setContents(fl_get_input(dialog_->database));
	controller().params().setOptions(fl_get_input(dialog_->style));
}
