/**
 * \file xforms/FormTabularCreate.C
 * See the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
#include <utility>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlTabularCreate.h"
#include "FormTabularCreate.h"
#include "forms/form_tabular_create.h"
#include "support/lstrings.h"
#include FORMS_H_LOCATION


using std::make_pair;


typedef FormCB<ControlTabularCreate, FormDB<FD_tabular_create> > base_class;

FormTabularCreate::FormTabularCreate()
	: base_class(_("Insert Tabular"))
{}


void FormTabularCreate::build()
{
	dialog_.reset(build_tabular_create(this));

	fl_set_slider_bounds(dialog_->slider_rows, 1, 50);
	fl_set_slider_bounds(dialog_->slider_columns, 1, 50);
	fl_set_slider_value(dialog_->slider_rows, 5);
	fl_set_slider_value(dialog_->slider_columns, 5);
	fl_set_slider_precision(dialog_->slider_rows, 0);
	fl_set_slider_precision(dialog_->slider_columns, 0);

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
}


void FormTabularCreate::apply()
{
	unsigned int ysize = (unsigned int)(fl_get_slider_value(dialog_->slider_columns) + 0.5);
	unsigned int xsize = (unsigned int)(fl_get_slider_value(dialog_->slider_rows) + 0.5);

	controller().params() = make_pair(xsize, ysize);
}
