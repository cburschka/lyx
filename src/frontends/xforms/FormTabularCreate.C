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
#include "BufferView.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "insets/insettabular.h"

FormTabularCreate::FormTabularCreate(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Insert Tabular"),
		     new OkApplyCancelReadOnlyPolicy),
	  dialog_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showTabularCreate.connect(slot(this, &FormTabularCreate::show));
}


FormTabularCreate::~FormTabularCreate()
{
	delete dialog_;
}


FL_FORM * FormTabularCreate::form() const
{
	if (dialog_) return dialog_->form;
	return 0;
}


void FormTabularCreate::connect()
{
	bc_.valid(true);
	FormBaseBD::connect();
}


void FormTabularCreate::build()
{
	dialog_ = build_tabular_create();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_slider_bounds(dialog_->slider_rows, 1, 50);
	fl_set_slider_bounds(dialog_->slider_columns, 1, 50);
	fl_set_slider_value(dialog_->slider_rows, 5);
	fl_set_slider_value(dialog_->slider_columns, 5);
	fl_set_slider_precision(dialog_->slider_rows, 0);
	fl_set_slider_precision(dialog_->slider_columns, 0);

        // manage the ok, apply and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_cancel);
	bc_.refresh();
}


void FormTabularCreate::apply()
{
	int ysize = int(fl_get_slider_value(dialog_->slider_columns) + 0.5);
	int xsize = int(fl_get_slider_value(dialog_->slider_rows) + 0.5);

	InsetTabular * in = new InsetTabular( *lv_->buffer(), xsize, ysize );
	if (!lv_->view()->open_new_inset(in)) {
		delete in;
	}
}
