// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */
/* FormTabularCreate.C
 * FormTabularCreate Interface Class Implementation
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormTabularCreate.h"
#include "form_tabular_create.h"
#include "buffer.h"
#include "BufferView.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "insets/insettabular.h"
#include "support/lstrings.h"

using SigC::slot;

FormTabularCreate::FormTabularCreate(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Insert Tabular"))
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showTabularCreate.connect(slot(this, &FormTabularCreate::show));
}


FL_FORM * FormTabularCreate::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormTabularCreate::connect()
{
	bc().valid(true);
	FormBaseBD::connect();
}


void FormTabularCreate::build()
{
	dialog_.reset(build_tabular_create());

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_slider_bounds(dialog_->slider_rows, 1, 50);
	fl_set_slider_bounds(dialog_->slider_columns, 1, 50);
	fl_set_slider_value(dialog_->slider_rows, 5);
	fl_set_slider_value(dialog_->slider_columns, 5);
	fl_set_slider_precision(dialog_->slider_rows, 0);
	fl_set_slider_precision(dialog_->slider_columns, 0);

        // Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().refresh();
}


void FormTabularCreate::apply()
{
	int ysize = int(fl_get_slider_value(dialog_->slider_columns) + 0.5);
	int xsize = int(fl_get_slider_value(dialog_->slider_rows) + 0.5);

	string tmp = tostr(xsize) + " " + tostr(ysize);
	lv_->getLyXFunc()->Dispatch(LFUN_INSET_TABULAR, tmp);
}


void FormTabularCreate::update()
{
	bc().readOnly(lv_->buffer()->isReadonly());
}
