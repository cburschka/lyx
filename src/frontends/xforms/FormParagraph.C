/**
 * \file FormParagraph.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlParagraph.h"
#include "FormParagraph.h"
#include "forms/form_paragraph.h"
#include "Tooltips.h"

#include "ParagraphParameters.h"

#include "xforms_helpers.h"
#include "lyxrc.h" // to set the deafult length values
#include "input_validators.h"
#include "helper_funcs.h"
#include "checkedwidgets.h"
#include "gettext.h"
#include "xformsBC.h"
#include "layout.h" // LyXAlignment

#include "support/lstrings.h"
#include "support/LAssert.h"

#include FORMS_H_LOCATION

#include <functional>

using std::vector;
using std::bind2nd;
using std::remove_if;


namespace
{

string defaultUnit("cm");

void validateVSpaceWidgets(FL_OBJECT * choice_type, FL_OBJECT * input_length);

VSpace const setVSpaceFromWidgets(FL_OBJECT * choice_type,
				  FL_OBJECT * input_length,
				  FL_OBJECT * choice_length,
				  FL_OBJECT * check_keep);

void setWidgetsFromVSpace(VSpace const & space,
			  FL_OBJECT * choice_type,
			  FL_OBJECT * input_length,
			  FL_OBJECT * choice_length,
			  FL_OBJECT * check_keep);

} // namespace anon


typedef FormCB<ControlParagraph, FormDB<FD_paragraph> > base_class;

FormParagraph::FormParagraph()
	: base_class(_("Paragraph Layout"))
{}


void FormParagraph::build()
{
	// the tabbed folder
	dialog_.reset(build_paragraph(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	// disable for read-only documents
	bc().addReadOnly(dialog_->check_line_above);
	bc().addReadOnly(dialog_->check_pagebreak_above);
	bc().addReadOnly(dialog_->choice_space_above);
	bc().addReadOnly(dialog_->input_space_above);
	bc().addReadOnly(dialog_->check_space_above);

	bc().addReadOnly(dialog_->check_noindent);
	bc().addReadOnly(dialog_->choice_linespacing);
	bc().addReadOnly(dialog_->input_linespacing);

	bc().addReadOnly(dialog_->check_line_below);
	bc().addReadOnly(dialog_->check_pagebreak_below);
	bc().addReadOnly(dialog_->choice_space_below);
	bc().addReadOnly(dialog_->input_space_below);
	bc().addReadOnly(dialog_->check_space_below);

	bc().addReadOnly(dialog_->input_labelwidth);

	// check validity of "length + unit" input
	addCheckedGlueLength(bc(),
			     dialog_->input_space_above,
			     dialog_->choice_space_above);
	addCheckedGlueLength(bc(),
			     dialog_->input_space_below,
			     dialog_->choice_space_below);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_space_above);
	setPrehandler(dialog_->input_space_below);
	setPrehandler(dialog_->input_linespacing);
	setPrehandler(dialog_->input_labelwidth);

	fl_set_input_return(dialog_->input_space_above, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_space_below, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_labelwidth,  FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_linespacing, FL_RETURN_CHANGED);

	// limit these inputs to unsigned floats
	fl_set_input_filter(dialog_->input_linespacing, fl_unsigned_float_filter);

	// add alignment radio buttons
	alignment_.init(dialog_->radio_align_left,   LYX_ALIGN_LEFT);
	alignment_.init(dialog_->radio_align_right,  LYX_ALIGN_RIGHT);
	alignment_.init(dialog_->radio_align_block,  LYX_ALIGN_BLOCK);
	alignment_.init(dialog_->radio_align_center, LYX_ALIGN_CENTER);

	string const parspacing = _("None|DefSkip|SmallSkip|MedSkip|BigSkip|VFill|Length");
	fl_addto_choice(dialog_->choice_space_above, parspacing.c_str());
	fl_addto_choice(dialog_->choice_space_below, parspacing.c_str());

	string const linespacing = _("Default|Single|OneHalf|Double|Custom");
	fl_addto_choice(dialog_->choice_linespacing, linespacing.c_str());

	// Create the contents of the unit choices; don't include the "%" terms.
	vector<string> units_vec = getLatexUnits();
	vector<string>::iterator del =
		remove_if(units_vec.begin(), units_vec.end(),
			  bind2nd(contains_functor(), "%"));
	units_vec.erase(del, units_vec.end());

	string const units = getStringFromVector(units_vec, "|");
	fl_addto_choice(dialog_->choice_unit_space_above, units.c_str());
	fl_addto_choice(dialog_->choice_unit_space_below, units.c_str());

	// set up the tooltips
	string str = _("Add a separator line above this paragraph.");
	tooltips().init(dialog_->check_line_above, str);
	str = _("Enforce a page break above this paragraph.");
	tooltips().init(dialog_->check_pagebreak_above, str);
	str = _("Add additional space above this paragraph.");
	tooltips().init(dialog_->choice_space_above, str);
	str = _("Never suppress space (e.g. at top of page or new page).");
	tooltips().init(dialog_->check_space_above, str);

	str = _("Add a separator line below this paragraph.");
	tooltips().init(dialog_->check_line_below, str);
	str = _("Enforce a page break below this paragraph.");
	tooltips().init(dialog_->check_pagebreak_below, str);
	str = _("Add additional space below this paragraph.");
	tooltips().init(dialog_->choice_space_below, str);
	str = _("Never suppress space (e.g. at bottom of page or new page).");
	tooltips().init(dialog_->check_space_below, str);

	// set default unit for custom length
	switch (lyxrc.default_papersize) {
		case BufferParams::PAPER_DEFAULT:
		case BufferParams::PAPER_USLETTER:
		case BufferParams::PAPER_LEGALPAPER:
		case BufferParams::PAPER_EXECUTIVEPAPER:
			defaultUnit = "in";
			break;
		case BufferParams::PAPER_A3PAPER:
		case BufferParams::PAPER_A4PAPER:
		case BufferParams::PAPER_A5PAPER:
		case BufferParams::PAPER_B5PAPER:
			defaultUnit = "cm";
			break;
	}
}


void FormParagraph::apply()
{
	if (!form()) return;

	// spacing
	// If a vspace choice is "Length" but there's no text in
	// the input field, reset the choice to "None".
	validateVSpaceWidgets(dialog_->choice_space_above,
			      dialog_->input_space_above);

	VSpace const space_above =
		setVSpaceFromWidgets(dialog_->choice_space_above,
				     dialog_->input_space_above,
				     dialog_->choice_unit_space_above,
				     dialog_->check_space_above);

	controller().params().spaceTop(space_above);

	validateVSpaceWidgets(dialog_->choice_space_below,
			      dialog_->input_space_below);

	VSpace const space_below =
		setVSpaceFromWidgets(dialog_->choice_space_below,
				     dialog_->input_space_below,
				     dialog_->choice_unit_space_below,
				     dialog_->check_space_below);

	controller().params().spaceBottom(space_below);

	// lines and pagebreaks
	bool const line_above = fl_get_button(dialog_->check_line_above);
	controller().params().lineTop(line_above);

	bool const line_below = fl_get_button(dialog_->check_line_below);
	controller().params().lineBottom(line_below);

	bool const pagebreak_above =
		fl_get_button(dialog_->check_pagebreak_above);
	controller().params().pagebreakTop(pagebreak_above);

	bool const pagebreak_below =
		fl_get_button(dialog_->check_pagebreak_below);
	controller().params().pagebreakBottom(pagebreak_below);


	// alignment
	LyXAlignment const alignment =
		static_cast<LyXAlignment>(alignment_.get());
	controller().params().align(alignment);

	// label width
	string const labelwidthstring =
		getString(dialog_->input_labelwidth);
	controller().params().labelWidthString(labelwidthstring);

	// indendation
	bool const noindent = fl_get_button(dialog_->check_noindent);
	controller().params().noindent(noindent);

	// get spacing
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
		// reset to default if input is empty
		other = getString(dialog_->input_linespacing);
		if (!other.empty()) {
			linespacing = Spacing::Other;
		} else {
			linespacing = Spacing::Default;
			fl_set_choice(dialog_->choice_linespacing, 1);
		}
		break;
	}
	Spacing const spacing(linespacing, other);
	controller().params().spacing(spacing);
}


void FormParagraph::update()
{
	if (!dialog_.get())
		return;

	// label width
	string const labelwidth = controller().params().labelWidthString();
	fl_set_input(dialog_->input_labelwidth, labelwidth.c_str());
	setEnabled(dialog_->input_labelwidth,
		   labelwidth != _("Senseless with this layout!"));

	// alignment
	alignment_.set(controller().params().align());

	// mark default alignment
	LyXAlignment const default_alignment = controller().alignDefault();

	string label = _("Block");
	if (default_alignment == LYX_ALIGN_BLOCK) {
		label += _(" (default)");
	}
	fl_set_object_label(dialog_->radio_align_block, label.c_str());
	fl_set_button_shortcut(dialog_->radio_align_block, "#B", 1);

	label = _("Center");
	if (default_alignment == LYX_ALIGN_CENTER) {
		label += _(" (default)");
	}
	fl_set_object_label(dialog_->radio_align_center, label.c_str());
	fl_set_button_shortcut(dialog_->radio_align_center, "#C", 1);

	label = _("Left");
	if (default_alignment == LYX_ALIGN_LEFT) {
		label += _(" (default)");
	}
	fl_set_object_label(dialog_->radio_align_left, label.c_str());
	fl_set_button_shortcut(dialog_->radio_align_left, "#L", 1);

	label = _("Right");
	if (default_alignment == LYX_ALIGN_RIGHT) {
		label = _(" (default)");
	}
	fl_set_object_label(dialog_->radio_align_right, label.c_str());
	fl_set_button_shortcut(dialog_->radio_align_right, "#R", 1);

	// Ensure that there's no crud left on the screen from this change
	// of labels.
	fl_redraw_form(form());

	LyXAlignment alignpos = controller().alignPossible();
	setEnabled(dialog_->radio_align_block,
		   bool(alignpos & LYX_ALIGN_BLOCK));
	setEnabled(dialog_->radio_align_center,
		   bool(alignpos & LYX_ALIGN_CENTER));
	setEnabled(dialog_->radio_align_left,
		   bool(alignpos & LYX_ALIGN_LEFT));
	setEnabled(dialog_->radio_align_right,
		   bool(alignpos & LYX_ALIGN_RIGHT));

	// no inset-text-owned paragraph may have pagebreaks
	bool ininset = controller().inInset();
	setEnabled(dialog_->check_pagebreak_above, !ininset);
	setEnabled(dialog_->check_pagebreak_below, !ininset);

	// lines, pagebreaks and indent
	fl_set_button(dialog_->check_line_above,
		      controller().params().lineTop());
	fl_set_button(dialog_->check_line_below,
		      controller().params().lineBottom());
	fl_set_button(dialog_->check_pagebreak_above,
		      controller().params().pagebreakTop());
	fl_set_button(dialog_->check_pagebreak_below,
		      controller().params().pagebreakBottom());
	fl_set_button(dialog_->check_noindent,
		      controller().params().noindent());

	// linespacing
	Spacing const space = controller().params().spacing();

	int pos;
	switch (space.getSpace()) {
	case Spacing::Other:
		pos = 5;
		break;
	case Spacing::Double:
		pos = 4;
		break;
	case Spacing::Onehalf:
		pos = 3;
		break;
	case Spacing::Single:
		pos = 2;
		break;
	case Spacing::Default:
	default:
		pos = 1;
		break;
	}
	fl_set_choice(dialog_->choice_linespacing, pos);

	bool const spacing_other = space.getSpace() == Spacing::Other;
	setEnabled(dialog_->input_linespacing, spacing_other);
	if (spacing_other) {
		string const sp = tostr(space.getValue());
		fl_set_input(dialog_->input_linespacing, sp.c_str());
	} else {
		fl_set_input(dialog_->input_linespacing, "");
	}

	// vspace top
	setWidgetsFromVSpace(controller().params().spaceTop(),
			     dialog_->choice_space_above,
			     dialog_->input_space_above,
			     dialog_->choice_unit_space_above,
			     dialog_->check_space_above);

	// vspace bottom
	setWidgetsFromVSpace(controller().params().spaceBottom(),
			     dialog_->choice_space_below,
			     dialog_->input_space_below,
			     dialog_->choice_unit_space_below,
			     dialog_->check_space_below);

	// no indent
	fl_set_button(dialog_->check_noindent,
		      controller().params().noindent());
}


ButtonPolicy::SMInput FormParagraph::input(FL_OBJECT * ob, long)
{
	// Enable input when custum length is choosen,
	// disable 'keep' when no space is choosen
	if (ob == dialog_->choice_space_above) {
		bool const custom_length =
			fl_get_choice(dialog_->choice_space_above) == 7;
		setEnabled(dialog_->input_space_above, custom_length);
		setEnabled(dialog_->choice_unit_space_above, custom_length);

		bool const space =
			fl_get_choice(dialog_->choice_space_above) != 1;
		setEnabled(dialog_->check_space_above, space);

	} else if (ob == dialog_->choice_space_below) {
		bool const custom_length =
			fl_get_choice(dialog_->choice_space_below) == 7;
		setEnabled(dialog_->input_space_below, custom_length);
		setEnabled(dialog_->choice_unit_space_below, custom_length);

		bool const space =
			fl_get_choice(dialog_->choice_space_below) != 1;
		setEnabled(dialog_->check_space_below, space);

	} else if (ob == dialog_->choice_linespacing) {
		bool const custom_spacing =
			fl_get_choice(dialog_->choice_linespacing) == 5;
		setEnabled(dialog_->input_linespacing, custom_spacing);
	}

	return ButtonPolicy::SMI_VALID;
}


namespace {

void validateVSpaceWidgets(FL_OBJECT * choice_type, FL_OBJECT * input_length)
{
	// Paranoia check!
	lyx::Assert(choice_type  && choice_type->objclass  == FL_CHOICE &&
		    input_length && input_length->objclass == FL_INPUT);

	if (fl_get_choice(choice_type) != 7)
		return;

	// If a vspace kind is "Length" but there's no text in
	// the input field, reset the kind to "None".
	string const input = rtrim(getString(input_length));
	if (input.empty())
		fl_set_choice(choice_type, 1);
}


VSpace const setVSpaceFromWidgets(FL_OBJECT * choice_type,
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

	fl_set_button(check_keep, space.keep());

	int pos = 1;
	switch (space.kind()) {
	case VSpace::NONE:
		pos = 1;
		break;
	case VSpace::DEFSKIP:
		pos = 2;
		break;
	case VSpace::SMALLSKIP:
		pos = 3;
		break;
	case VSpace::MEDSKIP:
		pos = 4;
		break;
	case VSpace::BIGSKIP:
		pos = 5;
		break;
	case VSpace::VFILL:
		pos = 6;
		break;
	case VSpace::LENGTH:
		pos = 7;
		break;
	}
	fl_set_choice(choice_type, pos);

	bool const custom_vspace = space.kind() == VSpace::LENGTH;
	setEnabled(input_length, custom_vspace);
	setEnabled(choice_length, custom_vspace);
	if (custom_vspace) {
		string const length = space.length().asString();
		updateWidgetsFromLengthString(input_length, choice_length,
					      length, defaultUnit);
	} else {
		bool const no_vspace = space.kind() == VSpace::NONE;
		setEnabled(check_keep, !no_vspace);
		fl_set_input(input_length, "");
		fl_set_choice_text(choice_length, defaultUnit.c_str());
	}
}

} // namespace anon
