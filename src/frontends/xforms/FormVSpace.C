/**
 * \file FormVSpace.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Jürgen Vigna
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormVSpace.h"
#include "ControlVSpace.h"
#include "forms/form_vspace.h"

#include "checkedwidgets.h"
#include "input_validators.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include "xformsBC.h"

#include "controllers/helper_funcs.h"

#include "lyxrc.h" // to set the deafult length values
#include "Spacing.h"
#include "vspace.h"

#include "support/lstrings.h"
#include "support/tostr.h"

#include "lyx_forms.h"

#include <boost/bind.hpp>

using boost::bind;

using std::remove_if;

using std::vector;
using std::string;

namespace lyx {

using support::contains;
using support::getStringFromVector;
using support::rtrim;

namespace frontend {

namespace {

string defaultUnit("cm");

void validateVSpaceWidgets(FL_OBJECT * choice_type, FL_OBJECT * input_length)
{
	// Paranoia check!
	BOOST_ASSERT(choice_type  && choice_type->objclass  == FL_CHOICE &&
		     input_length && input_length->objclass == FL_INPUT);

	if (fl_get_choice(choice_type) != 6)
		return;

	// If a vspace kind is "Length" but there's no text in
	// the input field, insert nothing.
	string const input = rtrim(getString(input_length));
	if (input.empty())
		return;
}


VSpace const setVSpaceFromWidgets(FL_OBJECT * choice_type,
				  FL_OBJECT * input_length,
				  FL_OBJECT * choice_length,
				  bool keep)
{
	// Paranoia check!
	BOOST_ASSERT(choice_type   && choice_type->objclass   == FL_CHOICE &&
		     input_length  && input_length->objclass  == FL_INPUT &&
		     choice_length && choice_length->objclass == FL_CHOICE);

	VSpace space;

	switch (fl_get_choice(choice_type)) {
	case 1:
		space = VSpace(VSpace::DEFSKIP);
		break;
	case 2:
		space = VSpace(VSpace::SMALLSKIP);
		break;
	case 3:
		space = VSpace(VSpace::MEDSKIP);
		break;
	case 4:
		space = VSpace(VSpace::BIGSKIP);
		break;
	case 5:
		space = VSpace(VSpace::VFILL);
		break;
	case 6:
		space = VSpace(LyXGlueLength(
			getLengthFromWidgets(input_length, choice_length)));
		break;
	}

	space.setKeep(keep);
	return space;
}


void setWidgetsFromVSpace(VSpace const & space,
			  FL_OBJECT * choice_type,
			  FL_OBJECT * input_length,
			  FL_OBJECT * choice_length,
			  FL_OBJECT * check_keep)
{
	// Paranoia check!
	BOOST_ASSERT(choice_type   && choice_type->objclass   == FL_CHOICE &&
		     input_length  && input_length->objclass  == FL_INPUT &&
		     choice_length && choice_length->objclass == FL_CHOICE &&
		     check_keep    && check_keep->objclass   == FL_CHECKBUTTON);

	int pos = 1;
	switch (space.kind()) {
	case VSpace::DEFSKIP:
		pos = 1;
		break;
	case VSpace::SMALLSKIP:
		pos = 2;
		break;
	case VSpace::MEDSKIP:
		pos = 3;
		break;
	case VSpace::BIGSKIP:
		pos = 4;
		break;
	case VSpace::VFILL:
		pos = 5;
		break;
	case VSpace::LENGTH:
		pos = 6;
		break;
	}
	fl_set_choice(choice_type, pos);
	fl_set_button(check_keep, space.keep());

	bool const custom_vspace = space.kind() == VSpace::LENGTH;
	if (custom_vspace) {
		string const length = space.length().asString();
		updateWidgetsFromLengthString(input_length, choice_length,
					      length, defaultUnit);
	} else {
		fl_set_input(input_length, "");
		fl_set_choice_text(choice_length, defaultUnit.c_str());
	}
}

} // namespace anon


typedef FormController<ControlVSpace, FormView<FD_vspace> > base_class;

FormVSpace::FormVSpace(Dialog & parent)
	: base_class(parent, _("VSpace Settings"))
{}


void FormVSpace::build()
{
	// the tabbed folder
	dialog_.reset(build_vspace(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
	bcview().setRestore(dialog_->button_restore);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->choice_space);
	bcview().addReadOnly(dialog_->input_space);
	bcview().addReadOnly(dialog_->choice_unit_space);

	// check validity of "length + unit" input.
	// If invalid, the label of input_space is displayed in red.
	addCheckedGlueLength(bcview(),
			     dialog_->input_space,
			     dialog_->input_space);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_space);

	fl_set_input_return(dialog_->input_space, FL_RETURN_CHANGED);

	string const spacing =
		_("DefSkip|SmallSkip|MedSkip|BigSkip|VFill|Length");
	fl_addto_choice(dialog_->choice_space, spacing.c_str());

	// Create the contents of the unit choices; don't include the "%" terms.
	vector<string> units_vec = getLatexUnits();
	vector<string>::iterator del =
		remove_if(units_vec.begin(), units_vec.end(),
			  bind(contains<char>, _1, '%'));
	units_vec.erase(del, units_vec.end());

	string const units = getStringFromVector(units_vec, "|");
	fl_addto_choice(dialog_->choice_unit_space, units.c_str());

	// set up the tooltips
	string str = _("Additional vertical space.");
	tooltips().init(dialog_->choice_space, str);

	// set default unit for custom length
	switch (lyxrc.default_papersize) {
	case PAPER_DEFAULT:
	case PAPER_USLETTER:
	case PAPER_LEGALPAPER:
	case PAPER_EXECUTIVEPAPER:
		defaultUnit = "in";
		break;
	case PAPER_A3PAPER:
	case PAPER_A4PAPER:
	case PAPER_A5PAPER:
	case PAPER_B5PAPER:
		defaultUnit = "cm";
		break;
	}
}


void FormVSpace::apply()
{
	// If a vspace choice is "Length" but there's no text in
	// the input field, insert nothing.
	validateVSpaceWidgets(dialog_->choice_space, dialog_->input_space);

	VSpace const space =
		setVSpaceFromWidgets(dialog_->choice_space,
				     dialog_->input_space,
				     dialog_->choice_unit_space,
				     fl_get_button(dialog_->check_keep));

	controller().params() = space;
}


void FormVSpace::update()
{
	setWidgetsFromVSpace(controller().params(),
			     dialog_->choice_space,
			     dialog_->input_space,
			     dialog_->choice_unit_space,
			     dialog_->check_keep);

	bool const custom_length =
		fl_get_choice(dialog_->choice_space) == 6;
	setEnabled(dialog_->input_space, custom_length);
	setEnabled(dialog_->choice_unit_space, custom_length);
}


ButtonPolicy::SMInput FormVSpace::input(FL_OBJECT * ob, long)
{
	// Enable input when custom length is chosen,
	// disable 'keep' when no space is chosen
	if (ob == dialog_->choice_space) {
		bool const custom_length =
			fl_get_choice(dialog_->choice_space) == 6;
		setEnabled(dialog_->input_space, custom_length);
		setEnabled(dialog_->choice_unit_space, custom_length);
	}
	return ButtonPolicy::SMI_VALID;
}

} // namespace frontend
} // namespace lyx
