/**
 * \file FormMathsDeco.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli, pdenapo@dm.uba.ar
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include "FormMathsDeco.h"
#include "form_maths_deco.h"
#include "Dialogs.h"
#include "bmtable.h"

#include "deco.xbm"

static char const * decoration_names[] = {
	"widehat", "widetilde", "overbrace", "overleftarrow", "overrightarrow", 
	"overline", "underbrace", "underline"
};

static int const nr_decoration_names = sizeof(decoration_names) / sizeof(char const *);
 
FormMathsDeco::FormMathsDeco(LyXView * lv, Dialogs * d,
			     FormMathsPanel const & p)
	: FormMathsSub(lv, d, p, _("Maths Decorations"))
{}


FL_FORM * FormMathsDeco::form() const
{
	if (dialog_.get())
 		return dialog_->form;
	return 0;
}


void FormMathsDeco::build()
{
	dialog_.reset(build_maths_deco());

	fl_set_bmtable_data(dialog_->bmtable, 3, 3,
			    deco_width, deco_height, deco_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable, 8);

	bc().setCancel(dialog_->button_cancel);
	bc().addReadOnly(dialog_->bmtable);
}


void FormMathsDeco::apply()
{
	int const i = fl_get_bmtable(dialog_->bmtable);
 
	if (i >= nr_decoration_names)
		return;
 
	parent_.insertSymbol(decoration_names[i]);
}
