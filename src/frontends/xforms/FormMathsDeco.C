/**
 * \file FormMathsDeco.C
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

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormMathsDeco.h"
#include "forms/form_maths_deco.h"
#include "bmtable.h"
#include FORMS_H_LOCATION

#include "deco.xbm"


FormMathsDeco::FormMathsDeco(LyXView & lv, Dialogs & d,
			     FormMathsPanel const & p)
	: FormMathsSub(lv, d, p, _("Maths Decorations & Accents"), false)
{}


FL_FORM * FormMathsDeco::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormMathsDeco::build()
{
	dialog_.reset(build_maths_deco(this));

	fl_set_bmtable_data(dialog_->bmtable_deco1, 3, 4,
			    deco1_width, deco1_height, deco1_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable_deco1, 12);

	fl_set_bmtable_data(dialog_->bmtable_deco2, 4, 3,
			    deco2_width, deco2_height, deco2_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable_deco2, 10);

	bc().setCancel(dialog_->button_close);
	bc().addReadOnly(dialog_->bmtable_deco1);
	bc().addReadOnly(dialog_->bmtable_deco2);
}


void FormMathsDeco::apply()
{
	if (deco_ < nr_latex_deco)
		parent_.insertSymbol(latex_deco[deco_]);
}


bool FormMathsDeco::input(FL_OBJECT * ob, long)
{
	deco_ = fl_get_bmtable(ob);
	if (deco_ < 0)
		return false;
	//if (ob == dialog_->bmtable_deco1)
	//	deco_ += 0;
	if (ob == dialog_->bmtable_deco2)
		deco_ += 12;
	apply();
	return true;
}
