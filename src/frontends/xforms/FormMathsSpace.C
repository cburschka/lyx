/**
 * \file FormMathsSpace.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "FormMathsSpace.h"
#include "forms/form_maths_space.h"
#include "ControlMath.h"
#include "xformsBC.h"

#include FORMS_H_LOCATION

extern char * latex_mathspace[];

typedef FormCB<ControlMathSub, FormDB<FD_maths_space> > base_class;

FormMathsSpace::FormMathsSpace()
	: base_class(_("Math Spacing"), false),
	  space_(-1)
{}


void FormMathsSpace::build()
{
	dialog_.reset(build_maths_space(this));

	space_ = -1;

	bcview().setCancel(dialog_->button_close);

	bcview().addReadOnly(dialog_->button_negative);
	bcview().addReadOnly(dialog_->button_negmedspace);
	bcview().addReadOnly(dialog_->button_negthickspace);
	bcview().addReadOnly(dialog_->button_thin);
	bcview().addReadOnly(dialog_->button_medium);
	bcview().addReadOnly(dialog_->button_thick);
	bcview().addReadOnly(dialog_->button_quadratin);
	bcview().addReadOnly(dialog_->button_twoquadratin);
}


void FormMathsSpace::apply()
{
	if (space_ >= 0)
		controller().insertSymbol(latex_mathspace[space_]);
}

ButtonPolicy::SMInput FormMathsSpace::input(FL_OBJECT *, long data)
{
	space_ = -1;

	if (data >= 0 && data < 8) {
		space_ = short(data);
		apply();
	}
	return ButtonPolicy::SMI_VALID;
}
