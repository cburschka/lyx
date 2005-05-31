/**
 * \file xforms/checkedwidgets.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "checkedwidgets.h"
#include "xforms_helpers.h"

#include "gettext.h"
#include "lyxgluelength.h"
#include "lyxrc.h"

#include "frontends/Alert.h"
#include "frontends/controllers/Kernel.h"

#include "support/lstrings.h"

#include "lyx_forms.h"

#include <sstream>


using lyx::support::isStrDbl;
using std::string;

namespace lyx {
namespace frontend {


CheckedLyXLength &
addCheckedLyXLength(BCView & bcview, FL_OBJECT * input, FL_OBJECT * label)
{
	CheckedLyXLength * cw(new CheckedLyXLength(input, label));
	bcview.addCheckedWidget(cw);
	return *cw;
}


CheckedGlueLength &
addCheckedGlueLength(BCView & bcview, FL_OBJECT * input, FL_OBJECT * label)
{
	CheckedGlueLength * cw(new CheckedGlueLength(input, label));
	bcview.addCheckedWidget(cw);
	return *cw;
}


CheckedPath &
addCheckedPath(BCView & bcview, bool acceptable_if_empty,
	       FL_OBJECT * input, FL_OBJECT * label)
{
	CheckedPath * cw(new CheckedPath(acceptable_if_empty, input, label));
	bcview.addCheckedWidget(cw);
	return *cw;
}


namespace {

void setWidget(bool valid, FL_OBJECT * input, FL_OBJECT * label)
{
	// define color to mark invalid input
	FL_COLOR const alert_col = FL_RED;

	FL_COLOR const lcol = valid ? FL_COLOR(FL_LCOL) : alert_col;
	if (label->lcol != lcol && isActive(label)) {
		fl_set_object_lcol(label, lcol);
	}
	if (input->lcol != lcol && isActive(input)) {
		fl_set_object_lcol(input, lcol);
	}

	// Reflect the validity of the data in the background color of the
	// input widget only when this widget is not being edited.
	FL_COLOR const icol1 = valid ? FL_COLOR(FL_COL1) : alert_col;
	if (input->col1 != icol1) {
		fl_set_object_color(input, icol1, FL_MCOL);
	}
}

} // namespace anon


CheckedLyXLength::CheckedLyXLength(FL_OBJECT * input, FL_OBJECT * label)
	: input_(input), label_(label ? label : input)
{
	BOOST_ASSERT(input && input->objclass == FL_INPUT);
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
	BOOST_ASSERT(input && input->objclass == FL_INPUT);
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


CheckedPath::CheckedPath(bool acceptable_if_empty,
			 FL_OBJECT * input, FL_OBJECT * label)
	: input_(input), label_(label ? label : input),
	  acceptable_if_empty_(acceptable_if_empty),
	  latex_doc_(false),
	  tex_allows_spaces_(false)
{}


void CheckedPath::setChecker(lyx::frontend::KernelDocType const & type,
			     LyXRC const & lyxrc)
{
	latex_doc_ = type == lyx::frontend::Kernel::LATEX;
	tex_allows_spaces_ = lyxrc.tex_allows_spaces;
}


namespace {

string const printable_list(string const & invalid_chars)
{
	std::ostringstream ss;
	string::const_iterator const begin = invalid_chars.begin();
	string::const_iterator const end = invalid_chars.end();
	string::const_iterator it = begin;

	for (; it != end; ++it) {
		if (it != begin)
			ss << ", ";
		if (*it == ' ')
			ss << _("space");
		else
			ss << *it;
	}

	return ss.str();
}

} // namespace anon


bool CheckedPath::check() const
{
	if (!latex_doc_) {
		setWidget(true, input_, label_);
		return true;
	}

	if (!isActive(input_)) {
		setWidget(true, input_, label_);
		return true;
	}

	string const text = getString(input_);
	if (text.empty()) {
		setWidget(acceptable_if_empty_, input_, label_);
		return acceptable_if_empty_;
	}

	string invalid_chars("#$%{}()[]\"^");
	if (!tex_allows_spaces_)
		invalid_chars += ' ';

	bool valid = true;
	if (text.find_first_of(invalid_chars) != string::npos) {

		static int counter = 0;
		if (counter == 0) {
			Alert::error(_("Invalid filename"),
				     _("LyX does not provide LateX support for file names containing any of these characters:\n") +
				     printable_list(invalid_chars));
		}
		++counter;
		valid = false;
	}

	// set the color of label and input widget
	setWidget(valid, input_, label_);
	return valid;
}

} // namespace frontend
} // namespace lyx
