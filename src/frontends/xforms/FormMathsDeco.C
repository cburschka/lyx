/**
 * \file FormMathsDeco.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli, pdenapo@dm.uba.ar
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <leeming@lyx.org>
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


static char const * decoration_names[] = {
	"widehat", "widetilde", "overbrace", "overleftarrow", "overrightarrow",
	"overline", "underbrace", "underline", "underleftarrow", "underrightarrow",
	"underleftrightarrow", "overleftrightarrow",
	"hat", "acute", "bar", "dot",
	"check", "grave", "vec", "ddot",
	"breve", "tilde"
};


static int const nr_decoration_names = sizeof(decoration_names) / sizeof(char const *);

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
	if (deco_ < nr_decoration_names)
		parent_.insertSymbol(decoration_names[deco_]);
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
