/**
 * \file FormMathsBitmap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormMathsBitmap.h"

#include "bmtable.h"
#include "forms_gettext.h"
#include "xformsBC.h"

#include "ControlMath.h"


using std::max;
using std::vector;


extern  "C" void C_FormDialogView_CancelCB(FL_OBJECT *, long);
extern  "C" void C_FormDialogView_InputCB(FL_OBJECT *, long);

FD_maths_bitmap::~FD_maths_bitmap()
{
	if (form->visible) fl_hide_form(form);
	fl_free_form(form);
}


typedef FormController<ControlMath, FormView<FD_maths_bitmap> > base_class;


FormMathsBitmap::FormMathsBitmap(Dialog & parent, string const & t, vector<string> const & l)
	: base_class(parent, t, false),
	  latex_(l), ww_(0), x_(0), y_(0), w_(0), h_(0)
{
	ww_ = 2 * FL_abs(FL_BOUND_WIDTH);
	x_ = y_ = ww_;
	y_ += 8;
}


void FormMathsBitmap::addBitmap(BitmapStore const & bm)
{
	bitmaps_.push_back(bm);

	int wx = bm.bw + ww_ / 2;
	int wy = bm.bh + ww_ / 2;
	wx += (wx % bm.nx);
	wy += (wy % bm.ny);

	if (bm.vert) {
		y_ += wy + 8;
		h_ = max(y_, h_);
		w_ = max(x_ + wx + ww_, w_);
	} else  {
		x_ += wx + 8;
		w_ = max(x_, w_);
		h_ = max(y_ + wy + ww_, h_);
	}
}


void FormMathsBitmap::build()
{
	BOOST_ASSERT(bitmaps_.size() > 0);

	h_+= 42; // Allow room for a Close button

	FD_maths_bitmap * fdui = new FD_maths_bitmap;

	fdui->form = fl_bgn_form(FL_UP_BOX, w_, h_);
	fdui->form->u_vdata = this;

	fl_add_box(FL_UP_BOX, 0, 0, w_, h_, "");

	x_ = y_ = ww_;
	y_ += 8;

	int y_close = 0;
	for (vector<BitmapStore>::const_iterator it = bitmaps_.begin();
	     it < bitmaps_.end(); ++it) {
		FL_OBJECT * obj = buildBitmap(*it);

		bcview().addReadOnly(obj);
		y_close = max(y_close, obj->y + obj->h);
	}
	bitmaps_.clear();

	x_ = (fdui->form->w - 90) / 2;
	y_ = y_close + 10;

	string const label = _("Close|^[");
	fdui->button_close = fl_add_button(FL_NORMAL_BUTTON, x_, y_, 90, 30,
					   idex(label).c_str());
	fl_set_button_shortcut(fdui->button_close, scex(label).c_str(), 1);
	fl_set_object_lsize(fdui->button_close, FL_NORMAL_SIZE);
	fl_set_object_callback(fdui->button_close, C_FormDialogView_CancelCB, 0);

	fl_end_form();

	fdui->form->fdui = fdui;

	dialog_.reset(fdui);
}



FL_OBJECT * FormMathsBitmap::buildBitmap(BitmapStore const & bmstore)
{
	// Add a bitmap to a button panel: one bitmap per panel.
	// nt is the number of buttons and nx, ny the nr. of buttons
	// in x and y direction.
	// bw, bh and data are the bitmap dimensions width, height and
	// bit pattern; these come directly from an .xbm file included
	// as source.
	// vert indicates whether the next button panel within this
	// window will be below (true, default) or next to this one.
	//
	// The scaling of the bitmap on top of the buttons will be
	// correct if the nx, ny values are given correctly.
	int wx = bmstore.bw + ww_ / 2;
	int wy = bmstore.bh + ww_ / 2;
	wx += (wx % bmstore.nx);
	wy += (wy % bmstore.ny);

	FL_OBJECT * obj = fl_add_bmtable(1, x_, y_, wx, wy, "");
	fl_set_object_lcol(obj, FL_BLUE);
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_bmtable_data(obj, bmstore.nx, bmstore.ny, bmstore.bw, bmstore.bh,
			    bmstore.data);
	fl_set_bmtable_maxitems(obj, bmstore.nt);
	fl_set_object_callback(obj, C_FormDialogView_InputCB, 0);

	if (bmstore.vert) {
		y_ += wy + 8;
	} else  {
		x_ += wx + 8;
	}

	return obj;
}


int FormMathsBitmap::GetIndex(FL_OBJECT * ob_in)
{
	int k = 0;

	for (FL_OBJECT * ob = form()->first; ob; ob = ob->next) {
		if (ob->objclass != FL_BMTABLE)
			continue;

		if (ob == ob_in)
			return k + fl_get_bmtable(ob);
		else
			k += fl_get_bmtable_maxitems(ob);
	}

	return -1;
}


void FormMathsBitmap::apply()
{
	string::size_type const i = latex_chosen_.find(' ');
	if (i != string::npos) {
		controller().dispatchFunc(LFUN_MATH_MODE);
		controller().dispatchInsert(latex_chosen_.substr(0,i));
		controller().dispatchInsert('\\' + latex_chosen_.substr(i + 1));
	} else
		controller().dispatchInsert(latex_chosen_);
}


ButtonPolicy::SMInput FormMathsBitmap::input(FL_OBJECT * ob, long)
{
	int const i = GetIndex(ob);

	if (i < 0)
		return ButtonPolicy::SMI_INVALID;

	latex_chosen_ = latex_[i];
	apply();
	return ButtonPolicy::SMI_VALID;
}
