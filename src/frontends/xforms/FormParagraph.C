/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000-2001 The LyX Team.
 *
 *           @author Jürgen Vigna
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include FORMS_H_LOCATION

#include "FormParagraph.h"
#include "form_paragraph.h"
#include "Dialogs.h"
#include "support/lstrings.h" 
#include "Liason.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxtext.h"
#include "xforms_helpers.h"
#include "BufferView.h"
#include "Spacing.h"
#include "ParagraphParameters.h"
#include "input_validators.h"

using Liason::setMinibuffer;
using SigC::slot;


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
	LyXText * text = 0;

	if (lv_->view()->theLockingInset())
		text = lv_->view()->theLockingInset()->getLyXText(lv_->view());
	if (!text)
		text = lv_->view()->text;
	return text->cursor.par();
}


void FormParagraph::changedParagraph()
{
	/// Record the paragraph
	Paragraph const * const p = getCurrentParagraph();
	if (p == 0 || p == par_)
		return;

	// For now don't bother checking if the params are different,
	// just activate the Apply button
	bc().valid();
}


void FormParagraph::redraw()
{
	if( form() && form()->visible )
		fl_redraw_form( form() );
}


FL_FORM * FormParagraph::form() const
{
    if (dialog_.get()) return dialog_->form;
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

    // Manage the ok, apply, restore and cancel/close buttons
    bc_.setOK(dialog_->button_ok);
    bc_.setApply(dialog_->button_apply);
    bc_.setCancel(dialog_->button_cancel);
    bc_.setRestore(dialog_->button_restore);

    bc_.addReadOnly (dialog_->group_radio_alignment);
    // bc_.addReadOnly (dialog_->radio_align_right);
    // bc_.addReadOnly (dialog_->radio_align_left);
    // bc_.addReadOnly (dialog_->radio_align_block);
    // bc_.addReadOnly (dialog_->radio_align_center);
    bc_.addReadOnly (dialog_->check_lines_top);
    bc_.addReadOnly (dialog_->check_lines_bottom);
    bc_.addReadOnly (dialog_->check_pagebreaks_top);
    bc_.addReadOnly (dialog_->check_pagebreaks_bottom);
    bc_.addReadOnly (dialog_->choice_space_above);
    bc_.addReadOnly (dialog_->input_space_above);
    bc_.addReadOnly (dialog_->check_space_above);
    bc_.addReadOnly (dialog_->choice_space_below);
    bc_.addReadOnly (dialog_->input_space_below);
    bc_.addReadOnly (dialog_->check_space_below);
    bc_.addReadOnly (dialog_->choice_linespacing);
    bc_.addReadOnly (dialog_->input_linespacing); 
    bc_.addReadOnly (dialog_->check_noindent);
    bc_.addReadOnly (dialog_->input_labelwidth);
}


void FormParagraph::apply()
{
    if (!lv_->view()->available() || !dialog_.get())
	return;

    VSpace space_top, space_bottom;
    LyXAlignment align;
    string labelwidthstring;
    bool noindent;

    // If a vspace kind is "Length" but there's no text in
    // the input field, reset the kind to "None". 
    if ((fl_get_choice (dialog_->choice_space_above) == 7) &&
	!*(fl_get_input (dialog_->input_space_above)))
    {
	fl_set_choice (dialog_->choice_space_above, 1);
    }
    if ((fl_get_choice (dialog_->choice_space_below) == 7) &&
	!*(fl_get_input (dialog_->input_space_below)))
    {
	fl_set_choice (dialog_->choice_space_below, 1);
    }
   
    bool line_top = fl_get_button(dialog_->check_lines_top);
    bool line_bottom = fl_get_button(dialog_->check_lines_bottom);
    bool pagebreak_top = fl_get_button(dialog_->check_pagebreaks_top);
    bool pagebreak_bottom = fl_get_button(dialog_->check_pagebreaks_bottom);
    
    switch (fl_get_choice (dialog_->choice_space_above)) {
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
    case 7:
	space_top =
		VSpace(LyXGlueLength(fl_get_input(dialog_->input_space_above)));
	break;
    }
    if (fl_get_button (dialog_->check_space_above))
	space_top.setKeep (true);
    switch (fl_get_choice (dialog_->choice_space_below)) {
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
	space_bottom =
		VSpace(LyXGlueLength(fl_get_input(dialog_->input_space_below)));
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
    Spacing::Space linespacing;
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
    LyXText * text = 0;
    if (lv_->view()->theLockingInset())
	text = lv_->view()->theLockingInset()->getLyXText(lv_->view());
    if (!text)
	text = lv_->view()->text;
    text->setParagraph(lv_->view(), line_top, line_bottom, pagebreak_top,
		       pagebreak_bottom, space_top, space_bottom, spacing,
                       align, labelwidthstring, noindent);


    // Actually apply these settings
    lv_->view()->update(lv_->view()->text, 
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
    fl_set_button(dialog_->radio_align_right, 0);
    fl_set_button(dialog_->radio_align_left, 0);
    fl_set_button(dialog_->radio_align_center, 0);
    fl_set_button(dialog_->radio_align_block, 0);

    int align = par_->getAlign();
    if (align == LYX_ALIGN_LAYOUT)
	align = textclasslist.Style(buf->params.textclass,
				    par_->getLayout()).align;

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

    LyXAlignment alignpos =
	    textclasslist.Style(buf->params.textclass,
				par_->getLayout()).alignpossible;

    setEnabled(dialog_->radio_align_block,  bool(alignpos & LYX_ALIGN_BLOCK));
    setEnabled(dialog_->radio_align_center, bool(alignpos & LYX_ALIGN_CENTER));
    setEnabled(dialog_->radio_align_left,   bool(alignpos & LYX_ALIGN_LEFT));
    setEnabled(dialog_->radio_align_right,  bool(alignpos & LYX_ALIGN_RIGHT));
    
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
    case VSpace::LENGTH:
	setEnabled(dialog_->input_space_above, true);
	fl_set_choice (dialog_->choice_space_above, 7);
	fl_set_input(dialog_->input_space_above, par_->
		     params().spaceTop().length().asString().c_str());
	break;
    }
    
    fl_set_button (dialog_->check_space_above,
		   par_->params().spaceTop().keep());
    fl_set_input (dialog_->input_space_below, "");

    setEnabled(dialog_->input_space_below, false);
    switch (par_->params().spaceBottom().kind()) {
    case VSpace::NONE:
	fl_set_choice (dialog_->choice_space_below, 1);
	break;
    case VSpace::DEFSKIP:
	fl_set_choice (dialog_->choice_space_below, 2);
	break;
    case VSpace::SMALLSKIP:
	fl_set_choice (dialog_->choice_space_below, 3);
	break;
    case VSpace::MEDSKIP:
	fl_set_choice (dialog_->choice_space_below, 4);
	break;
    case VSpace::BIGSKIP:
	fl_set_choice (dialog_->choice_space_below, 5);
	break;
    case VSpace::VFILL:
	fl_set_choice (dialog_->choice_space_below, 6);
	break;
    case VSpace::LENGTH:
        setEnabled(dialog_->input_space_below, true);
	fl_set_choice (dialog_->choice_space_below, 7);
        fl_set_input(dialog_->input_space_below, par_->
		     params().spaceBottom().length().asString().c_str());
	break;
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
            fl_set_input (dialog_->input_space_above, "");
            setEnabled (dialog_->input_space_above, false);
        } else {
            setEnabled (dialog_->input_space_above, !lv_->buffer()->isReadonly());
        }
    }
    if (ob == dialog_->choice_space_below) {
        if (fl_get_choice (dialog_->choice_space_below) != 7) {
            fl_set_input (dialog_->input_space_below, "");
            setEnabled (dialog_->input_space_below, false);
        } else {
            setEnabled (dialog_->input_space_below, !lv_->buffer()->isReadonly());
        }
    }
 
    if (fl_get_choice (dialog_->choice_linespacing) == 5)
        setEnabled (dialog_->input_linespacing, true);
    else {
        setEnabled (dialog_->input_linespacing, false);
        fl_set_input (dialog_->input_linespacing, "");
    }

    string input = fl_get_input (dialog_->input_space_above);
	
    if (fl_get_choice(dialog_->choice_space_above)==7 &&
        input.empty() || !isValidGlueLength(input))
        valid = false;

    if (ob == dialog_->input_space_above) {
        if (!isValidGlueLength(input)) {
            fl_set_object_label(dialog_->text_warning,
                _("Warning: Invalid Length (valid example: 10mm)"));
            fl_show_object(dialog_->text_warning);
            valid = false;
        } else
            fl_hide_object(dialog_->text_warning);
    }

    input = fl_get_input (dialog_->input_space_below);

    if (fl_get_choice(dialog_->choice_space_below)==7 &&
        input.empty() || !isValidGlueLength(input))
        valid = false;

    if (ob == dialog_->input_space_below) {
        if (!isValidGlueLength(input)) {
            fl_set_object_label(dialog_->text_warning,
                _("Warning: Invalid Length (valid example: 10mm)"));
            fl_show_object(dialog_->text_warning);
            valid = false;
        } else
            fl_hide_object(dialog_->text_warning);
    }

    double spacing(strToDbl(fl_get_input(dialog_->input_linespacing)));

    if (fl_get_choice (dialog_->choice_linespacing) == 5
        && int(spacing) == 0)
        valid = false;

    return valid;
}
