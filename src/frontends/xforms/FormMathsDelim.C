/**
 * \file FormMathsDelim.C
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

#include "FormMathsDelim.h"
#include "form_maths_delim.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "bmtable.h"
#include "debug.h"
#include "mathed/symbol_def.h"

#include "delim.xbm"
#include "delim0.xpm"

static int const delim_rversion[] = { 
	1,1,3,3,4,5,7,7,9,9,10,11,
	13,13,14,15,16,17,19,19,20,21,22,23 };
 
static int delim_values[] = {
	'(', ')', LM_lceil,  LM_rceil,  LM_uparrow,  LM_Uparrow,
	'[', ']', LM_lfloor,  LM_rfloor,  LM_updownarrow, LM_Updownarrow,
	'{', '}',  '/', LM_backslash,  LM_downarrow,  LM_Downarrow,
	LM_langle,  LM_rangle, '|', LM_Vert, '.', 0
};

using std::endl;

FormMathsDelim::FormMathsDelim(LyXView * lv, Dialogs * d, 
			       FormMathsPanel const & p)
	: FormMathsSub(lv, d, p, _("Maths Delimiters"))
{}


FL_FORM * FormMathsDelim::form() const
{
	if (dialog_.get())
 		return dialog_->form;
	return 0;
}


void FormMathsDelim::build()
{
	dialog_.reset(build_maths_delim());
	
	fl_set_button(dialog_->radio_left, 1);
	fl_set_pixmap_data(dialog_->button_pix, const_cast<char**>(delim0));
	dialog_->radio_left->u_ldata = 0;
	dialog_->radio_right->u_ldata = 1;
 
	fl_set_bmtable_data(dialog_->bmtable, 6, 4,
			    delim_width, delim_height, delim_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable, 23);

	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);

	bc().addReadOnly(dialog_->bmtable);
	bc().addReadOnly(dialog_->radio_right);
	bc().addReadOnly(dialog_->radio_left);
	bc().addReadOnly(dialog_->button_pix);
}


void FormMathsDelim::apply()
{
	int left = int(dialog_->radio_left->u_ldata);
	int right= int(dialog_->radio_right->u_ldata);

	std::ostringstream ost;
	ost << delim_values[left] << ' ' << delim_values[right];

	lv_->getLyXFunc()->Dispatch(LFUN_MATH_DELIM, ost.str().c_str());
}

bool FormMathsDelim::input(FL_OBJECT *, long)
{
	int left = int(dialog_->radio_left->u_ldata);
	int right= int(dialog_->radio_right->u_ldata);
	int const side = (fl_get_button(dialog_->radio_right) != 0);

	int const i = fl_get_bmtable(dialog_->bmtable);
	int const button = fl_get_bmtable_numb(dialog_->bmtable);
	bool const both = (button == FL_MIDDLE_MOUSE);
	
	if (i>= 0) {
		if (side || (button == FL_RIGHT_MOUSE))
			right = i;
		else {
			left = i;
			if (both)
				right = delim_rversion[i];
		}
	}
 
	Pixmap p1, p2;
 
	p1 = fl_get_pixmap_pixmap(dialog_->button_pix, &p1, &p2);
	fl_draw_bmtable_item(dialog_->bmtable, left, p1, 0, 0);
	fl_draw_bmtable_item(dialog_->bmtable, right, p1, 16, 0);
	fl_redraw_object(dialog_->button_pix);
	
	dialog_->radio_left->u_ldata  = left;
	dialog_->radio_right->u_ldata = right;

	return true;
}
