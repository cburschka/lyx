/**
 * \file xforms/FormTabularCreate.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author \author Jürgen Vigna, jug@sad.it
 */

#include <config.h>
#include <utility>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlTabularCreate.h"
#include "FormTabularCreate.h"
#include "form_tabular_create.h"
#include "support/lstrings.h"


using std::make_pair;


typedef FormCB<ControlTabularCreate, FormDB<FD_form_tabular_create> > base_class;

FormTabularCreate::FormTabularCreate(ControlTabularCreate & c)
	: base_class(c, _("Insert Tabular"))
{}


void FormTabularCreate::build()
{
	dialog_.reset(build_tabular_create());

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
}


void FormTabularCreate::apply()
{
	unsigned int ysize = (unsigned int)(fl_get_slider_value(dialog_->slider_columns) + 0.5);
	unsigned int xsize = (unsigned int)(fl_get_slider_value(dialog_->slider_rows) + 0.5);

	controller().params() = make_pair(xsize, ysize);
}
