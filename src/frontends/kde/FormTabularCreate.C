/**
 * \file FormTabularCreate.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "gettext.h"
#include "support/lstrings.h"
 
#include "FormTabularCreate.h"
#include "ControlTabularCreate.h" 
#include "tabcreatedlg.h"
 
FormTabularCreate::FormTabularCreate(ControlTabularCreate & c)
	: KFormBase<ControlTabularCreate, TabularCreateDialog>(c)
{
}


void FormTabularCreate::build()
{
	dialog_.reset(new TabularCreateDialog(this, 0, _("LyX: Insert Table")));

	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_cancel);
}


void FormTabularCreate::apply()
{
	// FIXME: angus, what's with this ? Why should the dialogs know about
	// how LFUN represents the row, column ... this should be params(uint, uint) 
	controller().params() = tostr(dialog_->spin_rows->value()) + " " + tostr(dialog_->spin_cols->value());
}
