/**
 * \file FormParagraph.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include FORMS_H_LOCATION

#include "FormParagraph.h"
#include "form_paragraph.h"
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxtext.h"
#include "xforms_helpers.h"
#include "lyxrc.h" // to set the deafult length values
#include "BufferView.h"
#include "lyxtextclasslist.h"
#include "Spacing.h"
#include "ParagraphParameters.h"
#include "input_validators.h"
#include "helper_funcs.h"

#include "support/lstrings.h"

#include <functional>

using Liason::setMinibuffer;
using SigC::slot;
using std::vector;
using std::bind2nd;
using std::remove_if;


FormParagraph::FormParagraph(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Paragraph Layout")), par_(0)
{
    // let the dialog be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showParagraph.connect(slot(this, &FormParagraph::show));
}


void FormParagraph::connect()
{
	cp_ = d_->updateParagraph
		.connect(slot(this, &FormParagraph::changedParagraph));
	FormBaseBD::connect();
}


void FormParagraph::disconnect()
{
	cp_.disconnect();
	FormBaseBD::disconnect();
}


Paragraph const * FormParagraph::getCurrentParagraph() const
{
	return lv_->view()->getLyXText()->cursor.par();
}


void FormParagraph::changedParagraph()
{
	/// Record the paragraph
	Paragraph const * const p = getCurrentParagraph();
	if (p == 0 || p == par_)
		return;

	// OBS FIX LOOK HERE

	// shouldn't we chage the par_ pointer too?
	// anyway for me the below function does just nothing!
	// (Jug 20020108)

	// For now don't bother checking if the params are different,
	// just activate the Apply button
	bc().valid();
}


void FormParagraph::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
}


FL_FORM * FormParagraph::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormParagraph::build()
{
	// the tabbed folder
	dialog_.reset(build_paragraph());

	fl_addto_choice(dialog_->choice_space_above,
		_(" None | Defskip | Smallskip "
		"| Medskip | Bigskip | VFill | Length "));
	fl_addto_choice(dialog_->choice_space_below,
		_(" None | Defskip | Smallskip "
		"| Medskip | Bigskip | VFill | Length ")); 

	fl_addto_choice(dialog_->choice_linespacing,
		_(" Default | Single | OneHalf | Double | Other "));

	fl_set_input_return(dialog_->input_space_above, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_space_below, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_labelwidth, FL_RETURN_CHANGED);
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

	fl_addto_choice(dialog_->choice_value_space_above,  units.c_str());
	fl_addto_choice(dialog_->choice_value_space_below, units.c_str());

	// Manage the ok, apply, restore and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_close);
	bc_.setRestore(dialog_->button_restore);

	bc_.addReadOnly(dialog_->radio_align_right);
	bc_.addReadOnly(dialog_->radio_align_left);
	bc_.addReadOnly(dialog_->radio_align_block);
	bc_.addReadOnly(dialog_->radio_align_center);
	bc_.addReadOnly(dialog_->check_lines_top);
	bc_.addReadOnly(dialog_->check_lines_bottom);
	bc_.addReadOnly(dialog_->check_pagebreaks_top);
	bc_.addReadOnly(dialog_->check_pagebreaks_bottom);
	bc_.addReadOnly(dialog_->choice_space_above);
	bc_.addReadOnly(dialog_->input_space_above);
	bc_.addReadOnly(dialog_->check_space_above);
	bc_.addReadOnly(dialog_->choice_space_below);
	bc_.addReadOnly(dialog_->input_space_below);
	bc_.addReadOnly(dialog_->check_space_below);
	bc_.addReadOnly(dialog_->choice_linespacing);
	bc_.addReadOnly(dialog_->input_linespacing); 
	bc_.addReadOnly(dialog_->check_noindent);
	bc_.addReadOnly(dialog_->input_labelwidth);
}


void FormParagraph::apply()
{
	if (!lv_->view()->available() || !dialog_.get())
		return;

	VSpace space_top;
	VSpace space_bottom;
	LyXAlignment align;
	string labelwidthstring;
	bool noindent;

	// If a vspace kind is "Length" but there's no text in
	// the input field, reset the kind to "None". 
	if ((fl_get_choice(dialog_->choice_space_above) == 7) &&
	    !*(fl_get_input(dialog_->input_space_above)))
	{
		fl_set_choice(dialog_->choice_space_above, 1);
	}

	if ((fl_get_choice (dialog_->choice_space_below) == 7) &&
	    !*(fl_get_input (dialog_->input_space_below)))
	{
		fl_set_choice(dialog_->choice_space_below, 1);
	}

	bool line_top         = fl_get_button(dialog_->check_lines_top);
	bool line_bottom      = fl_get_button(dialog_->check_lines_bottom);
	bool pagebreak_top    = fl_get_button(dialog_->check_pagebreaks_top);
	bool pagebreak_bottom = fl_get_button(dialog_->check_pagebreaks_bottom);

	switch (fl_get_choice(dialog_->choice_space_above)) {
		case 1:
			space_top = VSpace(VSpace::NONE);
			break;
		case 2:
			space_top = VSpace(VSpace::DEFSKIP);
			break;
		case 3:
			space_top = VSpace(VSpace::SMALLSKIP);
			break;
		case 4:
			space_top = VSpace(VSpace::MEDSKIP);
			break;
		case 5:
			space_top = VSpace(VSpace::BIGSKIP);
			break;
		case 6:
			space_top = VSpace(VSpace::VFILL);
			break;
		case 7: {
			string const length =
				getLengthFromWidgets(dialog_->input_space_above,
					dialog_->choice_value_space_above);
			space_top = VSpace(LyXGlueLength(length));
			break;
		}
	}

	if (fl_get_button(dialog_->check_space_above))
		space_top.setKeep(true);
	switch (fl_get_choice(dialog_->choice_space_below)) {
		case 1:
			space_bottom = VSpace(VSpace::NONE);
			break;
		case 2:
			space_bottom = VSpace(VSpace::DEFSKIP);
			break;
		case 3:
			space_bottom = VSpace(VSpace::SMALLSKIP);
			break;
		case 4:
			space_bottom = VSpace(VSpace::MEDSKIP);
			break;
		case 5:
			space_bottom = VSpace(VSpace::BIGSKIP);
			break;
		case 6:
			space_bottom = VSpace(VSpace::VFILL);
			break;
		case 7:
			string const length =
				getLengthFromWidgets(dialog_->input_space_below,
					dialog_->choice_value_space_below);
			space_bottom = VSpace(LyXGlueLength(length));
			break;
	}
	if (fl_get_button (dialog_->check_space_below))
		space_bottom.setKeep (true);

	if (fl_get_button(dialog_->radio_align_left))
		align = LYX_ALIGN_LEFT;
	else if (fl_get_button(dialog_->radio_align_right))
		align = LYX_ALIGN_RIGHT;
	else if (fl_get_button(dialog_->radio_align_center))
		align = LYX_ALIGN_CENTER;
	else 
		align = LYX_ALIGN_BLOCK;

	labelwidthstring = fl_get_input(dialog_->input_labelwidth);
	noindent = fl_get_button(dialog_->check_noindent);
	Spacing::Space linespacing = Spacing::Default;
	string other_linespacing;
	switch (fl_get_choice(dialog_->choice_linespacing)) {
		case 1: linespacing = Spacing::Default; break;
		case 2: linespacing = Spacing::Single; break;
		case 3: linespacing = Spacing::Onehalf; break;
		case 4: linespacing = Spacing::Double; break;
		case 5:
			linespacing = Spacing::Other;
			other_linespacing = fl_get_input(dialog_->input_linespacing);
			break;
	}

	Spacing const spacing(linespacing, other_linespacing);
	LyXText * text(lv_->view()->getLyXText());
	text->setParagraph(lv_->view(), line_top, line_bottom, pagebreak_top,
	pagebreak_bottom, space_top, space_bottom, spacing,
		 align, labelwidthstring, noindent);


	// Actually apply these settings
	lv_->view()->update(text, 
	BufferView::SELECT | BufferView::FITCUR | BufferView::CHANGE);
	lv_->buffer()->markDirty();
	setMinibuffer(lv_, _("Paragraph layout set"));
}


void FormParagraph::update()
{
	if (!dialog_.get())
		return;

	// Do this first; some objects may be de/activated subsequently.
	bc_.readOnly(lv_->buffer()->isReadonly());

	Buffer * buf = lv_->view()->buffer();

	/// Record the paragraph
	par_ = getCurrentParagraph();

	fl_set_input(dialog_->input_labelwidth,
		par_->getLabelWidthString().c_str());
	setEnabled(dialog_->input_labelwidth,
		(par_->getLabelWidthString() != _("Senseless with this layout!")));

	fl_set_button(dialog_->radio_align_right, 0);
	fl_set_button(dialog_->radio_align_left, 0);
	fl_set_button(dialog_->radio_align_center, 0);
	fl_set_button(dialog_->radio_align_block, 0);

	LyXTextClass const & tclass = textclasslist[buf->params.textclass];

	int align = par_->getAlign();
	if (align == LYX_ALIGN_LAYOUT)
		align = tclass[par_->layout()].align;

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

	LyXAlignment alignpos = tclass[par_->layout()].alignpossible;

	setEnabled(dialog_->radio_align_block,  bool(alignpos & LYX_ALIGN_BLOCK));
	setEnabled(dialog_->radio_align_center, bool(alignpos & LYX_ALIGN_CENTER));
	setEnabled(dialog_->radio_align_left,   bool(alignpos & LYX_ALIGN_LEFT));
	setEnabled(dialog_->radio_align_right,  bool(alignpos & LYX_ALIGN_RIGHT));

	// no inset-text-owned paragraph may have pagebreaks
	setEnabled(dialog_->check_pagebreaks_top, !par_->inInset());
	setEnabled(dialog_->check_pagebreaks_bottom, !par_->inInset());

	fl_set_button(dialog_->check_lines_top,
		par_->params().lineTop());
	fl_set_button(dialog_->check_lines_bottom,
		par_->params().lineBottom());
	fl_set_button(dialog_->check_pagebreaks_top,
		par_->params().pagebreakTop());
	fl_set_button(dialog_->check_pagebreaks_bottom,
		par_->params().pagebreakBottom());
	fl_set_button(dialog_->check_noindent,
		par_->params().noindent());

	int linespacing;
	Spacing const space = par_->params().spacing();

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

	fl_set_input (dialog_->input_space_above, "");

	setEnabled(dialog_->input_space_above, false);
	setEnabled(dialog_->choice_value_space_above, false);
	switch (par_->params().spaceTop().kind()) {
		case VSpace::NONE:
			fl_set_choice (dialog_->choice_space_above, 1);
			break;
		case VSpace::DEFSKIP:
			fl_set_choice (dialog_->choice_space_above, 2);
			break;
		case VSpace::SMALLSKIP:
			fl_set_choice (dialog_->choice_space_above, 3);
			break;
		case VSpace::MEDSKIP:
			fl_set_choice (dialog_->choice_space_above, 4);
			break;
		case VSpace::BIGSKIP:
			fl_set_choice (dialog_->choice_space_above, 5);
			break;
		case VSpace::VFILL:
			fl_set_choice (dialog_->choice_space_above, 6);
			break;
		case VSpace::LENGTH: {
			fl_set_choice (dialog_->choice_space_above, 7);
			setEnabled(dialog_->input_space_above, true);
			setEnabled(dialog_->choice_value_space_above, true);
			bool const metric = lyxrc.default_papersize > 3;
			string const default_unit = metric ? "cm" : "in";
			string const length = par_->params().spaceTop().length().asString();
			updateWidgetsFromLengthString(dialog_->input_space_above,
			dialog_->choice_value_space_above,
							length, default_unit);
			break;
		}
	}

	fl_set_button (dialog_->check_space_above,
	par_->params().spaceTop().keep());
	fl_set_input (dialog_->input_space_below, "");

	setEnabled(dialog_->input_space_below, false);
	setEnabled(dialog_->choice_value_space_below, false);
	switch (par_->params().spaceBottom().kind()) {
		case VSpace::NONE:
			fl_set_choice(dialog_->choice_space_below, 1);
			break;
		case VSpace::DEFSKIP:
			fl_set_choice(dialog_->choice_space_below, 2);
			break;
		case VSpace::SMALLSKIP:
			fl_set_choice(dialog_->choice_space_below, 3);
			break;
		case VSpace::MEDSKIP:
			fl_set_choice(dialog_->choice_space_below, 4);
			break;
		case VSpace::BIGSKIP:
			fl_set_choice(dialog_->choice_space_below, 5);
			break;
		case VSpace::VFILL:
			fl_set_choice(dialog_->choice_space_below, 6);
			break;
		case VSpace::LENGTH: {
			fl_set_choice(dialog_->choice_space_below, 7);
			setEnabled(dialog_->input_space_below, true);
			setEnabled(dialog_->choice_value_space_below, true);
			bool const metric = lyxrc.default_papersize > 3;
			string const default_unit = metric ? "cm" : "in";
			string const length =
				par_->params().spaceBottom().length().asString();
			updateWidgetsFromLengthString(dialog_->input_space_below,
				dialog_->choice_value_space_below,
				length, default_unit);
			break;
		}
	}

	fl_set_button(dialog_->check_space_below,
		par_->params().spaceBottom().keep());
	fl_set_button(dialog_->check_noindent,
		par_->params().noindent());
}


bool FormParagraph::input(FL_OBJECT * ob, long)
{
	bool valid = true; 

	fl_hide_object(dialog_->text_warning);

	// First check the buttons which are exclusive and you have to
	// check only the actuall de/activated button.
	//
	// "Synchronize" the choices and input fields, making it
	// impossible to commit senseless data.

	if (ob == dialog_->choice_space_above) {
		if (fl_get_choice (dialog_->choice_space_above) != 7) {
			fl_set_input(dialog_->input_space_above, "");
			setEnabled(dialog_->input_space_above, false);
			setEnabled(dialog_->choice_value_space_above, false);
		} else {
			setEnabled(dialog_->input_space_above,
				!lv_->buffer()->isReadonly());
			setEnabled(dialog_->choice_value_space_above,
				!lv_->buffer()->isReadonly());
			bool const metric = lyxrc.default_papersize > 3;
			int const default_unit = metric ? 8 : 9;
			if (strip(fl_get_input(dialog_->input_space_above)).empty())
				fl_set_choice(dialog_->choice_value_space_above, default_unit);
		}
	}
	if (ob == dialog_->choice_space_below) {
		if (fl_get_choice (dialog_->choice_space_below) != 7) {
			fl_set_input(dialog_->input_space_below, "");
			setEnabled(dialog_->input_space_below, false);
			setEnabled(dialog_->choice_value_space_below, false);
		} else {
			setEnabled(dialog_->input_space_below,
				!lv_->buffer()->isReadonly());
			setEnabled(dialog_->choice_value_space_below,
				!lv_->buffer()->isReadonly());
			bool const metric = lyxrc.default_papersize > 3;
			int const default_unit = metric ? 8 : 9;
			if (strip(fl_get_input(dialog_->input_space_below)).empty())
				fl_set_choice(dialog_->choice_value_space_below, default_unit);
		}
	}

	//
	// warnings if input is senseless
	//
	string input = fl_get_input(dialog_->input_space_above);
	bool invalid = false;

	if (fl_get_choice(dialog_->choice_space_above) == 7)
		invalid = !input.empty() &&
		          !isValidGlueLength(input) &&
		          !isStrDbl(input);

	input = fl_get_input(dialog_->input_space_below);

	if (fl_get_choice(dialog_->choice_space_below) == 7)
		invalid = invalid ||
			(!input.empty() && !isValidGlueLength(input) && !isStrDbl(input));

	if (ob == dialog_->input_space_above || ob == dialog_->input_space_below) {
		if (invalid) {
			fl_set_object_label(dialog_->text_warning,
				_("Warning: Invalid Length (valid example: 10mm)"));
			fl_show_object(dialog_->text_warning);
			return false;
		} else {
			fl_hide_object(dialog_->text_warning);
			return true;
		}
	}

	if (fl_get_choice (dialog_->choice_linespacing) == 5)
		setEnabled(dialog_->input_linespacing, true);
	else {
		setEnabled(dialog_->input_linespacing, false);
		fl_set_input (dialog_->input_linespacing, "");
	}

	double spacing(strToDbl(fl_get_input(dialog_->input_linespacing)));

	if (fl_get_choice(dialog_->choice_linespacing) == 5 && int(spacing) == 0)
		valid = false;

	return valid;
}
