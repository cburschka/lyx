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
#include "bmtable.h"

#include "style.xbm"
#include "font.xbm"

#include "debug.h"

char const * latex_mathstyle[] = {
	"displaystyle", "textstyle", "scriptstyle", "scriptscriptstyle"
};

kb_action latex_mathfontcmds[] = {
	LFUN_BOLD, LFUN_SANS, LFUN_ROMAN, LFUN_ITAL, LFUN_CODE,
	LFUN_NOUN, LFUN_FRAK, LFUN_EMPH, LFUN_FREE, LFUN_DEFAULT
};



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

	fl_set_bmtable_data(dialog_->bmtable_style1, 1, 1,
			    style1_width, style1_height, style1_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable_style1, 1);
	bc().addReadOnly(dialog_->bmtable_style1);

	fl_set_bmtable_data(dialog_->bmtable_style2, 1, 3,
			    style2_width, style2_height, style2_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable_style2, 3);
	bc().addReadOnly(dialog_->bmtable_style2);

	fl_set_bmtable_data(dialog_->bmtable_font1, 1, 5,
			    font1_width, font1_height, font1_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable_font1, 5);
	bc().addReadOnly(dialog_->bmtable_font1);

	fl_set_bmtable_data(dialog_->bmtable_font2, 1, 3,
			    font2_width, font2_height, font2_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable_font2, 3);
	bc().addReadOnly(dialog_->bmtable_font2);

	bc().setCancel(dialog_->button_close);
}


void FormMathsStyle::apply()
{
	if ((style_ >= 0) && (style_ < 4)) 
		parent_.insertSymbol(latex_mathstyle[style_]);
	else if ((style_ >= 4) && (style_ < 14)) 
		parent_.dispatchFunc(latex_mathfontcmds[style_ - 4]);
}


bool FormMathsStyle::input(FL_OBJECT * ob, long data)
{
	style_ = fl_get_bmtable(ob);
	if (style_ < 0) return false;
	//if (ob == dialog_->bmtable_style1) style_ += 0; 
	if (ob == dialog_->bmtable_style2) style_ += 1;
	if (ob == dialog_->bmtable_font1)  style_ += 4;
	if (ob == dialog_->bmtable_font2)  style_ += 9;
	if (data >= 12) style_ = short(data);
	apply();
	return true;
}
