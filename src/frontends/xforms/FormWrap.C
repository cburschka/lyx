/**
 * \file FormWrap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "xformsBC.h"
#include "ControlWrap.h"
#include "FormWrap.h"
#include "forms/form_wrap.h"
#include "Tooltips.h"

#include "helper_funcs.h"
#include "xforms_helpers.h"
#include "checkedwidgets.h"

#include "insets/insetwrap.h"
#include "support/lstrings.h"
#include "lyx_forms.h"
#include "support/tostr.h"

using lyx::support::getStringFromVector;
using lyx::support::subst;

using std::string;


typedef FormController<ControlWrap, FormView<FD_wrap> > base_class;

FormWrap::FormWrap(Dialog & parent)
	: base_class(parent, _("Text Wrap Settings"))
{}


void FormWrap::build()
{
	dialog_.reset(build_wrap(this));

	// Manage the ok, apply and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
	bcview().setRestore(dialog_->button_restore);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->input_width);
	bcview().addReadOnly(dialog_->choice_width_units);

	// check validity of "length + unit" input
	addCheckedGlueLength(bcview(), dialog_->input_width);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_width);

	fl_set_input_return(dialog_->input_width, FL_RETURN_CHANGED);

	string const choice = getStringFromVector(getLatexUnits(), "|");
	fl_addto_choice(dialog_->choice_width_units,
			subst(choice, "%", "%%").c_str());

	placement_.init(dialog_->radio_default, 0); // default
	placement_.init(dialog_->radio_outer, 'p');
	placement_.init(dialog_->radio_left,  'l');
	placement_.init(dialog_->radio_right, 'r');

	// set up the tooltips
	string str = _("Enter width for the float.");
	tooltips().init(dialog_->input_width, str);
	str = _("Forces float to the right in a paragraph if the page number "
		"is odd, and to the left if page number is even.");
	tooltips().init(dialog_->radio_default, str);
	str = _("Forces float to the left in a paragraph if the pagenumber "
		"is odd, and to the right if page number is even.");
	tooltips().init(dialog_->radio_outer, str);
	str = _("Forces float to the left in the paragraph.");
	tooltips().init(dialog_->radio_left, str);
	str = _("Forces float to the right in the paragraph.");
	tooltips().init(dialog_->radio_right, str);
}


void FormWrap::apply()
{
	InsetWrapParams & params = controller().params();

	params.width =
		LyXLength(getLengthFromWidgets(dialog_->input_width,
					       dialog_->choice_width_units));

	char const c = static_cast<char>(placement_.get());
	if (c)
		params.placement = c;
	else
		params.placement.erase(); // default
}


void FormWrap::update()
{
	InsetWrapParams const & params = controller().params();
	LyXLength len(params.width);
	fl_set_input(dialog_->input_width, tostr(len.value()).c_str());
	fl_set_choice(dialog_->choice_width_units, len.unit() + 1);

	if (params.placement.empty())
		placement_.set(dialog_->radio_default); // default
	else
		placement_.set(params.placement.c_str()[0]);
}
