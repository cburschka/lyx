// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormIndex.C
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlIndex.h"
#include "FormIndex.h"
#include "LyXView.h"
#include "form_index.h"

typedef FormCB<ControlIndex, FormDB<FD_form_index> > base_class;

FormIndex::FormIndex(ControlIndex & c)
	: base_class(c, _("Index"))
{}


void FormIndex::build()
{
	dialog_.reset(build_index());

	fl_set_input_return(dialog_->input_key, FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
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
