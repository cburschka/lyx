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

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormMathsDelim.h"
#include "forms/form_maths_delim.h"
#include "Dialogs.h"
#include "frontends/LyXView.h"
#include "bmtable.h"
#include "debug.h"
#include "support/lstrings.h"
#include "lyxfunc.h"
#include FORMS_H_LOCATION

#include "delim.xbm"
#include "delim0.xpm"
#include "delim1.xpm"

static int const delim_rversion[] = {
	1,1,3,3,4,5,7,7,9,9,10,11,
	13,13,14,15,16,17,19,19,20,21,22,23 };

static char const * delim_values[] = {
	"(", ")", "lceil",  "rceil",  "uparrow",  "Uparrow",
	"[", "]", "lfloor", "rfloor", "updownarrow", "Updownarrow",
	"{", "}",  "/", "\\",  "downarrow",  "Downarrow",
	"langle",  "rangle", "|", "Vert", ".", 0
};

using std::endl;

FormMathsDelim::FormMathsDelim(LyXView * lv, Dialogs * d,
			       FormMathsPanel const & p)
	: FormMathsSub(lv, d, p, _("Maths Delimiters"), false)
{}


FL_FORM * FormMathsDelim::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormMathsDelim::build()
{
	dialog_.reset(build_maths_delim(this));

	fl_set_button(dialog_->radio_left, 1);
	// Initialize button_pix to "()" as found in images/delim0.xpm:
	fl_set_pixmap_data(dialog_->button_pix, const_cast<char**>(delim0));
	dialog_->radio_left->u_ldata = 0;
	dialog_->radio_right->u_ldata = 1;
	//dialog_->radio_both->u_ldata = 2;

	fl_set_bmtable_data(dialog_->bmtable, 12, 2,
			    delim_width, delim_height, delim_bits);
	fl_set_bmtable_maxitems(dialog_->bmtable, 23);

	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->bmtable);
	bc().addReadOnly(dialog_->radio_right);
	bc().addReadOnly(dialog_->radio_left);
	bc().addReadOnly(dialog_->radio_both);
	bc().addReadOnly(dialog_->button_pix);
}


void FormMathsDelim::apply()
{
	int const left = int(dialog_->radio_left->u_ldata);
	int const right= int(dialog_->radio_right->u_ldata);

	ostringstream ost;
	ost << delim_values[left] << ' ' << delim_values[right];

	lv_->getLyXFunc()->dispatch(LFUN_MATH_DELIM, ost.str().c_str());
}

bool FormMathsDelim::input(FL_OBJECT *, long)
{
	int left = int(dialog_->radio_left->u_ldata);
	int right= int(dialog_->radio_right->u_ldata);
	int const side = (fl_get_button(dialog_->radio_right) != 0);

	int const i = fl_get_bmtable(dialog_->bmtable);
	int const button = fl_get_bmtable_numb(dialog_->bmtable);
	bool const both = (button == FL_MIDDLE_MOUSE ||
			   fl_get_button(dialog_->radio_both) != 0);

	if (i >= 0) {
		if (side || (button == FL_RIGHT_MOUSE))
			right = i;
		else {
			left = i;
			if (both) {
				right = delim_rversion[i];
				// Add left delimiter in "both" case if right one was pressed:
				for (int j = 0; j <= 23; ++j) {
					if (delim_rversion[j] == left) {
						right = left;
						left = j;
					}
				}
			}
		}
	}

	// Re-initialize button_pix to solid blue 
	// (not elegant but works, MV 24.5.2002)
	fl_free_pixmap_pixmap(dialog_->button_pix);
	fl_set_pixmap_data(dialog_->button_pix, const_cast<char**>(delim1));
	Pixmap p1;
	fl_get_pixmap_pixmap(dialog_->button_pix, &p1, 0);
	
	fl_draw_bmtable_item(dialog_->bmtable, left, p1, 0, 0);
	fl_draw_bmtable_item(dialog_->bmtable, right, p1, 16, 0);
	fl_redraw_object(dialog_->button_pix);

	dialog_->radio_left->u_ldata  = left;
	dialog_->radio_right->u_ldata = right;

	return true;
}
