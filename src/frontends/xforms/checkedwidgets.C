/**
 * \file checkedwidgets.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "checkedwidgets.h"
#include "xforms_helpers.h"
#include "lyxlength.h"
#include "lyxgluelength.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "LString.h"

#include FORMS_H_LOCATION


void addCheckedLyXLength(ButtonControllerBase & bc,
			 FL_OBJECT * input, FL_OBJECT * label)
{
	bc.addCheckedWidget(new CheckedLyXLength(input, label));
}


void addCheckedGlueLength(ButtonControllerBase & bc,
			  FL_OBJECT * input, FL_OBJECT * label)
{
	bc.addCheckedWidget(new CheckedGlueLength(input, label));
}


namespace {

void setWidget(bool valid, FL_OBJECT * input, FL_OBJECT * label)
{
	// define color to mark invalid input
	FL_COLOR const alert_col = FL_RED;
	
	FL_COLOR const lcol = valid ? FL_LCOL : alert_col;
	if (label->lcol != lcol && isActive(label)) {
		fl_set_object_lcol(label, lcol);
	}
	if (input->lcol != lcol && isActive(input)) {
		fl_set_object_lcol(input, lcol);
	}

	// set background color of input widget
	FL_COLOR const icol1 = valid ? FL_INPUT_COL1 : alert_col;
	FL_COLOR const icol2 = valid ? FL_INPUT_COL2 : alert_col;
	if (input->col1 != icol1 || input->col2 != icol2) {
		fl_set_object_color(input, icol1, icol2);
	}
}
 
} // namespace anon


CheckedLyXLength::CheckedLyXLength(FL_OBJECT * input, FL_OBJECT * label)
	: input_(input), label_(label ? label : input)
{
	lyx::Assert(input && input->objclass == FL_INPUT);
}


bool CheckedLyXLength::check() const
{
	string const str = getString(input_);
	bool const valid = !isActive(input_) || str.empty()
		|| isStrDbl(str) || isValidLength(str);

	// set the color of label and input widget
	setWidget(valid, input_, label_);

	return valid;
}


CheckedGlueLength::CheckedGlueLength(FL_OBJECT * input, FL_OBJECT * label)
	: input_(input), label_(label ? label : input)
{
	lyx::Assert(input && input->objclass == FL_INPUT);
}


bool CheckedGlueLength::check() const
{
	string const str = getString(input_);
	bool const valid = !isActive(input_) || str.empty()
		|| isStrDbl(str) || isValidGlueLength(str);

	// set the color of label and input widget
	setWidget(valid, input_, label_);

	return valid;
}
