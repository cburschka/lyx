/**
 * \file FormBox.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna (Minipage stuff)
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormBox.h"
#include "ControlBox.h"
#include "forms/form_box.h"
#include "controllers/ButtonPolicies.h"
#include "controllers/helper_funcs.h"
#include "support/lstrings.h"
#include "support/tostr.h"

#include "Tooltips.h"
#include "xforms_helpers.h"
#include "xformsBC.h"

#include "insets/insetbox.h"

#include "lyx_forms.h"
#include "debug.h"

#include <vector>

using lyx::support::getStringFromVector;
using lyx::support::isStrDbl;
using lyx::support::subst;
using std::string;


typedef FormController<ControlBox, FormView<FD_box> > base_class;

FormBox::FormBox(Dialog & parent)
	: base_class(parent, _("Box"))
{}


void FormBox::build()
{
	dialog_.reset(build_box(this));

	box_gui_tokens(ids_, gui_names_);

	for (unsigned int i = 0; i < gui_names_.size(); ++i) {
		fl_addto_choice(dialog_->choice_type, gui_names_[i].c_str());
	}

	string str = _("Frameless: No border\n"
	               "Boxed: Rectangular\n"
	               "ovalbox: Oval, thin border\n"
	               "Ovalbox: Oval, thick border\n"
	               "Shadowbox: Box casting shadow\n"
	               "Doublebox: Double line border");
	tooltips().init(dialog_->choice_type, str);

	bcview().addReadOnly(dialog_->check_inner_box);

	str = _("The inner box may be a parbox or a minipage,\n"
	        "with appropriate arguments from this dialog.");
	tooltips().init(dialog_->check_inner_box, str);

	bcview().addReadOnly(dialog_->radio_parbox);
	bcview().addReadOnly(dialog_->radio_minipage);

	bcview().addReadOnly(dialog_->choice_pos);
	fl_addto_choice(dialog_->choice_pos, _("Top").c_str());
	fl_addto_choice(dialog_->choice_pos, _("Middle").c_str());
	fl_addto_choice(dialog_->choice_pos, _("Bottom").c_str());

	bcview().addReadOnly(dialog_->choice_inner_pos);
	fl_addto_choice(dialog_->choice_inner_pos, _("Top").c_str());
	fl_addto_choice(dialog_->choice_inner_pos, _("Middle").c_str());
	fl_addto_choice(dialog_->choice_inner_pos, _("Bottom").c_str());
	fl_addto_choice(dialog_->choice_inner_pos, _("Stretch").c_str());

	bcview().addReadOnly(dialog_->choice_hor_pos);
	fl_addto_choice(dialog_->choice_hor_pos, _("Left").c_str());
	fl_addto_choice(dialog_->choice_hor_pos, _("Center").c_str());
	fl_addto_choice(dialog_->choice_hor_pos, _("Right").c_str());
	fl_addto_choice(dialog_->choice_hor_pos, _("Stretch").c_str());

	bcview().addReadOnly(dialog_->input_width);
	bcview().addReadOnly(dialog_->choice_width_unit);
	bcview().addReadOnly(dialog_->choice_special);

	box_gui_tokens_special_length(ids_spec_, gui_names_spec_);
	for (unsigned int i = 0; i < gui_names_spec_.size(); ++i) {
		fl_addto_choice(dialog_->choice_special, gui_names_spec_[i].c_str());
	}

	string choice = getStringFromVector(getLatexUnits(), "|");
	fl_addto_choice(dialog_->choice_width_unit,
		subst(choice, "%", "%%").c_str());

	bcview().addReadOnly(dialog_->input_height);
	bcview().addReadOnly(dialog_->choice_height_unit);
	bcview().addReadOnly(dialog_->choice_height_special);
	for (unsigned int i = 0; i < gui_names_spec_.size(); ++i) {
		fl_addto_choice(dialog_->choice_height_special,
			gui_names_spec_[i].c_str());
	}

	choice = getStringFromVector(getLatexUnits(), "|");
	fl_addto_choice(dialog_->choice_height_unit,
		subst(choice, "%", "%%").c_str());

	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_cancel);
}


void FormBox::update()
{
	string type(controller().params().type);
	for (unsigned int i = 0; i < gui_names_.size(); ++i) {
		if (type == ids_[i])
			fl_set_choice_text(dialog_->choice_type, gui_names_[i].c_str());
		}

	fl_set_button(dialog_->check_inner_box, controller().params().inner_box);
	if (controller().params().type == "Frameless")
		setEnabled(dialog_->check_inner_box, false);

	char c = controller().params().pos;
	fl_set_choice(dialog_->choice_pos, string("tcb").find(c, 0) + 1);
	c = controller().params().inner_pos;
	fl_set_choice(dialog_->choice_inner_pos, string("tcbs").find(c, 0) + 1);
	c = controller().params().hor_pos;
	fl_set_choice(dialog_->choice_hor_pos, string("lcrs").find(c, 0) + 1);
	setEnabled(dialog_->choice_pos, controller().params().inner_box);
	setEnabled(dialog_->choice_inner_pos, controller().params().inner_box);
	setEnabled(dialog_->choice_hor_pos, !controller().params().inner_box);

	fl_set_button(dialog_->radio_parbox,    controller().params().use_parbox);
	fl_set_button(dialog_->radio_minipage, !controller().params().use_parbox);
	setEnabled(dialog_->radio_parbox, controller().params().inner_box);
	setEnabled(dialog_->radio_minipage, controller().params().inner_box);

	LyXLength len(controller().params().width);
	fl_set_input(dialog_->input_width, tostr(len.value()).c_str());
	fl_set_choice(dialog_->choice_width_unit, len.unit() + 1);
	string special(controller().params().special);
	for (unsigned int i = 0; i < gui_names_spec_.size(); ++i) {
		if (special == ids_spec_[i])
			fl_set_choice_text(dialog_->choice_special,
				gui_names_spec_[i].c_str());
		}
	// Special width unit must be default for general units to be enabled
	if(controller().params().special != "none")
		setEnabled(dialog_->choice_width_unit, false);
	setEnabled(dialog_->choice_special, !controller().params().inner_box);

	LyXLength ht(controller().params().height);
	fl_set_input(dialog_->input_height, tostr(ht.value()).c_str());
	fl_set_choice(dialog_->choice_height_unit, ht.unit() + 1);
	string const height_special(controller().params().height_special);
	for (unsigned int i = 0; i < gui_names_spec_.size(); ++i) {
		if (height_special == ids_spec_[i])
			fl_set_choice_text(dialog_->choice_height_special,
				gui_names_spec_[i].c_str());
		}
	setEnabled(dialog_->input_height, controller().params().inner_box);
	setEnabled(dialog_->choice_height_unit, controller().params().inner_box);
	setEnabled(dialog_->choice_height_special, controller().params().inner_box);
	// Same here
	if(height_special != "none")
		setEnabled(dialog_->choice_height_unit, false);
}


void FormBox::apply()
{
	int i = fl_get_choice(dialog_->choice_type);
	controller().params().type = ids_[i - 1];

	controller().params().inner_box = fl_get_button(dialog_->check_inner_box);
	controller().params().use_parbox =
		fl_get_button(dialog_->radio_parbox);
	controller().params().width =
		LyXLength(getLengthFromWidgets(dialog_->input_width,
		dialog_->choice_width_unit));

	controller().params().pos =
		"tcb"[fl_get_choice(dialog_->choice_pos) - 1];
	controller().params().inner_pos =
		"tcbs"[fl_get_choice(dialog_->choice_inner_pos) - 1];
	controller().params().hor_pos =
		"lcrs"[fl_get_choice(dialog_->choice_hor_pos) - 1];

	i = fl_get_choice(dialog_->choice_special);
	controller().params().special = ids_spec_[i - 1];

	controller().params().height =
		LyXLength(getLengthFromWidgets(dialog_->input_height,
		dialog_->choice_height_unit));
	i = fl_get_choice(dialog_->choice_height_special);
	controller().params().height_special = ids_spec_[i - 1];
}

ButtonPolicy::SMInput FormBox::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->check_inner_box) {
		setEnabled(dialog_->choice_pos,
			fl_get_button(dialog_->check_inner_box));
		setEnabled(dialog_->radio_parbox,
			fl_get_button(dialog_->check_inner_box));
		setEnabled(dialog_->radio_minipage,
			fl_get_button(dialog_->check_inner_box));
		setEnabled(dialog_->choice_width_unit, true);
		setEnabled(dialog_->choice_special,
			!fl_get_button(dialog_->check_inner_box));
		setEnabled(dialog_->input_height,
			fl_get_button(dialog_->check_inner_box));
		setEnabled(dialog_->choice_height_unit,
			fl_get_button(dialog_->check_inner_box));
		setEnabled(dialog_->choice_height_special,
			fl_get_button(dialog_->check_inner_box));
		setEnabled(dialog_->choice_hor_pos,
			!fl_get_button(dialog_->check_inner_box));
		setEnabled(dialog_->choice_inner_pos,
			fl_get_button(dialog_->check_inner_box));

		// Reset to defaults to not confuse users:
		if (fl_get_button(dialog_->check_inner_box))
			fl_set_choice(dialog_->choice_special, NONE);
		else
			fl_set_choice(dialog_->choice_height_special, NONE);
	}

	if (ob == dialog_->choice_special || ob == dialog_->check_inner_box)
		setEnabled(dialog_->choice_width_unit,
			fl_get_choice(dialog_->choice_special) == NONE);
	if (ob == dialog_->choice_height_special || ob == dialog_->check_inner_box)
		if (fl_get_choice(dialog_->choice_height_special) != NONE)
		    setEnabled(dialog_->choice_height_unit, false);
		else
		    setEnabled(dialog_->choice_height_unit,
				fl_get_button(dialog_->check_inner_box));

	// An inner box (parbox, minipage) is mandatory if no outer box
	if (ob == dialog_->choice_type) {
		int i = fl_get_choice(dialog_->choice_type);
		if (ids_[i - 1] == "Frameless") {
			setEnabled(dialog_->check_inner_box, false);
			fl_set_button(dialog_->check_inner_box, true);
		} else {
			setEnabled(dialog_->check_inner_box, true);
		}
	}
	// disallow senseless data, warnings if input is senseless
	if (ob == dialog_->input_width) {
		string const input = getString(dialog_->input_width);
		bool const invalid = !isValidLength(input) && !isStrDbl(input);
		if (invalid) {
			postWarning(_("Invalid length!"));
			return ButtonPolicy::SMI_INVALID;
		}
	}
	if (ob == dialog_->input_height) {
		string const input = getString(dialog_->input_height);
		bool const invalid = !isValidLength(input) && !isStrDbl(input);
		if (invalid) {
			postWarning(_("Invalid length!"));
			return ButtonPolicy::SMI_INVALID;
		}
	}
	if (ob == dialog_->button_defaults) {
		fl_set_button(dialog_->check_inner_box, true);
		fl_set_button(dialog_->radio_parbox, false);
		fl_set_input(dialog_->input_width, "100");
		fl_set_choice(dialog_->choice_width_unit, LyXLength::PCW + 1);
		fl_set_choice(dialog_->choice_special, NONE);
		fl_set_input(dialog_->input_height, "1");
		fl_set_choice(dialog_->choice_height_special, TOTALHEIGHT);
	}

	return ButtonPolicy::SMI_VALID;
}
