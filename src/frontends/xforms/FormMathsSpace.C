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
	: base_class(_("Maths Spacing"), false),
	  space_(-1)
{}


void FormMathsSpace::build()
{
	dialog_.reset(build_maths_space(this));

	space_ = -1;

	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->button_negative);
	bc().addReadOnly(dialog_->button_negmedspace);
	bc().addReadOnly(dialog_->button_negthickspace);
	bc().addReadOnly(dialog_->button_thin);
	bc().addReadOnly(dialog_->button_medium);
	bc().addReadOnly(dialog_->button_thick);
	bc().addReadOnly(dialog_->button_quadratin);
	bc().addReadOnly(dialog_->button_twoquadratin);
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
