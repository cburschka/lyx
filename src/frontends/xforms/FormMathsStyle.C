/**
 * \file FormMathsStyle.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli, pdenapo@dm.uba.ar
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * Adapted from FormMathsSpace martin.vermeer@hut.fi
 */

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include "FormMathsStyle.h"
#include "form_maths_style.h"

extern char * latex_mathstyle[];

FormMathsStyle::FormMathsStyle(LyXView * lv, Dialogs * d,
			       FormMathsPanel const & p)
	: FormMathsSub(lv, d, p, _("Maths Styles & Fonts"), false),
	  style_(-1)
{}


FL_FORM * FormMathsStyle::form() const
{
	if (dialog_.get())
 		return dialog_->form;
	return 0;
}


void FormMathsStyle::build()
{
	dialog_.reset(build_maths_style());

	fl_set_button(dialog_->radio_text, 1);
	style_ = 1;

	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);

	bc().addReadOnly(dialog_->radio_display);
	bc().addReadOnly(dialog_->radio_text);
	bc().addReadOnly(dialog_->radio_script);
	bc().addReadOnly(dialog_->radio_scriptscript);
	bc().addReadOnly(dialog_->radio_bold);
	bc().addReadOnly(dialog_->radio_calligraphic);
	bc().addReadOnly(dialog_->radio_roman);
	bc().addReadOnly(dialog_->radio_typewriter);
	bc().addReadOnly(dialog_->radio_sans);
	bc().addReadOnly(dialog_->radio_italic);
	bc().addReadOnly(dialog_->radio_bbbold);
	bc().addReadOnly(dialog_->radio_fraktur);
	bc().addReadOnly(dialog_->radio_textrm);
	bc().addReadOnly(dialog_->radio_normal);
}


void FormMathsStyle::apply()
{
	if (style_ >= 0)
		parent_.insertSymbol(latex_mathstyle[style_]);
}

bool FormMathsStyle::input(FL_OBJECT *, long data)
{
	style_ = -1;
 
	if (data >= 0 && data < 14) {
		style_ = short(data);
	}
	return true;
}
