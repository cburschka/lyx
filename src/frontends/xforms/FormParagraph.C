/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000 The LyX Team.
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

#ifdef CXX_WORKING_NAMESPACES
using Liason::setMinibuffer;
#endif

FormParagraph::FormParagraph(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Paragraph Layout"),
		     new NoRepeatedApplyReadOnlyPolicy),
	dialog_(0), general_(0), extra_(0)
{
    // let the popup be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showLayoutParagraph.connect(slot(this, &FormParagraph::show));
}


FormParagraph::~FormParagraph()
{
    delete general_;
    delete extra_;
    delete dialog_;
}


FL_FORM * FormParagraph::form() const
{
    if (dialog_) return dialog_->form;
    return 0;
}


void FormParagraph::build()
{
    // the tabbed folder
    dialog_ = build_tabbed_paragraph();

    // manage the restore, ok, apply and cancel/close buttons
    bc_.setOK(dialog_->button_ok);
    bc_.setApply(dialog_->button_apply);
    bc_.setCancel(dialog_->button_cancel);
    bc_.setUndoAll(dialog_->button_restore);
    bc_.refresh();

    // the general paragraph data form
    general_ = build_paragraph_general();

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

    // the document class form
    extra_ = build_paragraph_extra();

    fl_set_input_return(extra_->input_pextra_width, FL_RETURN_CHANGED);
    fl_set_input_return(extra_->input_pextra_widthp, FL_RETURN_CHANGED);

    bc_.addReadOnly (extra_->radio_pextra_indent);
    bc_.addReadOnly (extra_->radio_pextra_minipage);
    bc_.addReadOnly (extra_->radio_pextra_floatflt);
    bc_.addReadOnly (extra_->radio_pextra_hfill);
    bc_.addReadOnly (extra_->radio_pextra_startmp);

    // now make them fit together
    fl_addto_tabfolder(dialog_->tabbed_folder,_("General"), general_->form);
    fl_addto_tabfolder(dialog_->tabbed_folder,_("Extra"), extra_->form);
}


void FormParagraph::apply()
{
    if (!lv_->view()->available() || !dialog_)
	return;

    general_apply();
    extra_apply();

    lv_->view()->update(BufferView::SELECT | BufferView::FITCUR |
			BufferView::CHANGE);
    lv_->buffer()->markDirty();
    setMinibuffer(lv_, _("Paragraph layout set"));
}


// we can safely ignore the parameter because we can always update
void FormParagraph::update(bool)
{
    if (!dialog_)
        return;

    general_update();
    extra_update();
    bc_.readOnly(lv_->buffer()->isReadonly());
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


void FormParagraph::extra_apply()
{
    char const * width = fl_get_input(extra_->input_pextra_width);
    char const * widthp = fl_get_input(extra_->input_pextra_widthp);
    LyXText * text = lv_->view()->text;
    int type = LyXParagraph::PEXTRA_NONE;
    LyXParagraph::MINIPAGE_ALIGNMENT
	alignment = LyXParagraph::MINIPAGE_ALIGN_TOP;
    bool
	hfill = false,
	start_minipage = false;

    if (fl_get_button(extra_->radio_pextra_indent)) {
	type = LyXParagraph::PEXTRA_INDENT;
    } else if (fl_get_button(extra_->radio_pextra_minipage)) {
	type = LyXParagraph::PEXTRA_MINIPAGE;
	hfill = fl_get_button(extra_->radio_pextra_hfill);
	start_minipage = fl_get_button(extra_->radio_pextra_startmp);
	if (fl_get_button(extra_->radio_pextra_top))
	    alignment = LyXParagraph::MINIPAGE_ALIGN_TOP;
	else if (fl_get_button(extra_->radio_pextra_middle))
	    alignment = LyXParagraph::MINIPAGE_ALIGN_MIDDLE;
	else if (fl_get_button(extra_->radio_pextra_bottom))
	    alignment = LyXParagraph::MINIPAGE_ALIGN_BOTTOM;
    } else if (fl_get_button(extra_->radio_pextra_floatflt)) {
	type = LyXParagraph::PEXTRA_FLOATFLT;
    }
    text->SetParagraphExtraOpt(lv_->view(), type, width, widthp, alignment,
			       hfill, start_minipage);
}


void FormParagraph::general_update()
{
    if (!general_)
        return;

    Buffer * buf = lv_->view()->buffer();
    LyXText * text = 0;

    if (lv_->view()->theLockingInset())
	text = lv_->view()->theLockingInset()->getLyXText(lv_->view());
    if (!text)
	text = lv_->view()->text;

    fl_set_input(general_->input_labelwidth,
		 text->cursor.par()->GetLabelWidthString().c_str());
    fl_set_button(general_->radio_align_right, 0);
    fl_set_button(general_->radio_align_left, 0);
    fl_set_button(general_->radio_align_center, 0);
    fl_set_button(general_->radio_align_block, 0);

    int align = text->cursor.par()->GetAlign();
    if (align == LYX_ALIGN_LAYOUT)
	align = textclasslist.Style(buf->params.textclass,
				    text->cursor.par()->GetLayout()).align;
	 
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

#ifndef NEW_INSETS
    fl_set_button(general_->check_lines_top,
		  text->cursor.par()->FirstPhysicalPar()->line_top);
    
    fl_set_button(general_->check_lines_bottom,
		  text->cursor.par()->FirstPhysicalPar()->line_bottom);
    
    fl_set_button(general_->check_pagebreaks_top,
		  text->cursor.par()->FirstPhysicalPar()->pagebreak_top);
    
    fl_set_button(general_->check_pagebreaks_bottom,
		  text->cursor.par()->FirstPhysicalPar()->pagebreak_bottom);
    fl_set_button(general_->check_noindent,
		  text->cursor.par()->FirstPhysicalPar()->noindent);
#else
    fl_set_button(general_->check_lines_top,
		  text->cursor.par()->line_top);
    fl_set_button(general_->check_lines_bottom,
		  text->cursor.par()->line_bottom);
    fl_set_button(general_->check_pagebreaks_top,
		  text->cursor.par()->pagebreak_top);
    fl_set_button(general_->check_pagebreaks_bottom,
		  text->cursor.par()->pagebreak_bottom);
    fl_set_button(general_->check_noindent,
		  text->cursor.par()->noindent);
#endif
    fl_set_input (general_->input_space_above, "");

#ifndef NEW_INSETS
    switch (text->cursor.par()->FirstPhysicalPar()->added_space_top.kind()) {
#else
    switch (text->cursor.par()->added_space_top.kind()) {
#endif

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
#ifndef NEW_INSETS
	fl_set_input(general_->input_space_above, 
		     text->cursor.par()->FirstPhysicalPar()->
		     added_space_top.length().asString().c_str());
#else
	fl_set_input(general_->input_space_above, text->cursor.par()->
		     added_space_top.length().asString().c_str());
#endif
	break;
    }
#ifndef NEW_INSETS
    fl_set_button(general_->check_space_above,
		   text->cursor.par()->FirstPhysicalPar()->
		   added_space_top.keep());
    fl_set_input(general_->input_space_below, "");
    switch (text->cursor.par()->FirstPhysicalPar()->
	    added_space_bottom.kind()) {
#else
    fl_set_button (general_->check_space_above,
		   text->cursor.par()->added_space_top.keep());
    fl_set_input (general_->input_space_below, "");
    switch (text->cursor.par()->added_space_bottom.kind()) {
#endif
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
#ifndef NEW_INSETS
	fl_set_input(general_->input_space_below, 
		     text->cursor.par()->FirstPhysicalPar()->
		     added_space_bottom.length().asString().c_str());
	break;
    }
    fl_set_button(general_->check_space_below,
		   text->cursor.par()->FirstPhysicalPar()->
		   added_space_bottom.keep());
    fl_set_button(general_->check_noindent,
		  text->cursor.par()->FirstPhysicalPar()->noindent);
    if (text->cursor.par()->FirstPhysicalPar()->InInset()) {
	fl_set_button(general_->check_pagebreaks_top, 0);
	fl_deactivate_object(general_->check_pagebreaks_top);
	fl_set_object_lcol(general_->check_pagebreaks_top, FL_INACTIVE);
	fl_set_button(general_->check_pagebreaks_bottom, 0);
	fl_deactivate_object(general_->check_pagebreaks_bottom);
	fl_set_object_lcol(general_->check_pagebreaks_bottom, FL_INACTIVE);
    } else {
	fl_activate_object(general_->check_pagebreaks_top);
	fl_set_object_lcol(general_->check_pagebreaks_top, FL_BLACK);
	fl_activate_object(general_->check_pagebreaks_bottom);
	fl_set_object_lcol(general_->check_pagebreaks_bottom, FL_BLACK);
    }
#else
        fl_set_input(general_->input_space_below, text->cursor.par()->
		     added_space_bottom.length().asString().c_str());
	break;
    }
    fl_set_button(general_->check_space_below,
		   text->cursor.par()->added_space_bottom.keep());
    fl_set_button(general_->check_noindent,
		  text->cursor.par()->noindent);
#endif
}


void FormParagraph::extra_update()
{
    if (!lv_->view()->available() || !extra_)
        return;

    LyXParagraph * par = lv_->view()->text->cursor.par();

    fl_activate_object(extra_->input_pextra_width);
    fl_set_object_lcol(extra_->input_pextra_width, FL_BLACK);
    fl_activate_object(extra_->input_pextra_widthp);
    fl_set_object_lcol(extra_->input_pextra_widthp, FL_BLACK);
    fl_set_input(extra_->input_pextra_width,
		 par->pextra_width.c_str());
    fl_set_input(extra_->input_pextra_widthp,
		 par->pextra_widthp.c_str());
    switch(par->pextra_alignment) {
    case LyXParagraph::MINIPAGE_ALIGN_TOP:
	fl_set_button(extra_->radio_pextra_top, 1);
	break;
    case LyXParagraph::MINIPAGE_ALIGN_MIDDLE:
	fl_set_button(extra_->radio_pextra_middle, 1);
	break;
    case LyXParagraph::MINIPAGE_ALIGN_BOTTOM:
	fl_set_button(extra_->radio_pextra_bottom, 1);
	break;
    }
    fl_set_button(extra_->radio_pextra_hfill,
		  par->pextra_hfill);
    fl_set_button(extra_->radio_pextra_startmp,
		  par->pextra_start_minipage);
    if (par->pextra_type == LyXParagraph::PEXTRA_INDENT) {
	fl_set_button(extra_->radio_pextra_indent, 1);
	fl_set_button(extra_->radio_pextra_minipage, 0);
	fl_set_button(extra_->radio_pextra_floatflt, 0);
	fl_deactivate_object(extra_->radio_pextra_top);
	fl_set_object_lcol(extra_->radio_pextra_top, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_middle);
	fl_set_object_lcol(extra_->radio_pextra_middle, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_bottom);
	fl_set_object_lcol(extra_->radio_pextra_bottom, FL_INACTIVE);
	input(extra_->radio_pextra_indent, 0);
    } else if (par->pextra_type == LyXParagraph::PEXTRA_MINIPAGE) {
	fl_set_button(extra_->radio_pextra_indent, 0);
	fl_set_button(extra_->radio_pextra_minipage, 1);
	fl_set_button(extra_->radio_pextra_floatflt, 0);
	fl_activate_object(extra_->radio_pextra_top);
	fl_set_object_lcol(extra_->radio_pextra_top, FL_BLACK);
	fl_activate_object(extra_->radio_pextra_middle);
	fl_set_object_lcol(extra_->radio_pextra_middle, FL_BLACK);
	fl_activate_object(extra_->radio_pextra_bottom);
	fl_set_object_lcol(extra_->radio_pextra_bottom, FL_BLACK);
	input(extra_->radio_pextra_minipage, 0);
    } else if (par->pextra_type == LyXParagraph::PEXTRA_FLOATFLT) {
	fl_set_button(extra_->radio_pextra_indent, 0);
	fl_set_button(extra_->radio_pextra_minipage, 0);
	fl_set_button(extra_->radio_pextra_floatflt, 1);
	fl_deactivate_object(extra_->radio_pextra_top);
	fl_set_object_lcol(extra_->radio_pextra_top, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_middle);
	fl_set_object_lcol(extra_->radio_pextra_middle, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_bottom);
	fl_set_object_lcol(extra_->radio_pextra_bottom, FL_INACTIVE);
	input(extra_->radio_pextra_floatflt, 0);
    } else {
	fl_set_button(extra_->radio_pextra_indent, 0);
	fl_set_button(extra_->radio_pextra_minipage, 0);
	fl_set_button(extra_->radio_pextra_floatflt, 0);
	fl_deactivate_object(extra_->input_pextra_width);
	fl_set_object_lcol(extra_->input_pextra_width, FL_INACTIVE);
	fl_deactivate_object(extra_->input_pextra_widthp);
	fl_set_object_lcol(extra_->input_pextra_widthp, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_top);
	fl_set_object_lcol(extra_->radio_pextra_top, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_middle);
	fl_set_object_lcol(extra_->radio_pextra_middle, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_bottom);
	fl_set_object_lcol(extra_->radio_pextra_bottom, FL_INACTIVE);
	input(0, 0);
    }
    fl_hide_object(dialog_->text_warning);
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
    if (ob == general_->choice_space_above) {
	if (fl_get_choice (general_->choice_space_above) != 7)
	    fl_set_input (general_->input_space_above, "");
    } else if (ob == general_->choice_space_below) {
	if (fl_get_choice (general_->choice_space_below) != 7)
	    fl_set_input (general_->input_space_below, "");
    }
    //
    // then the extra form
    //
    else if (ob == extra_->radio_pextra_indent) {
	int n = fl_get_button(extra_->radio_pextra_indent);
	if (n) {
	    fl_set_button(extra_->radio_pextra_minipage, 0);
	    fl_set_button(extra_->radio_pextra_floatflt, 0);
	    fl_activate_object(extra_->input_pextra_width);
	    fl_set_object_lcol(extra_->input_pextra_width, FL_BLACK);
	    fl_activate_object(extra_->input_pextra_widthp);
	    fl_set_object_lcol(extra_->input_pextra_widthp, FL_BLACK);
	} else {
	    fl_deactivate_object(extra_->input_pextra_width);
	    fl_set_object_lcol(extra_->input_pextra_width, FL_INACTIVE);
	    fl_deactivate_object(extra_->input_pextra_widthp);
	    fl_set_object_lcol(extra_->input_pextra_widthp, FL_INACTIVE);
	}
	fl_deactivate_object(extra_->radio_pextra_top);
	fl_set_object_lcol(extra_->radio_pextra_top, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_middle);
	fl_set_object_lcol(extra_->radio_pextra_middle, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_bottom);
	fl_set_object_lcol(extra_->radio_pextra_bottom, FL_INACTIVE);
	fl_activate_object(extra_->radio_pextra_hfill);
	fl_set_object_lcol(extra_->radio_pextra_hfill, FL_INACTIVE);
	fl_activate_object(extra_->radio_pextra_startmp);
	fl_set_object_lcol(extra_->radio_pextra_startmp, FL_INACTIVE);
    } else if (ob == extra_->radio_pextra_minipage) {
	int n = fl_get_button(extra_->radio_pextra_minipage);
	if (n) {
	    fl_set_button(extra_->radio_pextra_indent, 0);
	    fl_set_button(extra_->radio_pextra_floatflt, 0);
	    fl_activate_object(extra_->input_pextra_width);
	    fl_set_object_lcol(extra_->input_pextra_width, FL_BLACK);
	    fl_activate_object(extra_->input_pextra_widthp);
	    fl_set_object_lcol(extra_->input_pextra_widthp, FL_BLACK);
	    fl_activate_object(extra_->radio_pextra_top);
	    fl_set_object_lcol(extra_->radio_pextra_top, FL_BLACK);
	    fl_activate_object(extra_->radio_pextra_middle);
	    fl_set_object_lcol(extra_->radio_pextra_middle, FL_BLACK);
	    fl_activate_object(extra_->radio_pextra_bottom);
	    fl_set_object_lcol(extra_->radio_pextra_bottom, FL_BLACK);
	    fl_activate_object(extra_->radio_pextra_hfill);
	    fl_set_object_lcol(extra_->radio_pextra_hfill, FL_BLACK);
	    fl_activate_object(extra_->radio_pextra_startmp);
	    fl_set_object_lcol(extra_->radio_pextra_startmp, FL_BLACK);
	} else {
	    fl_deactivate_object(extra_->input_pextra_width);
	    fl_set_object_lcol(extra_->input_pextra_width, FL_INACTIVE);
	    fl_deactivate_object(extra_->input_pextra_widthp);
	    fl_set_object_lcol(extra_->input_pextra_widthp, FL_INACTIVE);
	    fl_deactivate_object(extra_->radio_pextra_top);
	    fl_set_object_lcol(extra_->radio_pextra_top, FL_INACTIVE);
	    fl_deactivate_object(extra_->radio_pextra_middle);
	    fl_set_object_lcol(extra_->radio_pextra_middle, FL_INACTIVE);
	    fl_deactivate_object(extra_->radio_pextra_bottom);
	    fl_set_object_lcol(extra_->radio_pextra_bottom, FL_INACTIVE);
	    fl_activate_object(extra_->radio_pextra_hfill);
	    fl_set_object_lcol(extra_->radio_pextra_hfill, FL_INACTIVE);
	    fl_activate_object(extra_->radio_pextra_startmp);
	    fl_set_object_lcol(extra_->radio_pextra_startmp, FL_INACTIVE);
	}
    } else if (ob == extra_->radio_pextra_floatflt) {
	int n = fl_get_button(extra_->radio_pextra_floatflt);
	if (n) {
	    fl_set_button(extra_->radio_pextra_indent, 0);
	    fl_set_button(extra_->radio_pextra_minipage, 0);
	    fl_activate_object(extra_->input_pextra_width);
	    fl_set_object_lcol(extra_->input_pextra_width, FL_BLACK);
	    fl_activate_object(extra_->input_pextra_widthp);
	    fl_set_object_lcol(extra_->input_pextra_widthp, FL_BLACK);
	} else {
	    fl_deactivate_object(extra_->input_pextra_width);
	    fl_set_object_lcol(extra_->input_pextra_width, FL_INACTIVE);
	    fl_deactivate_object(extra_->input_pextra_widthp);
	    fl_set_object_lcol(extra_->input_pextra_widthp, FL_INACTIVE);
	}
	fl_deactivate_object(extra_->radio_pextra_top);
	fl_set_object_lcol(extra_->radio_pextra_top, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_middle);
	fl_set_object_lcol(extra_->radio_pextra_middle, FL_INACTIVE);
	fl_deactivate_object(extra_->radio_pextra_bottom);
	fl_set_object_lcol(extra_->radio_pextra_bottom, FL_INACTIVE);
	fl_activate_object(extra_->radio_pextra_hfill);
	fl_set_object_lcol(extra_->radio_pextra_hfill, FL_INACTIVE);
	fl_activate_object(extra_->radio_pextra_startmp);
	fl_set_object_lcol(extra_->radio_pextra_startmp, FL_INACTIVE);
    }
    
    //
    // first the general form
    //
    string input = fl_get_input (general_->input_space_above);
	
    if (input.empty()) {
	fl_set_choice (general_->choice_space_above, 1);
    } else if (isValidGlueLength (input)) {
	fl_set_choice (general_->choice_space_above, 7);
    } else {
	fl_set_choice (general_->choice_space_above, 7);
	fl_set_object_label(dialog_->text_warning,
		    _("Warning: Invalid Length (valid example: 10mm)"));
	fl_show_object(dialog_->text_warning);
	ret = false;
    }
    
    input = fl_get_input (general_->input_space_below);
	
    if (input.empty()) {
	fl_set_choice (general_->choice_space_below, 1);
    } else if (isValidGlueLength(input)) {
	fl_set_choice (general_->choice_space_below, 7);
    } else {
	fl_set_choice (general_->choice_space_below, 7);
	fl_set_object_label(dialog_->text_warning,
		    _("Warning: Invalid Length (valid example: 10mm)"));
	fl_show_object(dialog_->text_warning);
	ret = false;
    }
    //
    // then the extra form
    //
    int n = fl_get_button(extra_->radio_pextra_indent) +
	fl_get_button(extra_->radio_pextra_minipage) +
	fl_get_button(extra_->radio_pextra_floatflt);
    string s1 = fl_get_input(extra_->input_pextra_width);
    string s2 = fl_get_input(extra_->input_pextra_widthp);
    if (!n) { // no button pressed both should be deactivated now
	fl_deactivate_object(extra_->input_pextra_width);
	fl_set_object_lcol(extra_->input_pextra_width, FL_INACTIVE);
	fl_deactivate_object(extra_->input_pextra_widthp);
	fl_set_object_lcol(extra_->input_pextra_widthp, FL_INACTIVE);
	fl_hide_object(dialog_->text_warning);
    } else if (s1.empty() && s2.empty()) {
	fl_activate_object(extra_->input_pextra_width);
	fl_set_object_lcol(extra_->input_pextra_width, FL_BLACK);
	fl_activate_object(extra_->input_pextra_widthp);
	fl_set_object_lcol(extra_->input_pextra_widthp, FL_BLACK);
	fl_hide_object(dialog_->text_warning);
	ret = false;
    } else if (!s1.empty()) { // LyXLength parameter
	fl_activate_object(extra_->input_pextra_width);
	fl_set_object_lcol(extra_->input_pextra_width, FL_BLACK);
	fl_deactivate_object(extra_->input_pextra_widthp);
	fl_set_object_lcol(extra_->input_pextra_widthp, FL_INACTIVE);
	if (!isValidLength(s1)) {
	    fl_set_object_label(dialog_->text_warning,
			_("Warning: Invalid Length (valid example: 10mm)"));
	    fl_show_object(dialog_->text_warning);
	    ret = false;
	}
    } else { // !s2.empty() % parameter
	fl_deactivate_object(extra_->input_pextra_width);
	fl_set_object_lcol(extra_->input_pextra_width, FL_INACTIVE);
	fl_activate_object(extra_->input_pextra_widthp);
	fl_set_object_lcol(extra_->input_pextra_widthp, FL_BLACK);
	if ((lyx::atoi(s2) < 0 ) || (lyx::atoi(s2) > 100)) {
	    ret = false;
	    fl_set_object_label(dialog_->text_warning,
			_("Warning: Invalid percent value (0-100)"));
	    fl_show_object(dialog_->text_warning);
	}
    }
    return ret;
}

