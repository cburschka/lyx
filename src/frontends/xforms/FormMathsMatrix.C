/**
 * \file FormMathsMatrix.C
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

#include "FormMathsMatrix.h"
#include "forms/form_maths_matrix.h"
#include "ControlMath.h"
#include "xformsBC.h"

#include "support/LAssert.h"
#include "support/lyxalgo.h" // lyx::count

#include "Lsstream.h"

#include FORMS_H_LOCATION

#include <algorithm>

#ifndef CXX_GLOBAL_CSTD
using std::strlen;
#endif


static char h_align_str[80] = "c";
static char v_align_c[] = "tcb";


extern "C" {

	static
	int C_FormMathsMatrixAlignFilter(FL_OBJECT * ob, char const *,
					 char const * cur, int c)
	{
		lyx::Assert(ob);
		FormMathsMatrix * pre =
			static_cast<FormMathsMatrix *>(ob->u_vdata);
		lyx::Assert(pre);
		return pre->AlignFilter(cur, c);
	}

}


typedef FormCB<ControlMathSub, FormDB<FD_maths_matrix> > base_class;

FormMathsMatrix::FormMathsMatrix()
	: base_class(_("Maths Matrix"), false)
{}


void FormMathsMatrix::build()
{
	dialog_.reset(build_maths_matrix(this));

	fl_addto_choice(dialog_->choice_valign, _("Top | Center | Bottom"));
	fl_set_choice(dialog_->choice_valign, 2);
	fl_set_input(dialog_->input_halign, h_align_str);
	dialog_->input_halign->u_vdata = this;
	fl_set_input_filter(dialog_->input_halign,
			    C_FormMathsMatrixAlignFilter);
	setPrehandler(dialog_->input_halign);

	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->slider_rows);
	bc().addReadOnly(dialog_->slider_columns);
	bc().addReadOnly(dialog_->choice_valign);
	bc().addReadOnly(dialog_->input_halign);
}


void FormMathsMatrix::apply()
{
	char const c = v_align_c[fl_get_choice(dialog_->choice_valign) - 1];
	char const * sh = fl_get_input(dialog_->input_halign);
	int const nx = int(fl_get_slider_value(dialog_->slider_columns) + 0.5);
	int const ny = int(fl_get_slider_value(dialog_->slider_rows) + 0.5);

	ostringstream os;
	os << nx << ' ' << ny << ' ' << c << ' ' << sh;
	controller().dispatchFunc(LFUN_INSERT_MATRIX, STRCONV(os.str()));
}

			      
void FormMathsMatrix::update()
{
	bc().valid();
}


ButtonPolicy::SMInput FormMathsMatrix::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->choice_valign ||
	    ob == dialog_->slider_rows) return ButtonPolicy::SMI_VALID;

	int const nx = int(fl_get_slider_value(dialog_->slider_columns)+0.5);
	for (int i = 0; i < nx; ++i)
		h_align_str[i] = 'c';

	h_align_str[nx] = '\0';

	fl_set_input(dialog_->input_halign, h_align_str);
	fl_redraw_object(dialog_->input_halign);
	return ButtonPolicy::SMI_VALID;
}


int FormMathsMatrix::AlignFilter(char const * cur, int c)
{
	size_t const len = strlen(cur);

	int const n = int(fl_get_slider_value(dialog_->slider_columns) + 0.5)
		- int(len)
		+ int(lyx::count(cur, cur + len, '|'));
	if (n < 0)
		return FL_INVALID;

	if (c == 'c' || c == 'l' || c == 'r' || c == '|')
		return FL_VALID;

	return FL_INVALID;
}
