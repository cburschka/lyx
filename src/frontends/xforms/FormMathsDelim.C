/**
 * \file FormMathsDelim.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormMathsDelim.h"
#include "forms/form_maths_delim.h"
#include "ControlMath.h"

#include "bmtable.h"
#include "xformsBC.h"

#include "controllers/ButtonController.h"

#include "Lsstream.h"

#include "delim.xbm"
#include "delim0.xpm"
#include "delim1.xpm"


static int const delim_rversion[] = {
1,1,3,3,4,5,7,7,9,9,10,11,

};


static char const * delim_values[] = {
	"(", ")", "lceil",  "rceil",  "uparrow",  "Uparrow",
	"[", "]", "lfloor", "rfloor", "updownarrow", "Updownarrow",
	"{", "}",  "/", "backslash",  "downarrow",  "Downarrow",
	"langle",  "rangle", "|", "Vert", ".", 0
};

using std::endl;


typedef FormController<ControlMath, FormView<FD_maths_delim> > base_class;

FormMathsDelim::FormMathsDelim(Dialog & parent)
	: base_class(parent, _("Math Delimiters"), false)
{}


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

	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);

	bcview().addReadOnly(dialog_->bmtable);
	bcview().addReadOnly(dialog_->radio_right);
	bcview().addReadOnly(dialog_->radio_left);
	bcview().addReadOnly(dialog_->radio_both);
	bcview().addReadOnly(dialog_->button_pix);
}


void FormMathsDelim::apply()
{
	int const left  = int(dialog_->radio_left->u_ldata);
	int const right = int(dialog_->radio_right->u_ldata);

	ostringstream os;
	os << delim_values[left] << ' ' << delim_values[right];
	controller().dispatchDelim(STRCONV(os.str()));
}


void FormMathsDelim::update()
{
	bc().valid();
}


ButtonPolicy::SMInput FormMathsDelim::input(FL_OBJECT *, long)
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

	return ButtonPolicy::SMI_VALID;
}
