/**
 * \file FormMathsSpace.C
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormMathsSpace.h"
#include "forms/form_maths_space.h"
#include FORMS_H_LOCATION

extern char * latex_mathspace[];

FormMathsSpace::FormMathsSpace(LyXView & lv, Dialogs & d,
			       FormMathsPanel const & p)
	: FormMathsSub(lv, d, p, _("Maths Spacing"), false),
	  space_(-1)
{}


FL_FORM * FormMathsSpace::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormMathsSpace::build()
{
	dialog_.reset(build_maths_space(this));

	space_ = -1;

	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->button_thin);
	bc().addReadOnly(dialog_->button_medium);
	bc().addReadOnly(dialog_->button_thick);
	bc().addReadOnly(dialog_->button_negative);
	bc().addReadOnly(dialog_->button_quadratin);
	bc().addReadOnly(dialog_->button_twoquadratin);
}


void FormMathsSpace::apply()
{
	if (space_ >= 0)
		parent_.insertSymbol(latex_mathspace[space_]);
}

bool FormMathsSpace::input(FL_OBJECT *, long data)
{
	space_ = -1;

	if (data >= 0 && data < 6) {
		space_ = short(data);
		apply();
	}
	return true;
}
