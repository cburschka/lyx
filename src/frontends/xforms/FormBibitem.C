// -*- C++ -*-
/**
 * \file FormBibitem.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk 
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "xformsBC.h"
#include "ControlBibitem.h"
#include "FormBibitem.h"
#include "form_bibitem.h"
#include "gettext.h"
#include "support/lstrings.h" // compare

typedef FormCB<ControlBibitem, FormDB<FD_form_bibitem> > base_class;

FormBibitem::FormBibitem(ControlBibitem & c)
	: base_class(c, _("Bibliography Entry"))
{}


void FormBibitem::build()
{
	dialog_.reset(build_bibitem());

	fl_set_input_return(dialog_->key,   FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->label, FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_cancel);

	bc().addReadOnly(dialog_->key);
	bc().addReadOnly(dialog_->label);
}


ButtonPolicy::SMInput FormBibitem::input(FL_OBJECT *, long)
{
	// minimal validation 
	if (!compare(fl_get_input(dialog_->key), ""))
		return ButtonPolicy::SMI_NOOP;

	return ButtonPolicy::SMI_VALID;
}


void FormBibitem::update()
{
	fl_set_input(dialog_->key,
		     controller().params().getContents().c_str());
	fl_set_input(dialog_->label,
		     controller().params().getOptions().c_str());
}


void FormBibitem::apply()
{
	controller().params().setContents(fl_get_input(dialog_->key));
	controller().params().setOptions(fl_get_input(dialog_->label));
}
