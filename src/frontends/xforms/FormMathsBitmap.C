/**
 * \file FormMathsBitmap.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>
#include <algorithm>
#include <iomanip>

#include XPM_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormMathsBitmap.h"

#include "Dialogs.h"
#include "LyXView.h"
#include "bmtable.h"
#include "debug.h"
#include "xforms_helpers.h"
#include "gettext.h"
#include "support/LAssert.h"

using std::vector;
using std::endl;
using std::setw;
using std::max;

extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

FormMathsBitmap::FormMathsBitmap(LyXView * lv, Dialogs * d,
				 FormMathsPanel const & p, string const & t,
				 vector<string> const & l)
	: FormMathsSub(lv, d, p, t, false),
	  latex_(l), form_(0), ww_(0), x_(0), y_(0), w_(0), h_(0)
{
	ww_ = 2 * FL_abs(FL_BOUND_WIDTH);
	x_ = y_ = ww_;
	y_ += 8;
}


FormMathsBitmap::~FormMathsBitmap()
{
	if (!form())
		return;

	if (form()->visible) fl_hide_form(form());
	fl_free_form(form());
}


FL_FORM * FormMathsBitmap::form() const
{
	return form_.get();
}


void FormMathsBitmap::build()
{
	lyx::Assert(bitmaps_.size() > 0);

	h_+= 50; // Allow room for a Close button

	form_.reset(fl_bgn_form(FL_UP_BOX, w_, h_));
	form_->u_vdata = this;

	fl_add_box(FL_UP_BOX, 0, 0, w_, h_, "");

	y_ = 0;
	for (vector<bm_ptr>::const_iterator it = bitmaps_.begin();
	     it < bitmaps_.end(); ++it) {
		FL_OBJECT * obj = it->get();

		fl_add_object(form_.get(), obj);
		bc().addReadOnly(obj);

		y_ = max(y_, obj->y + obj->h);
	}

	char const * const label = N_("Close|^[");
	x_ = (form_->w - 90) / 2;
	y_ += 10;

	FL_OBJECT * button_close =
		fl_add_button(FL_NORMAL_BUTTON, x_, y_, 90, 30, idex(_(label)));
	fl_set_button_shortcut(button_close, scex(_(label)), 1);
	fl_set_object_lsize(button_close, FL_NORMAL_SIZE);
	fl_set_object_callback(button_close, C_FormBaseDeprecatedCancelCB, 0);

	fl_end_form();

	bc().setCancel(button_close);
}


void FormMathsBitmap::addBitmap(int nt, int nx, int ny, int bw, int bh,
				unsigned char const * data, bool vert)
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
	int wx = bw + ww_ / 2;
	int wy = bh + ww_ / 2;
	wx += (wx % nx);
	wy += (wy % ny);
	FL_OBJECT * obj = fl_create_bmtable(1, x_, y_, wx, wy, "");
	fl_set_object_lcol(obj, FL_BLUE);
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_bmtable_data(obj, nx, ny, bw, bh, data);
	fl_set_bmtable_maxitems(obj, nt);
	fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);

	if (vert) {
		y_ += wy + 8;
		h_ = max(y_, h_);
		w_ = max(x_ + wx + ww_, w_);
	} else  {
		x_ += wx + 8;
		w_ = max(x_, w_);
		h_ = max(y_ + wy + ww_, h_);
	}

	bitmaps_.push_back(bm_ptr(obj));
}


int FormMathsBitmap::GetIndex(FL_OBJECT * ob)
{
	int k = 0;
	for (vector<bm_ptr>::const_iterator it = bitmaps_.begin();
	     it < bitmaps_.end(); ++it) {
		if (it->get() == ob)
			return k + fl_get_bmtable(ob);
		else
			k += fl_get_bmtable_maxitems(it->get());
	}
	return -1;
}


void FormMathsBitmap::apply()
{
	string::size_type const i = latex_chosen_.find(' ');
	if (i != string::npos) {
		parent_.dispatchFunc(LFUN_MATH_MODE);
		parent_.insertSymbol(latex_chosen_.substr(0,i));
		parent_.insertSymbol(latex_chosen_.substr(i + 1), false);
	} else
		parent_.insertSymbol(latex_chosen_);
}


bool FormMathsBitmap::input(FL_OBJECT * ob, long)
{
	int const i = GetIndex(ob);

	if (i < 0)
		return false;

	latex_chosen_ = latex_[i];
	apply();
	return true;
}
