/**
 * \file FormParagraph.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION

#include "FormParagraph.h"
#include "form_paragraph.h"
#include "ControlParagraph.h"
#include "xforms_helpers.h"
#include "lyxrc.h" // to set the deafult length values
#include "input_validators.h"
#include "helper_funcs.h"
#include "gettext.h"
#include "xformsBC.h"
#include "layout.h" // LyXAlignment

#include "support/lstrings.h"
#include "support/LAssert.h"

#include <functional>

using std::vector;
using std::bind2nd;
using std::remove_if;

typedef FormCB<ControlParagraph, FormDB<FD_form_paragraph> > base_class;
  
FormParagraph::FormParagraph(ControlParagraph & c)
	: base_class(c, _("Paragraph Layout"), false)
{}

void FormParagraph::build()
{
	// the tabbed folder
	dialog_.reset(build_paragraph());

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	fl_addto_choice(dialog_->choice_space_above,
			_(" None | Defskip | Smallskip "
			  "| Medskip | Bigskip | VFill | Length "));
	fl_addto_choice(dialog_->choice_space_below,
			_(" None | Defskip | Smallskip "
			  "| Medskip | Bigskip | VFill | Length "));

	fl_addto_choice(dialog_->choice_linespacing,
			_(" Default | Single | OneHalf | Double | Custom "));

	fl_set_input_return(dialog_->input_space_above, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_space_below, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_labelwidth,  FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_linespacing, FL_RETURN_CHANGED);
	fl_set_input_filter(dialog_->input_linespacing, fl_unsigned_float_filter);

	setPrehandler(dialog_->input_space_above);
	setPrehandler(dialog_->input_space_below);
	setPrehandler(dialog_->input_labelwidth);
	setPrehandler(dialog_->input_linespacing);

	// Create the contents of the unit choices
	// Don't include the "%" terms...
	vector<string> units_vec = getLatexUnits();
#if 0
	for (vector<string>::iterator it = units_vec.begin();
	     it != units_vec.end(); ++it) {
		if (contains(*it, "%"))
			it = units_vec.erase(it, it+1) - 1;
	}
#else
	// Something similar to this is a better way to erase
	vector<string>::iterator del =
		remove_if(units_vec.begin(), units_vec.end(),
			  bind2nd(contains_functor(), "%"));
	units_vec.erase(del, units_vec.end());
#endif

	string units = getStringFromVector(units_vec, "|");

	fl_addto_choice(dialog_->choice_value_space_above, units.c_str());
	fl_addto_choice(dialog_->choice_value_space_below, units.c_str());

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->radio_align_right);
	bc().addReadOnly(dialog_->radio_align_left);
	bc().addReadOnly(dialog_->radio_align_block);
	bc().addReadOnly(dialog_->radio_align_center);
	bc().addReadOnly(dialog_->check_lines_top);
	bc().addReadOnly(dialog_->check_lines_bottom);
	bc().addReadOnly(dialog_->check_pagebreaks_top);
	bc().addReadOnly(dialog_->check_pagebreaks_bottom);
	bc().addReadOnly(dialog_->choice_space_above);
	bc().addReadOnly(dialog_->input_space_above);
	bc().addReadOnly(dialog_->check_space_above);
	bc().addReadOnly(dialog_->choice_space_below);
	bc().addReadOnly(dialog_->input_space_below);
	bc().addReadOnly(dialog_->check_space_below);
	bc().addReadOnly(dialog_->choice_linespacing);
	bc().addReadOnly(dialog_->input_linespacing);
	bc().addReadOnly(dialog_->check_noindent);
	bc().addReadOnly(dialog_->input_labelwidth);
}

namespace {

VSpace setVSpaceFromWidgets(FL_OBJECT * choice_type,
			    FL_OBJECT * input_length,
			    FL_OBJECT * choice_length,
			    FL_OBJECT * check_keep)
{
	// Paranoia check!
	lyx::Assert(choice_type   && choice_type->objclass   == FL_CHOICE &&
		    input_length  && input_length->objclass  == FL_INPUT &&
		    choice_length && choice_length->objclass == FL_CHOICE &&
		    check_keep    && check_keep->objclass    == FL_CHECKBUTTON);

	VSpace space;

	switch (fl_get_choice(choice_type)) {
	case 1:
		space = VSpace(VSpace::NONE);
		break;
	case 2:
		space = VSpace(VSpace::DEFSKIP);
		break;
	case 3:
		space = VSpace(VSpace::SMALLSKIP);
		break;
	case 4:
		space = VSpace(VSpace::MEDSKIP);
		break;
	case 5:
		space = VSpace(VSpace::BIGSKIP);
		break;
	case 6:
		space = VSpace(VSpace::VFILL);
		break;
	case 7:
	{
		string const length =
			getLengthFromWidgets(input_length, choice_length);
		space = VSpace(LyXGlueLength(length));
		break;
	}
	}

	if (fl_get_button(check_keep))
		space.setKeep(true);

	return space;
}

void validateVSpaceWidgets(FL_OBJECT * choice_type, FL_OBJECT * input_length)
{
	// Paranoia check!
	lyx::Assert(choice_type  && choice_type->objclass   == FL_CHOICE &&
		    input_length && input_length->objclass  == FL_INPUT);

	if (fl_get_choice(choice_type) != 7)
		return;

	// If a vspace kind is "Length" but there's no text in
	// the input field, reset the kind to "None".
	string const input = strip(getStringFromInput(input_length));
	if (input.empty())
		fl_set_choice(choice_type, 1);
}

} // namespace anon

void FormParagraph::apply()
{
	if (!form()) return;

	/* spacing */
	// If a vspace kind is "Length" but there's no text in
	// the input field, reset the kind to "None".
	validateVSpaceWidgets(dialog_->choice_space_above,
			      dialog_->input_space_above);
	
	VSpace const space_top =
		setVSpaceFromWidgets(dialog_->choice_space_above,
				     dialog_->input_space_above,
				     dialog_->choice_value_space_above,
				     dialog_->check_space_above);
	
	controller().params().spaceTop(space_top);
	
	validateVSpaceWidgets(dialog_->choice_space_below,
			      dialog_->input_space_below);

	VSpace const space_bottom =
		setVSpaceFromWidgets(dialog_->choice_space_below,
				     dialog_->input_space_below,
				     dialog_->choice_value_space_below,
				     dialog_->check_space_below);
	
	controller().params().spaceBottom(space_bottom);

	/* lines and pagebreaks */
	bool const line_top = fl_get_button(dialog_->check_lines_top);
	controller().params().lineTop(line_top);

	bool const line_bottom = fl_get_button(dialog_->check_lines_bottom);
	controller().params().lineBottom(line_bottom);

	bool const pagebreak_top = fl_get_button(dialog_->check_pagebreaks_top);
	controller().params().pagebreakTop(pagebreak_top);
	
	bool const pagebreak_bottom = fl_get_button(dialog_->check_pagebreaks_bottom);
	controller().params().pagebreakBottom(pagebreak_bottom);
	

	/* alignment */
	LyXAlignment align;
	if (fl_get_button(dialog_->radio_align_left))
		align = LYX_ALIGN_LEFT;
	else if (fl_get_button(dialog_->radio_align_right))
		align = LYX_ALIGN_RIGHT;
	else if (fl_get_button(dialog_->radio_align_center))
		align = LYX_ALIGN_CENTER;
	else
		align = LYX_ALIGN_BLOCK;
	controller().params().align(align);
	
	/* label width */
	string const labelwidthstring =
		getStringFromInput(dialog_->input_labelwidth);
	controller().params().labelWidthString(labelwidthstring);

	/* indendation */
	bool const noindent = fl_get_button(dialog_->check_noindent);
	controller().params().noindent(noindent);

	/* get spacing */
	Spacing::Space linespacing = Spacing::Default;
	string other;
	switch (fl_get_choice(dialog_->choice_linespacing)) {
	case 1:
		linespacing = Spacing::Default;
		break;
	case 2:
		linespacing = Spacing::Single;
		break;
	case 3:
		linespacing = Spacing::Onehalf;
		break;
	case 4:
		linespacing = Spacing::Double;
		break;
	case 5:
		linespacing = Spacing::Other;
		other = getStringFromInput(dialog_->input_linespacing);
		break;
	}

	Spacing const spacing(linespacing, other);
	controller().params().spacing(spacing);
	
}

namespace {

void setWidgetsFromVSpace(VSpace const & space,
			  FL_OBJECT * choice_type,
			  FL_OBJECT * input_length,
			  FL_OBJECT * choice_length,
			  FL_OBJECT * check_keep)
{
	// Paranoia check!
	lyx::Assert(choice_type   && choice_type->objclass   == FL_CHOICE &&
		    input_length  && input_length->objclass  == FL_INPUT &&
		    choice_length && choice_length->objclass == FL_CHOICE &&
		    check_keep    && check_keep->objclass    == FL_CHECKBUTTON);

	fl_set_input(input_length, "");
	setEnabled(input_length,  false);
	setEnabled(choice_length, false);

	switch (space.kind()) {
	case VSpace::NONE:
		fl_set_choice(choice_type, 1);
		break;
	case VSpace::DEFSKIP:
		fl_set_choice(choice_type, 2);
		break;
	case VSpace::SMALLSKIP:
		fl_set_choice(choice_type, 3);
		break;
	case VSpace::MEDSKIP:
		fl_set_choice(choice_type, 4);
		break;
	case VSpace::BIGSKIP:
		fl_set_choice(choice_type, 5);
		break;
	case VSpace::VFILL:
		fl_set_choice(choice_type, 6);
		break;
	case VSpace::LENGTH:
	{
		fl_set_choice(choice_type, 7);

		setEnabled(input_length,  true);
		setEnabled(choice_length, true);

		bool const metric = lyxrc.default_papersize > 3;
		string const default_unit = metric ? "cm" : "in";
		string const length = space.length().asString();

		updateWidgetsFromLengthString(input_length, choice_length,
					      length, default_unit);
		break;
	}
	}

	fl_set_button(check_keep, space.keep());
}

} // namespace anon

void FormParagraph::update()
{
	if (!dialog_.get())
		return;

	/* label width */
	string labelwidth = controller().params().labelWidthString();
	fl_set_input(dialog_->input_labelwidth, labelwidth.c_str());
	setEnabled(dialog_->input_labelwidth,
		   labelwidth != _("Senseless with this layout!"));

	/* alignment */
	fl_set_button(dialog_->radio_align_right, 0);
	fl_set_button(dialog_->radio_align_left, 0);
	fl_set_button(dialog_->radio_align_center, 0);
	fl_set_button(dialog_->radio_align_block, 0);
	
	LyXAlignment align = controller().params().align();

	switch (align) {
	case LYX_ALIGN_RIGHT:
		fl_set_button(dialog_->radio_align_right, 1);
		break;
	case LYX_ALIGN_LEFT:
		fl_set_button(dialog_->radio_align_left, 1);
		break;
	case LYX_ALIGN_CENTER:
		fl_set_button(dialog_->radio_align_center, 1);
		break;
	default:
		fl_set_button(dialog_->radio_align_block, 1);
		break;
	}

	LyXAlignment alignpos = controller().alignPossible();

	setEnabled(dialog_->radio_align_block,  bool(alignpos & LYX_ALIGN_BLOCK));
	setEnabled(dialog_->radio_align_center, bool(alignpos & LYX_ALIGN_CENTER));
	setEnabled(dialog_->radio_align_left,   bool(alignpos & LYX_ALIGN_LEFT));
	setEnabled(dialog_->radio_align_right,  bool(alignpos & LYX_ALIGN_RIGHT));

	// no inset-text-owned paragraph may have pagebreaks
	bool ininset = controller().inInset();
	setEnabled(dialog_->check_pagebreaks_top, !ininset);
	setEnabled(dialog_->check_pagebreaks_bottom, !ininset);

	/* lines, pagebreaks and indent */
	fl_set_button(dialog_->check_lines_top,
		      controller().params().lineTop());
	fl_set_button(dialog_->check_lines_bottom,
		      controller().params().lineBottom());
	fl_set_button(dialog_->check_pagebreaks_top,
		      controller().params().pagebreakTop());
	fl_set_button(dialog_->check_pagebreaks_bottom,
		      controller().params().pagebreakBottom());
	fl_set_button(dialog_->check_noindent,
		      controller().params().noindent());

	/* linespacing */
	int linespacing;
	Spacing const space = controller().params().spacing();

	switch (space.getSpace()) {
	default: linespacing = 1; break;
	case Spacing::Single: linespacing = 2; break;
	case Spacing::Onehalf: linespacing = 3; break;
	case Spacing::Double: linespacing = 4; break;
	case Spacing::Other: linespacing = 5; break;
	}

	fl_set_choice(dialog_->choice_linespacing, linespacing);
	if (space.getSpace() == Spacing::Other) {
		string const sp = tostr(space.getValue());
		fl_set_input(dialog_->input_linespacing, sp.c_str());
		setEnabled(dialog_->input_linespacing, true);
	} else {
		fl_set_input(dialog_->input_linespacing, "");
		setEnabled(dialog_->input_linespacing, false);
	}

	/* vspace top */
	setWidgetsFromVSpace(controller().params().spaceTop(),
			     dialog_->choice_space_above,
			     dialog_->input_space_above,
			     dialog_->choice_value_space_above,
			     dialog_->check_space_above);

	/* vspace bottom */
	setWidgetsFromVSpace(controller().params().spaceBottom(),
			     dialog_->choice_space_below,
			     dialog_->input_space_below,
			     dialog_->choice_value_space_below,
			     dialog_->check_space_below);

	/* no indent */
	fl_set_button(dialog_->check_noindent,
		      controller().params().noindent());
}

namespace {

void synchronizeSpaceWidgets(FL_OBJECT * choice_type,
			     FL_OBJECT * input_length,
			     FL_OBJECT * choice_length)
{
	// Paranoia check!
	lyx::Assert(choice_type   && choice_type->objclass   == FL_CHOICE &&
		    input_length  && input_length->objclass  == FL_INPUT &&
		    choice_length && choice_length->objclass == FL_CHOICE);

	if (fl_get_choice(choice_type) != 7) {
		fl_set_input(input_length, "");
		setEnabled(input_length, false);
		setEnabled(choice_length, false);

	} else {
		setEnabled(input_length, true);
		setEnabled(choice_length, true);

		string const length = getStringFromInput(input_length);

		if (strip(length).empty()) {
			bool const metric = lyxrc.default_papersize > 3;
			int const default_unit = metric ? 8 : 9;

			fl_set_choice(choice_length, default_unit);
		}
	}
}

bool validSpaceWidgets(FL_OBJECT * choice_type, FL_OBJECT * input_length)
{
	// Paranoia check!
	lyx::Assert(choice_type  && choice_type->objclass   == FL_CHOICE &&
		    input_length && input_length->objclass  == FL_INPUT);

	if (fl_get_choice(choice_type) != 7)
		return true;

	string const input = getStringFromInput(input_length);
	return (input.empty() ||
		isValidGlueLength(input) ||
		isStrDbl(input));
}

} // namespace anon

ButtonPolicy::SMInput FormParagraph::input(FL_OBJECT * ob, long)
{
	clearMessage();

	// First check the buttons which are exclusive and you have to
	// check only the actuall de/activated button.
	//
	// "Synchronize" the choices and input fields, making it
	// impossible to commit senseless data.
	if (ob == dialog_->choice_space_above) {
		synchronizeSpaceWidgets(dialog_->choice_space_above,
					dialog_->input_space_above,
					dialog_->choice_value_space_above);
	}

	if (ob == dialog_->choice_space_below) {
		synchronizeSpaceWidgets(dialog_->choice_space_below,
					dialog_->input_space_below,
					dialog_->choice_value_space_below);
	}

	// Display a warning if the input is senseless
	bool valid = (validSpaceWidgets(dialog_->choice_space_above,
					dialog_->input_space_above) &&
		      validSpaceWidgets(dialog_->choice_space_below,
					dialog_->input_space_below));

	if (!valid) {
		postWarning(_("Invalid Length (valid example: 10mm)"));
	}

	int const choice_spacing = fl_get_choice(dialog_->choice_linespacing);

	if (choice_spacing == 5)
		setEnabled(dialog_->input_linespacing, true);
	else {
		fl_set_input(dialog_->input_linespacing, "");
		setEnabled(dialog_->input_linespacing, false);
	}

	double const spacing =
		strToDbl(getStringFromInput(dialog_->input_linespacing));

	if (choice_spacing == 5 && int(spacing) == 0)
		valid = false;

	return ButtonPolicy::SMI_VALID;
}


