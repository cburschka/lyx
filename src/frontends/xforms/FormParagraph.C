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
#include "Liason.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxtext.h"
#include "xforms_helpers.h"

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


LyXParagraph const * FormParagraph::getCurrentParagraph() const
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
	LyXParagraph const * const p = getCurrentParagraph();
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
	else
		return;

	FL_FORM * outer_form = fl_get_active_folder(dialog_->tabbed_folder);
	if (outer_form && outer_form->visible)
		fl_redraw_form( outer_form );
}


FL_FORM * FormParagraph::form() const
{
    if (dialog_.get()) return dialog_->form;
    return 0;
}


void FormParagraph::build()
{
    // the tabbed folder
    dialog_.reset(build_tabbed_paragraph());

    // Manage the ok, apply, restore and cancel/close buttons
    bc_.setOK(dialog_->button_ok);
    bc_.setApply(dialog_->button_apply);
    bc_.setCancel(dialog_->button_cancel);
    bc_.setRestore(dialog_->button_restore);

    // the general paragraph data form
    general_.reset(build_paragraph_general());

    fl_addto_choice(general_->choice_space_above,
		    _(" None | Defskip | Smallskip "
		      "| Medskip | Bigskip | VFill | Length "));
    fl_addto_choice(general_->choice_space_below,
		    _(" None | Defskip | Smallskip "
		      "| Medskip | Bigskip | VFill | Length ")); 

    fl_set_input_return(general_->input_space_above, FL_RETURN_CHANGED);
    fl_set_input_return(general_->input_space_below, FL_RETURN_CHANGED);
    fl_set_input_return(general_->input_labelwidth, FL_RETURN_CHANGED);

    bc_.addReadOnly (general_->radio_align_right);
    bc_.addReadOnly (general_->radio_align_left);
    bc_.addReadOnly (general_->radio_align_block);
    bc_.addReadOnly (general_->radio_align_center);
    bc_.addReadOnly (general_->check_lines_top);
    bc_.addReadOnly (general_->check_lines_bottom);
    bc_.addReadOnly (general_->check_pagebreaks_top);
    bc_.addReadOnly (general_->check_pagebreaks_bottom);
    bc_.addReadOnly (general_->choice_space_above);
    bc_.addReadOnly (general_->input_space_above);
    bc_.addReadOnly (general_->check_space_above);
    bc_.addReadOnly (general_->choice_space_below);
    bc_.addReadOnly (general_->input_space_below);
    bc_.addReadOnly (general_->check_space_below);
    bc_.addReadOnly (general_->check_noindent);
    bc_.addReadOnly (general_->input_labelwidth);

    // now make them fit together
    fl_addto_tabfolder(dialog_->tabbed_folder,_("General"), general_->form);
}


void FormParagraph::apply()
{
    if (!lv_->view()->available() || !dialog_.get())
	return;

    general_apply();

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

    general_update();
}


void FormParagraph::general_apply()
{
    VSpace space_top, space_bottom;
    LyXAlignment align;
    string labelwidthstring;
    bool noindent;

    // If a vspace kind is "Length" but there's no text in
    // the input field, reset the kind to "None". 
    if ((fl_get_choice (general_->choice_space_above) == 7) &&
	!*(fl_get_input (general_->input_space_above)))
    {
	fl_set_choice (general_->choice_space_above, 1);
    }
    if ((fl_get_choice (general_->choice_space_below) == 7) &&
	!*(fl_get_input (general_->input_space_below)))
    {
	fl_set_choice (general_->choice_space_below, 1);
    }
   
    bool line_top = fl_get_button(general_->check_lines_top);
    bool line_bottom = fl_get_button(general_->check_lines_bottom);
    bool pagebreak_top = fl_get_button(general_->check_pagebreaks_top);
    bool pagebreak_bottom = fl_get_button(general_->check_pagebreaks_bottom);
    
    switch (fl_get_choice (general_->choice_space_above)) {
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
		VSpace(LyXGlueLength(fl_get_input(general_->input_space_above)));
	break;
    }
    if (fl_get_button (general_->check_space_above))
	space_top.setKeep (true);
    switch (fl_get_choice (general_->choice_space_below)) {
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
		VSpace(LyXGlueLength(fl_get_input(general_->input_space_below)));
	break;
    }
    if (fl_get_button (general_->check_space_below))
	space_bottom.setKeep (true);

    if (fl_get_button(general_->radio_align_left))
	align = LYX_ALIGN_LEFT;
    else if (fl_get_button(general_->radio_align_right))
	align = LYX_ALIGN_RIGHT;
    else if (fl_get_button(general_->radio_align_center))
	align = LYX_ALIGN_CENTER;
    else 
	align = LYX_ALIGN_BLOCK;
   
    labelwidthstring = fl_get_input(general_->input_labelwidth);
    noindent = fl_get_button(general_->check_noindent);

    LyXText * text = 0;
    if (lv_->view()->theLockingInset())
	text = lv_->view()->theLockingInset()->getLyXText(lv_->view());
    if (!text)
	text = lv_->view()->text;
    text->SetParagraph(lv_->view(), line_top, line_bottom, pagebreak_top,
		       pagebreak_bottom, space_top, space_bottom, align, 
		       labelwidthstring, noindent);
}


void FormParagraph::general_update()
{
    if (!general_.get())
        return;

    Buffer * buf = lv_->view()->buffer();

    /// Record the paragraph
    par_ = getCurrentParagraph();

    fl_set_input(general_->input_labelwidth,
		 par_->GetLabelWidthString().c_str());
    fl_set_button(general_->radio_align_right, 0);
    fl_set_button(general_->radio_align_left, 0);
    fl_set_button(general_->radio_align_center, 0);
    fl_set_button(general_->radio_align_block, 0);

    int align = par_->GetAlign();
    if (align == LYX_ALIGN_LAYOUT)
	align = textclasslist.Style(buf->params.textclass,
				    par_->GetLayout()).align;

    switch (align) {
    case LYX_ALIGN_RIGHT:
	fl_set_button(general_->radio_align_right, 1);
	break;
    case LYX_ALIGN_LEFT:
	fl_set_button(general_->radio_align_left, 1);
	break;
    case LYX_ALIGN_CENTER:
	fl_set_button(general_->radio_align_center, 1);
	break;
    default:
	fl_set_button(general_->radio_align_block, 1);
	break;
    }

    LyXAlignment alignpos =
	    textclasslist.Style(buf->params.textclass,
				par_->GetLayout()).alignpossible;

    setEnabled(general_->radio_align_block,  bool(alignpos & LYX_ALIGN_BLOCK));
    setEnabled(general_->radio_align_center, bool(alignpos & LYX_ALIGN_CENTER));
    setEnabled(general_->radio_align_left,   bool(alignpos & LYX_ALIGN_LEFT));
    setEnabled(general_->radio_align_right,  bool(alignpos & LYX_ALIGN_RIGHT));
    
    fl_set_button(general_->check_lines_top,
		  par_->params.lineTop());
    fl_set_button(general_->check_lines_bottom,
		  par_->params.lineBottom());
    fl_set_button(general_->check_pagebreaks_top,
		  par_->params.pagebreakTop());
    fl_set_button(general_->check_pagebreaks_bottom,
		  par_->params.pagebreakBottom());
    fl_set_button(general_->check_noindent,
		  par_->params.noindent());

    fl_set_input (general_->input_space_above, "");

    switch (par_->params.spaceTop().kind()) {
    case VSpace::NONE:
	fl_set_choice (general_->choice_space_above, 1);
	break;
    case VSpace::DEFSKIP:
	fl_set_choice (general_->choice_space_above, 2);
	break;
    case VSpace::SMALLSKIP:
	fl_set_choice (general_->choice_space_above, 3);
	break;
    case VSpace::MEDSKIP:
	fl_set_choice (general_->choice_space_above, 4);
	break;
    case VSpace::BIGSKIP:
	fl_set_choice (general_->choice_space_above, 5);
	break;
    case VSpace::VFILL:
	fl_set_choice (general_->choice_space_above, 6);
	break;
    case VSpace::LENGTH:
	fl_set_choice (general_->choice_space_above, 7);
	fl_set_input(general_->input_space_above, par_->
		     params.spaceTop().length().asString().c_str());
	break;
    }
    
    fl_set_button (general_->check_space_above,
		   par_->params.spaceTop().keep());
    fl_set_input (general_->input_space_below, "");

    switch (par_->params.spaceBottom().kind()) {
    case VSpace::NONE:
	fl_set_choice (general_->choice_space_below, 1);
	break;
    case VSpace::DEFSKIP:
	fl_set_choice (general_->choice_space_below, 2);
	break;
    case VSpace::SMALLSKIP:
	fl_set_choice (general_->choice_space_below, 3);
	break;
    case VSpace::MEDSKIP:
	fl_set_choice (general_->choice_space_below, 4);
	break;
    case VSpace::BIGSKIP:
	fl_set_choice (general_->choice_space_below, 5);
	break;
    case VSpace::VFILL:
	fl_set_choice (general_->choice_space_below, 6);
	break;
    case VSpace::LENGTH:
	fl_set_choice (general_->choice_space_below, 7);
        fl_set_input(general_->input_space_below, par_->
		     params.spaceBottom().length().asString().c_str());
	break;
    }
    fl_set_button(general_->check_space_below,
		   par_->params.spaceBottom().keep());
    fl_set_button(general_->check_noindent,
		  par_->params.noindent());
}


bool FormParagraph::input(FL_OBJECT * ob, long)
{
    bool ret = true;

    fl_hide_object(dialog_->text_warning);

    // First check the buttons which are exclusive and you have to
    // check only the actuall de/activated button.
    //
    // general form first
    //
    // "Synchronize" the choices and input fields, making it
    // impossible to commit senseless data.

    if (fl_get_choice (general_->choice_space_above) != 7)
        fl_set_input (general_->input_space_above, "");

    if (fl_get_choice (general_->choice_space_below) != 7)
        fl_set_input (general_->input_space_below, "");

    //
    // first the general form
    //
    string input = fl_get_input (general_->input_space_above);
    bool invalid = false;
	
    if (fl_get_choice(general_->choice_space_above)==7)
        invalid = !input.empty() && !isValidGlueLength(input);

    input = fl_get_input (general_->input_space_below);

    if (fl_get_choice(general_->choice_space_below)==7)
        invalid = invalid || (!input.empty() && !isValidGlueLength(input));
    
    if (ob == general_->input_space_above || ob == general_->input_space_below) {
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

    return ret;
}

