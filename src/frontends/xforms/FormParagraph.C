/**
 * \file FormParagraph.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormParagraph.h"
#include "ControlParagraph.h"
#include "forms/form_paragraph.h"

#include "checkedwidgets.h"
#include "input_validators.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include "xformsBC.h"

#include "controllers/helper_funcs.h"

#include "lyxrc.h" // to set the deafult length values
#include "ParagraphParameters.h"
#include "Spacing.h"

#include "support/lstrings.h"
#include "support/tostr.h"

#include "lyx_forms.h"

#include <boost/bind.hpp>

using lyx::support::contains;
using lyx::support::getStringFromVector;
using lyx::support::rtrim;

using boost::bind;

using std::remove_if;

using std::vector;
using std::string;


namespace {

string defaultUnit("cm");

} // namespace anon


typedef FormController<ControlParagraph, FormView<FD_paragraph> > base_class;

FormParagraph::FormParagraph(Dialog & parent)
	: base_class(parent, _("Paragraph Settings"))
{}


void FormParagraph::build()
{
	// the tabbed folder
	dialog_.reset(build_paragraph(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
	bcview().setRestore(dialog_->button_restore);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->check_noindent);
	bcview().addReadOnly(dialog_->choice_linespacing);
	bcview().addReadOnly(dialog_->input_linespacing);

	bcview().addReadOnly(dialog_->input_labelwidth);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_linespacing);
	setPrehandler(dialog_->input_labelwidth);

	fl_set_input_return(dialog_->input_labelwidth,  FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_linespacing, FL_RETURN_CHANGED);

	// limit these inputs to unsigned floats
	fl_set_input_filter(dialog_->input_linespacing, fl_unsigned_float_filter);

	// add alignment radio buttons
	alignment_.init(dialog_->radio_align_left,   LYX_ALIGN_LEFT);
	alignment_.init(dialog_->radio_align_right,  LYX_ALIGN_RIGHT);
	alignment_.init(dialog_->radio_align_block,  LYX_ALIGN_BLOCK);
	alignment_.init(dialog_->radio_align_center, LYX_ALIGN_CENTER);

	string const linespacing = _("Default|Single|OneHalf|Double|Custom");
	fl_addto_choice(dialog_->choice_linespacing, linespacing.c_str());

	// Create the contents of the unit choices; don't include the "%" terms.
	vector<string> units_vec = getLatexUnits();
	vector<string>::iterator del =
		remove_if(units_vec.begin(), units_vec.end(),
			  bind(contains<char>, _1, '%'));
	units_vec.erase(del, units_vec.end());

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


void FormParagraph::apply()
{
	if (!form())
		return;

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

	// lines, pagebreaks and indent
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

	// no indent
	fl_set_button(dialog_->check_noindent,
		      controller().params().noindent());
}


ButtonPolicy::SMInput FormParagraph::input(FL_OBJECT * ob, long)
{
	// Enable input when custum length is choosen,
	// disable 'keep' when no space is choosen
	if (ob == dialog_->choice_linespacing) {
		bool const custom_spacing =
			fl_get_choice(dialog_->choice_linespacing) == 5;
		setEnabled(dialog_->input_linespacing, custom_spacing);
	}

	return ButtonPolicy::SMI_VALID;
}
