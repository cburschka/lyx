/**
 * \file FormTabularCreate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormTabularCreate.h"
#include "ControlTabularCreate.h"
#include "forms/form_tabular_create.h"

#include "Tooltips.h"
#include "xformsBC.h"

#include "lyx_forms.h"

using std::make_pair;
using std::string;

namespace lyx {
namespace frontend {

typedef FormController<ControlTabularCreate, FormView<FD_tabular_create> > base_class;

FormTabularCreate::FormTabularCreate(Dialog & parent)
	: base_class(parent, _("Insert Table"))
{}


void FormTabularCreate::build()
{
	dialog_.reset(build_tabular_create(this));

	// Manage the ok, apply and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);

	// set up the tooltips
	string str = _("Number of columns in the tabular.");
	tooltips().init(dialog_->slider_columns, str);
	str = _("Number of rows in the tabular.");
	tooltips().init(dialog_->slider_rows, str);
}


void FormTabularCreate::apply()
{
	unsigned int ysize = (unsigned int)(fl_get_slider_value(dialog_->slider_columns) + 0.5);
	unsigned int xsize = (unsigned int)(fl_get_slider_value(dialog_->slider_rows) + 0.5);

	controller().params() = make_pair(xsize, ysize);
}

} // namespace frontend
} // namespace lyx
